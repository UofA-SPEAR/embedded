#include "paramServer.hpp"
#include "coms.hpp"
#include "hal.h"
#include "hal_mfs.h"
static MFSDriver mfs1;
dataCfg data;
static const MFSConfig mfscfg1 = {
  .flashp           = (BaseFlash *)&EFLD1,
  .erased           = 0xFFFFFFFFU,
  .bank_size        = 4096U,
  .bank0_start      = 61U,
  .bank0_sectors    = 2U,
  .bank1_start      = 63U,
  .bank1_sectors    = 2U
};

int dataCfg::init(const cfgArray_t *defaultConfig)
{
  _defaultSetting = defaultConfig;
  chMtxObjectInit(&_mtx);
  size_t fileSize = _setting.size() * sizeof(setting_value_t);
  mfs_error_t err = mfsReadRecord(&mfs1, 1, &fileSize, (uint8_t*)_setting.data());
  if(err) {
    err = mfsEraseRecord(&mfs1, 1);
    if(err != MFS_NO_ERROR && err != MFS_ERR_NOT_FOUND) return err;
    err = mfsWriteRecord(&mfs1, 1, sizeof(cfgArray_t), (uint8_t*)_defaultSetting);
    if(err) return err;
    err = mfsReadRecord(&mfs1, 1, &fileSize, (uint8_t*)_setting.data());
    if(err) return err;
  }
}

void dataCfg::readParamValue(const Name& name, Value& out_value) const 
{
  const stringName _name = stringName(name.c_str());
  size_t index = get_setting_index_by_name(_name);
  switch(_setting[index].union_tag.tag) {
    case uavcan::protocol::param::Value::Tag::Type::boolean_value:
      out_value.to<uavcan::protocol::param::Value::Tag::Type::boolean_value>() = _setting[index].value.boolean;
      break;
    case uavcan::protocol::param::Value::Tag::Type::integer_value:
      out_value.to<uavcan::protocol::param::Value::Tag::Type::integer_value>() = _setting[index].value.integer;
      break;
    case uavcan::protocol::param::Value::Tag::Type::real_value:
      out_value.to<uavcan::protocol::param::Value::Tag::Type::real_value>() = _setting[index].value.real;
      break;
    default:
      break;
  }
}

void dataCfg::assignParamValue(const Name& name, const Value& value)
{
  size_t index = get_setting_index_by_name(name.c_str());
  if(index != get_setting_index_by_name("enabled")) {
    if(!chMtxTryLock(&_mtx)) {
      return;
    }
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
  chMtxUnlock(&_mtx);
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
  return 0;
}
