#include "paramServer.hpp"
#include "coms.hpp"
#include "hal.h"
#include "hal_mfs.h"
static MFSDriver mfs1;

static const MFSConfig mfscfg1 = {
  .flashp           = (BaseFlash *)&EFLD1,
  .erased           = 0xFFFFFFFFU,
  .bank_size        = 4096U,
  .bank0_start      = 122U,
  .bank0_sectors    = 2U,
  .bank1_start      = 124U,
  .bank1_sectors    = 2U
};

void dataCfg::init(const cfgArray_t *defaultConfig)
{
  _defaultSetting = defaultConfig;
  chMtxObjectInit(&_mtx);
  size_t fileSize;
  int err = mfsReadRecord(&mfs1, 1, &fileSize, (uint8_t*)_setting.data());
  if(fileSize != sizeof(cfgArray_t) || err) {
    err = mfsEraseRecord(&mfs1, 1);
    err = mfsWriteRecord(&mfs1, 1, sizeof(cfgArray_t), (uint8_t*)_defaultSetting);
    err = mfsReadRecord(&mfs1, 1, &fileSize, (uint8_t*)_setting.data());
  }
}

void dataCfg::assignParamValue(const Name& name, const Value& value)
{
  bool isEnableVar = false;
  size_t index = get_setting_index_by_name(name.c_str());
  if(index != get_setting_index_by_name("enabled")) {
    if(!chMtxTryLock(&_mtx)) {
      return;
    }
    isEnableVar = true;
  }
  switch(_setting[index].union_tag.tag) {
    case uavcan::protocol::param::Value::Tag::Type::boolean_value:
      _setting[index].value.boolean = *value.as<uavcan::protocol::param::Value::Tag::boolean_value>();
      break;
    case uavcan::protocol::param::Value::Tag::Type::integer_value:
      _setting[index].value.integer = *value.as<uavcan::protocol::param::Value::Tag::integer_value>();
      break;
    case uavcan::protocol::param::Value::Tag::Type::real_value:
      _setting[index].value.real = *value.as<uavcan::protocol::param::Value::Tag::real_value>();
      break;
    default:
      break;
  }
  if(isEnableVar) chMtxUnlock(&_mtx);
}

int dataCfg::saveAllParams()
{
  mfs_error_t err;
  if(!chMtxTryLock(&_mtx)) {
      return -1;
  }
  err = mfsEraseRecord(&mfs1, 1);
  if(err) goto error;
  err = mfsWriteRecord(&mfs1, 1, sizeof(cfgArray_t), (uint8_t*)_setting.data());
error:
  chMtxUnlock(&_mtx);
  return err;
}

int dataCfg::eraseAllParams()
{
  size_t fileSize;
  mfs_error_t err;
  if(!chMtxTryLock(&_mtx)) {
      return -1;
  }
  err = mfsEraseRecord(&mfs1, 1);
  if(err) goto error;
  err = mfsWriteRecord(&mfs1, 1, sizeof(cfgArray_t), (uint8_t*)_defaultSetting);
  if(err) goto error;
  err = mfsReadRecord(&mfs1, 1, &fileSize, (uint8_t*)_setting.data());
error:
  chMtxUnlock(&_mtx);
  return err;
}
void dataCfg::lock()
{
  chMtxLock(&_mtx);
}

void dataCfg::unlock()
{
  chMtxUnlock(&_mtx);
}
uavcan::ParamServer *server;
dataCfg data;
// uavcan::ParamServer *server;
int paramServerInit()
{
  eflStart(&EFLD1, NULL);
  mfsObjectInit(&mfs1);
  mfs_error_t err = mfsStart(&mfs1, &mfscfg1);
  if(err) {
    err = mfsErase(&mfs1);
    mfsStop(&mfs1);
    mfsStart(&mfs1, &mfscfg1);
  }
  server = new uavcan::ParamServer(getNode());
  data.init(&defaultCfg);
  int res = server->start(&data);
  return res;
}