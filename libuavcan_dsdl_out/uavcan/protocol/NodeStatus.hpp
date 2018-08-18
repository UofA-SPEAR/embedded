/*
 * UAVCAN data structure definition for libuavcan.
 *
 * Autogenerated, do not edit.
 *
 * Source file: /home/isthatme/Documents/SPEAR/embedded/libuavcan/dsdl/uavcan/protocol/341.NodeStatus.uavcan
 */

#ifndef UAVCAN_PROTOCOL_NODESTATUS_HPP_INCLUDED
#define UAVCAN_PROTOCOL_NODESTATUS_HPP_INCLUDED

#include <uavcan/build_config.hpp>
#include <uavcan/node/global_data_type_registry.hpp>
#include <uavcan/marshal/types.hpp>

/******************************* Source text **********************************
#
# Abstract node status information.
#
# All UAVCAN nodes are required to publish this message periodically.
#

#
# Publication period may vary within these limits.
# It is NOT recommended to change it at run time.
#
uint16 MAX_BROADCASTING_PERIOD_MS = 1000
uint16 MIN_BROADCASTING_PERIOD_MS = 2

#
# If a node fails to publish this message in this amount of time, it should be considered offline.
#
uint16 OFFLINE_TIMEOUT_MS = 3000

#
# Uptime counter should never overflow.
# Other nodes may detect that a remote node has restarted when this value goes backwards.
#
uint32 uptime_sec

#
# Abstract node health.
#
uint2 HEALTH_OK         = 0     # The node is functioning properly.
uint2 HEALTH_WARNING    = 1     # A critical parameter went out of range or the node encountered a minor failure.
uint2 HEALTH_ERROR      = 2     # The node encountered a major failure.
uint2 HEALTH_CRITICAL   = 3     # The node suffered a fatal malfunction.
uint2 health

#
# Current mode.
#
# Mode OFFLINE can be actually reported by the node to explicitly inform other network
# participants that the sending node is about to shutdown. In this case other nodes will not
# have to wait OFFLINE_TIMEOUT_MS before they detect that the node is no longer available.
#
# Reserved values can be used in future revisions of the specification.
#
uint3 MODE_OPERATIONAL      = 0         # Normal operating mode.
uint3 MODE_INITIALIZATION   = 1         # Initialization is in progress; this mode is entered immediately after startup.
uint3 MODE_MAINTENANCE      = 2         # E.g. calibration, the bootloader is running, etc.
uint3 MODE_SOFTWARE_UPDATE  = 3         # New software/firmware is being loaded.
uint3 MODE_OFFLINE          = 7         # The node is no longer available.
uint3 mode

#
# Not used currently, keep zero when publishing, ignore when receiving.
#
uint3 sub_mode

#
# Optional, vendor-specific node status code, e.g. a fault code or a status bitmask.
#
uint16 vendor_specific_status_code
******************************************************************************/

/********************* DSDL signature source definition ***********************
uavcan.protocol.NodeStatus
saturated uint32 uptime_sec
saturated uint2 health
saturated uint3 mode
saturated uint3 sub_mode
saturated uint16 vendor_specific_status_code
******************************************************************************/

#undef uptime_sec
#undef health
#undef mode
#undef sub_mode
#undef vendor_specific_status_code
#undef MAX_BROADCASTING_PERIOD_MS
#undef MIN_BROADCASTING_PERIOD_MS
#undef OFFLINE_TIMEOUT_MS
#undef HEALTH_OK
#undef HEALTH_WARNING
#undef HEALTH_ERROR
#undef HEALTH_CRITICAL
#undef MODE_OPERATIONAL
#undef MODE_INITIALIZATION
#undef MODE_MAINTENANCE
#undef MODE_SOFTWARE_UPDATE
#undef MODE_OFFLINE

namespace uavcan
{
namespace protocol
{

template <int _tmpl>
struct UAVCAN_EXPORT NodeStatus_
{
    typedef const NodeStatus_<_tmpl>& ParameterType;
    typedef NodeStatus_<_tmpl>& ReferenceType;

    struct ConstantTypes
    {
        typedef ::uavcan::IntegerSpec< 16, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > MAX_BROADCASTING_PERIOD_MS;
        typedef ::uavcan::IntegerSpec< 16, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > MIN_BROADCASTING_PERIOD_MS;
        typedef ::uavcan::IntegerSpec< 16, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > OFFLINE_TIMEOUT_MS;
        typedef ::uavcan::IntegerSpec< 2, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > HEALTH_OK;
        typedef ::uavcan::IntegerSpec< 2, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > HEALTH_WARNING;
        typedef ::uavcan::IntegerSpec< 2, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > HEALTH_ERROR;
        typedef ::uavcan::IntegerSpec< 2, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > HEALTH_CRITICAL;
        typedef ::uavcan::IntegerSpec< 3, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > MODE_OPERATIONAL;
        typedef ::uavcan::IntegerSpec< 3, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > MODE_INITIALIZATION;
        typedef ::uavcan::IntegerSpec< 3, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > MODE_MAINTENANCE;
        typedef ::uavcan::IntegerSpec< 3, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > MODE_SOFTWARE_UPDATE;
        typedef ::uavcan::IntegerSpec< 3, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > MODE_OFFLINE;
    };

    struct FieldTypes
    {
        typedef ::uavcan::IntegerSpec< 32, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > uptime_sec;
        typedef ::uavcan::IntegerSpec< 2, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > health;
        typedef ::uavcan::IntegerSpec< 3, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > mode;
        typedef ::uavcan::IntegerSpec< 3, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > sub_mode;
        typedef ::uavcan::IntegerSpec< 16, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > vendor_specific_status_code;
    };

    enum
    {
        MinBitLen
            = FieldTypes::uptime_sec::MinBitLen
            + FieldTypes::health::MinBitLen
            + FieldTypes::mode::MinBitLen
            + FieldTypes::sub_mode::MinBitLen
            + FieldTypes::vendor_specific_status_code::MinBitLen
    };

    enum
    {
        MaxBitLen
            = FieldTypes::uptime_sec::MaxBitLen
            + FieldTypes::health::MaxBitLen
            + FieldTypes::mode::MaxBitLen
            + FieldTypes::sub_mode::MaxBitLen
            + FieldTypes::vendor_specific_status_code::MaxBitLen
    };

    // Constants
    static const typename ::uavcan::StorageType< typename ConstantTypes::MAX_BROADCASTING_PERIOD_MS >::Type MAX_BROADCASTING_PERIOD_MS; // 1000
    static const typename ::uavcan::StorageType< typename ConstantTypes::MIN_BROADCASTING_PERIOD_MS >::Type MIN_BROADCASTING_PERIOD_MS; // 2
    static const typename ::uavcan::StorageType< typename ConstantTypes::OFFLINE_TIMEOUT_MS >::Type OFFLINE_TIMEOUT_MS; // 3000
    static const typename ::uavcan::StorageType< typename ConstantTypes::HEALTH_OK >::Type HEALTH_OK; // 0
    static const typename ::uavcan::StorageType< typename ConstantTypes::HEALTH_WARNING >::Type HEALTH_WARNING; // 1
    static const typename ::uavcan::StorageType< typename ConstantTypes::HEALTH_ERROR >::Type HEALTH_ERROR; // 2
    static const typename ::uavcan::StorageType< typename ConstantTypes::HEALTH_CRITICAL >::Type HEALTH_CRITICAL; // 3
    static const typename ::uavcan::StorageType< typename ConstantTypes::MODE_OPERATIONAL >::Type MODE_OPERATIONAL; // 0
    static const typename ::uavcan::StorageType< typename ConstantTypes::MODE_INITIALIZATION >::Type MODE_INITIALIZATION; // 1
    static const typename ::uavcan::StorageType< typename ConstantTypes::MODE_MAINTENANCE >::Type MODE_MAINTENANCE; // 2
    static const typename ::uavcan::StorageType< typename ConstantTypes::MODE_SOFTWARE_UPDATE >::Type MODE_SOFTWARE_UPDATE; // 3
    static const typename ::uavcan::StorageType< typename ConstantTypes::MODE_OFFLINE >::Type MODE_OFFLINE; // 7

    // Fields
    typename ::uavcan::StorageType< typename FieldTypes::uptime_sec >::Type uptime_sec;
    typename ::uavcan::StorageType< typename FieldTypes::health >::Type health;
    typename ::uavcan::StorageType< typename FieldTypes::mode >::Type mode;
    typename ::uavcan::StorageType< typename FieldTypes::sub_mode >::Type sub_mode;
    typename ::uavcan::StorageType< typename FieldTypes::vendor_specific_status_code >::Type vendor_specific_status_code;

    NodeStatus_()
        : uptime_sec()
        , health()
        , mode()
        , sub_mode()
        , vendor_specific_status_code()
    {
        ::uavcan::StaticAssert<_tmpl == 0>::check();  // Usage check

#if UAVCAN_DEBUG
        /*
         * Cross-checking MaxBitLen provided by the DSDL compiler.
         * This check shall never be performed in user code because MaxBitLen value
         * actually depends on the nested types, thus it is not invariant.
         */
        ::uavcan::StaticAssert<56 == MaxBitLen>::check();
#endif
    }

    bool operator==(ParameterType rhs) const;
    bool operator!=(ParameterType rhs) const { return !operator==(rhs); }

    /**
     * This comparison is based on @ref uavcan::areClose(), which ensures proper comparison of
     * floating point fields at any depth.
     */
    bool isClose(ParameterType rhs) const;

    static int encode(ParameterType self, ::uavcan::ScalarCodec& codec,
                      ::uavcan::TailArrayOptimizationMode tao_mode = ::uavcan::TailArrayOptEnabled);

    static int decode(ReferenceType self, ::uavcan::ScalarCodec& codec,
                      ::uavcan::TailArrayOptimizationMode tao_mode = ::uavcan::TailArrayOptEnabled);

    /*
     * Static type info
     */
    enum { DataTypeKind = ::uavcan::DataTypeKindMessage };
    enum { DefaultDataTypeID = 341 };

    static const char* getDataTypeFullName()
    {
        return "uavcan.protocol.NodeStatus";
    }

    static void extendDataTypeSignature(::uavcan::DataTypeSignature& signature)
    {
        signature.extend(getDataTypeSignature());
    }

    static ::uavcan::DataTypeSignature getDataTypeSignature();

};

/*
 * Out of line struct method definitions
 */

template <int _tmpl>
bool NodeStatus_<_tmpl>::operator==(ParameterType rhs) const
{
    return
        uptime_sec == rhs.uptime_sec &&
        health == rhs.health &&
        mode == rhs.mode &&
        sub_mode == rhs.sub_mode &&
        vendor_specific_status_code == rhs.vendor_specific_status_code;
}

template <int _tmpl>
bool NodeStatus_<_tmpl>::isClose(ParameterType rhs) const
{
    return
        ::uavcan::areClose(uptime_sec, rhs.uptime_sec) &&
        ::uavcan::areClose(health, rhs.health) &&
        ::uavcan::areClose(mode, rhs.mode) &&
        ::uavcan::areClose(sub_mode, rhs.sub_mode) &&
        ::uavcan::areClose(vendor_specific_status_code, rhs.vendor_specific_status_code);
}

template <int _tmpl>
int NodeStatus_<_tmpl>::encode(ParameterType self, ::uavcan::ScalarCodec& codec,
    ::uavcan::TailArrayOptimizationMode tao_mode)
{
    (void)self;
    (void)codec;
    (void)tao_mode;
    int res = 1;
    res = FieldTypes::uptime_sec::encode(self.uptime_sec, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::health::encode(self.health, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::mode::encode(self.mode, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::sub_mode::encode(self.sub_mode, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::vendor_specific_status_code::encode(self.vendor_specific_status_code, codec,  tao_mode);
    return res;
}

template <int _tmpl>
int NodeStatus_<_tmpl>::decode(ReferenceType self, ::uavcan::ScalarCodec& codec,
    ::uavcan::TailArrayOptimizationMode tao_mode)
{
    (void)self;
    (void)codec;
    (void)tao_mode;
    int res = 1;
    res = FieldTypes::uptime_sec::decode(self.uptime_sec, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::health::decode(self.health, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::mode::decode(self.mode, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::sub_mode::decode(self.sub_mode, codec,  ::uavcan::TailArrayOptDisabled);
    if (res <= 0)
    {
        return res;
    }
    res = FieldTypes::vendor_specific_status_code::decode(self.vendor_specific_status_code, codec,  tao_mode);
    return res;
}

/*
 * Out of line type method definitions
 */
template <int _tmpl>
::uavcan::DataTypeSignature NodeStatus_<_tmpl>::getDataTypeSignature()
{
    ::uavcan::DataTypeSignature signature(0xF0868D0C1A7C6F1ULL);

    FieldTypes::uptime_sec::extendDataTypeSignature(signature);
    FieldTypes::health::extendDataTypeSignature(signature);
    FieldTypes::mode::extendDataTypeSignature(signature);
    FieldTypes::sub_mode::extendDataTypeSignature(signature);
    FieldTypes::vendor_specific_status_code::extendDataTypeSignature(signature);

    return signature;
}

/*
 * Out of line constant definitions
 */

template <int _tmpl>
const typename ::uavcan::StorageType< typename NodeStatus_<_tmpl>::ConstantTypes::MAX_BROADCASTING_PERIOD_MS >::Type
    NodeStatus_<_tmpl>::MAX_BROADCASTING_PERIOD_MS = 1000U; // 1000

template <int _tmpl>
const typename ::uavcan::StorageType< typename NodeStatus_<_tmpl>::ConstantTypes::MIN_BROADCASTING_PERIOD_MS >::Type
    NodeStatus_<_tmpl>::MIN_BROADCASTING_PERIOD_MS = 2U; // 2

template <int _tmpl>
const typename ::uavcan::StorageType< typename NodeStatus_<_tmpl>::ConstantTypes::OFFLINE_TIMEOUT_MS >::Type
    NodeStatus_<_tmpl>::OFFLINE_TIMEOUT_MS = 3000U; // 3000

template <int _tmpl>
const typename ::uavcan::StorageType< typename NodeStatus_<_tmpl>::ConstantTypes::HEALTH_OK >::Type
    NodeStatus_<_tmpl>::HEALTH_OK = 0U; // 0

template <int _tmpl>
const typename ::uavcan::StorageType< typename NodeStatus_<_tmpl>::ConstantTypes::HEALTH_WARNING >::Type
    NodeStatus_<_tmpl>::HEALTH_WARNING = 1U; // 1

template <int _tmpl>
const typename ::uavcan::StorageType< typename NodeStatus_<_tmpl>::ConstantTypes::HEALTH_ERROR >::Type
    NodeStatus_<_tmpl>::HEALTH_ERROR = 2U; // 2

template <int _tmpl>
const typename ::uavcan::StorageType< typename NodeStatus_<_tmpl>::ConstantTypes::HEALTH_CRITICAL >::Type
    NodeStatus_<_tmpl>::HEALTH_CRITICAL = 3U; // 3

template <int _tmpl>
const typename ::uavcan::StorageType< typename NodeStatus_<_tmpl>::ConstantTypes::MODE_OPERATIONAL >::Type
    NodeStatus_<_tmpl>::MODE_OPERATIONAL = 0U; // 0

template <int _tmpl>
const typename ::uavcan::StorageType< typename NodeStatus_<_tmpl>::ConstantTypes::MODE_INITIALIZATION >::Type
    NodeStatus_<_tmpl>::MODE_INITIALIZATION = 1U; // 1

template <int _tmpl>
const typename ::uavcan::StorageType< typename NodeStatus_<_tmpl>::ConstantTypes::MODE_MAINTENANCE >::Type
    NodeStatus_<_tmpl>::MODE_MAINTENANCE = 2U; // 2

template <int _tmpl>
const typename ::uavcan::StorageType< typename NodeStatus_<_tmpl>::ConstantTypes::MODE_SOFTWARE_UPDATE >::Type
    NodeStatus_<_tmpl>::MODE_SOFTWARE_UPDATE = 3U; // 3

template <int _tmpl>
const typename ::uavcan::StorageType< typename NodeStatus_<_tmpl>::ConstantTypes::MODE_OFFLINE >::Type
    NodeStatus_<_tmpl>::MODE_OFFLINE = 7U; // 7

/*
 * Final typedef
 */
typedef NodeStatus_<0> NodeStatus;

namespace
{

const ::uavcan::DefaultDataTypeRegistrator< ::uavcan::protocol::NodeStatus > _uavcan_gdtr_registrator_NodeStatus;

}

} // Namespace protocol
} // Namespace uavcan

/*
 * YAML streamer specialization
 */
namespace uavcan
{

template <>
class UAVCAN_EXPORT YamlStreamer< ::uavcan::protocol::NodeStatus >
{
public:
    template <typename Stream>
    static void stream(Stream& s, ::uavcan::protocol::NodeStatus::ParameterType obj, const int level);
};

template <typename Stream>
void YamlStreamer< ::uavcan::protocol::NodeStatus >::stream(Stream& s, ::uavcan::protocol::NodeStatus::ParameterType obj, const int level)
{
    (void)s;
    (void)obj;
    (void)level;
    if (level > 0)
    {
        s << '\n';
        for (int pos = 0; pos < level; pos++)
        {
            s << "  ";
        }
    }
    s << "uptime_sec: ";
    YamlStreamer< ::uavcan::protocol::NodeStatus::FieldTypes::uptime_sec >::stream(s, obj.uptime_sec, level + 1);
    s << '\n';
    for (int pos = 0; pos < level; pos++)
    {
        s << "  ";
    }
    s << "health: ";
    YamlStreamer< ::uavcan::protocol::NodeStatus::FieldTypes::health >::stream(s, obj.health, level + 1);
    s << '\n';
    for (int pos = 0; pos < level; pos++)
    {
        s << "  ";
    }
    s << "mode: ";
    YamlStreamer< ::uavcan::protocol::NodeStatus::FieldTypes::mode >::stream(s, obj.mode, level + 1);
    s << '\n';
    for (int pos = 0; pos < level; pos++)
    {
        s << "  ";
    }
    s << "sub_mode: ";
    YamlStreamer< ::uavcan::protocol::NodeStatus::FieldTypes::sub_mode >::stream(s, obj.sub_mode, level + 1);
    s << '\n';
    for (int pos = 0; pos < level; pos++)
    {
        s << "  ";
    }
    s << "vendor_specific_status_code: ";
    YamlStreamer< ::uavcan::protocol::NodeStatus::FieldTypes::vendor_specific_status_code >::stream(s, obj.vendor_specific_status_code, level + 1);
}

}

namespace uavcan
{
namespace protocol
{

template <typename Stream>
inline Stream& operator<<(Stream& s, ::uavcan::protocol::NodeStatus::ParameterType obj)
{
    ::uavcan::YamlStreamer< ::uavcan::protocol::NodeStatus >::stream(s, obj, 0);
    return s;
}

} // Namespace protocol
} // Namespace uavcan

#endif // UAVCAN_PROTOCOL_NODESTATUS_HPP_INCLUDED