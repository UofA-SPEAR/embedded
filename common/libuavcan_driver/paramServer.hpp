#ifndef __PARAM_SERVER_HPP
#define __PARAM_SERVER_HPP
#include "uavcan/protocol/param/Value.hpp"
#include "ch.h"
#include "uavcan/protocol/param_server.hpp"
#include <etl/string.h>
#include <etl/to_string.h>
#include <etl/array.h>
/** @brief Structure to hold the relevant information for settings
 */
#define NUM_SETTINGS 18
typedef etl::string<20> stringName;
typedef struct {
  stringName name;
  uavcan::protocol::param::Value::Tag::Type tag;
} setting_spec_t;

typedef union {
  float real;
  int integer;
  bool boolean;
} value_t;

typedef struct {
  setting_spec_t union_tag;
  value_t value;
  value_t min;
  value_t max;
  value_t def;
} setting_value_t;
using cfgArray_t = etl::array<setting_value_t, NUM_SETTINGS>;

/** @brief Standard initialisation for a real setting */
#define SETTING_SPEC_REAL(str, defaultVal, min, max) \
  {{str, uavcan::protocol::param::Value::Tag::Type::real_value}, \
    {real: defaultVal}, {real: min}, {real: max}, {real: defaultVal}}

/** @brief Standard initialisation for an integer setting */
#define SETTING_SPEC_INT(str, defaultVal, min, max) \
  {{str, uavcan::protocol::param::Value::Tag::Type::integer_value}, \
    {integer: defaultVal}, {integer: min}, {integer: max}, {integer: defaultVal}}

/** @brief Standard initialisation for a boolean setting */
#define SETTING_SPEC_BOOL(str, defaultVal) \
  {{str, uavcan::protocol::param::Value::Tag::Type::boolean_value}, \
    {boolean: defaultVal}, {boolean: 0}, {boolean: 1}, {boolean: defaultVal}}



class dataCfg : public uavcan::IParamManager
{
  cfgArray_t _setting;
  const cfgArray_t *_defaultSetting;
  mutex_t _mtx; 

  size_t get_setting_index_by_name(const stringName& name) const {
    for (size_t i = 0; i < _setting.size(); i++) {
      // assumes strings are defined constant, thus have null termination
        if (name.compare(_setting[i].union_tag.name) == 0) return i;
    }

    return -1;
  }

  setting_value_t* setting_by_name(const stringName& name) {
      size_t index = get_setting_index_by_name(name);
      return &_setting[index];
  }

  void getParamNameByIndex(Index index, Name& out_name) const override
  {
    stringName actualName = _setting[index].union_tag.name;
    out_name = actualName.c_str();
  }

  void assignParamValue(const Name& name, const Value& value) override;

  void readParamValue(const Name& name, Value& out_value) const override
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
  int saveAllParams() override;

  int eraseAllParams() override;

  void readParamDefaultMaxMin(const Name& name, Value& out_def,
                                    NumericValue& out_max, NumericValue& out_min) const override
  {
    const stringName _name = stringName(name.c_str());
    int8_t index = get_setting_index_by_name(_name);
    switch(_setting[index].union_tag.tag) {
      case uavcan::protocol::param::Value::Tag::Type::boolean_value:
        out_def.to<uavcan::protocol::param::Value::Tag::Type::boolean_value>() = _setting[index].def.boolean;
        break;
      case uavcan::protocol::param::Value::Tag::Type::integer_value:
        out_def.to<uavcan::protocol::param::Value::Tag::Type::integer_value>() = _setting[index].def.integer;
        out_max.to<uavcan::protocol::param::NumericValue::Tag::integer_value>() = _setting[index].max.integer;
        out_min.to<uavcan::protocol::param::NumericValue::Tag::integer_value>() = _setting[index].min.integer;
        break;
      case uavcan::protocol::param::Value::Tag::Type::real_value:
        out_def.to<uavcan::protocol::param::Value::Tag::Type::real_value>() = _setting[index].def.real;
        out_max.to<uavcan::protocol::param::NumericValue::Tag::real_value>() = _setting[index].max.real;
        out_min.to<uavcan::protocol::param::NumericValue::Tag::real_value>() = _setting[index].min.real;
        break;
      default:
        break;
    }
  }


  public:
    dataCfg(){}
    void init(const cfgArray_t *defaultConfig);
    // In order to avoid writing to flash while the MCU is doing something critical
    void lock();
    // Unlock after finished using
    void unlock();
    int get_setting_int(const stringName& name) {
      return setting_by_name(name)->value.integer;
    }
    float get_setting_real(const stringName& name) {
      return setting_by_name(name)->value.real;
    }
    bool get_setting_bool(const stringName& name) {
      return setting_by_name(name)->value.boolean;
    }
};
extern const cfgArray_t defaultCfg;
extern dataCfg data;
int paramServerInit();
#endif