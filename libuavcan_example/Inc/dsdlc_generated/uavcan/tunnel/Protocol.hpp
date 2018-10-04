/*
 * UAVCAN data structure definition for libuavcan.
 *
 * Autogenerated, do not edit.
 *
 * Source file: /home/isthatme/Documents/SPEAR/embedded/common/libuavcan/dsdl/uavcan/tunnel/Protocol.uavcan
 */

#ifndef UAVCAN_TUNNEL_PROTOCOL_HPP_INCLUDED
#define UAVCAN_TUNNEL_PROTOCOL_HPP_INCLUDED

#include <uavcan/build_config.hpp>
#include <uavcan/node/global_data_type_registry.hpp>
#include <uavcan/marshal/types.hpp>

/******************************* Source text **********************************
#
# This enumeration specifies the encapsulated protocol.
# New protocols are likely to be added in the future.
#

uint8 MAVLINK                   = 0     # MAVLink

uint8 protocol
******************************************************************************/

/********************* DSDL signature source definition ***********************
uavcan.tunnel.Protocol
saturated uint8 protocol
******************************************************************************/

#undef protocol
#undef MAVLINK

namespace uavcan
{
namespace tunnel
{

template <int _tmpl>
struct UAVCAN_EXPORT Protocol_
{
    typedef const Protocol_<_tmpl>& ParameterType;
    typedef Protocol_<_tmpl>& ReferenceType;

    struct ConstantTypes
    {
        typedef ::uavcan::IntegerSpec< 8, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > MAVLINK;
    };

    struct FieldTypes
    {
        typedef ::uavcan::IntegerSpec< 8, ::uavcan::SignednessUnsigned, ::uavcan::CastModeSaturate > protocol;
    };

    enum
    {
        MinBitLen
            = FieldTypes::protocol::MinBitLen
    };

    enum
    {
        MaxBitLen
            = FieldTypes::protocol::MaxBitLen
    };

    // Constants
    static const typename ::uavcan::StorageType< typename ConstantTypes::MAVLINK >::Type MAVLINK; // 0

    // Fields
    typename ::uavcan::StorageType< typename FieldTypes::protocol >::Type protocol;

    Protocol_()
        : protocol()
    {
        ::uavcan::StaticAssert<_tmpl == 0>::check();  // Usage check

#if UAVCAN_DEBUG
        /*
         * Cross-checking MaxBitLen provided by the DSDL compiler.
         * This check shall never be performed in user code because MaxBitLen value
         * actually depends on the nested types, thus it is not invariant.
         */
        ::uavcan::StaticAssert<8 == MaxBitLen>::check();
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
    // This type has no default data type ID

    static const char* getDataTypeFullName()
    {
        return "uavcan.tunnel.Protocol";
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
bool Protocol_<_tmpl>::operator==(ParameterType rhs) const
{
    return
        protocol == rhs.protocol;
}

template <int _tmpl>
bool Protocol_<_tmpl>::isClose(ParameterType rhs) const
{
    return
        ::uavcan::areClose(protocol, rhs.protocol);
}

template <int _tmpl>
int Protocol_<_tmpl>::encode(ParameterType self, ::uavcan::ScalarCodec& codec,
    ::uavcan::TailArrayOptimizationMode tao_mode)
{
    (void)self;
    (void)codec;
    (void)tao_mode;
    int res = 1;
    res = FieldTypes::protocol::encode(self.protocol, codec,  tao_mode);
    return res;
}

template <int _tmpl>
int Protocol_<_tmpl>::decode(ReferenceType self, ::uavcan::ScalarCodec& codec,
    ::uavcan::TailArrayOptimizationMode tao_mode)
{
    (void)self;
    (void)codec;
    (void)tao_mode;
    int res = 1;
    res = FieldTypes::protocol::decode(self.protocol, codec,  tao_mode);
    return res;
}

/*
 * Out of line type method definitions
 */
template <int _tmpl>
::uavcan::DataTypeSignature Protocol_<_tmpl>::getDataTypeSignature()
{
    ::uavcan::DataTypeSignature signature(0xA367483C9B920E49ULL);

    FieldTypes::protocol::extendDataTypeSignature(signature);

    return signature;
}

/*
 * Out of line constant definitions
 */

template <int _tmpl>
const typename ::uavcan::StorageType< typename Protocol_<_tmpl>::ConstantTypes::MAVLINK >::Type
    Protocol_<_tmpl>::MAVLINK = 0U; // 0

/*
 * Final typedef
 */
typedef Protocol_<0> Protocol;

// No default registration

} // Namespace tunnel
} // Namespace uavcan

/*
 * YAML streamer specialization
 */
namespace uavcan
{

template <>
class UAVCAN_EXPORT YamlStreamer< ::uavcan::tunnel::Protocol >
{
public:
    template <typename Stream>
    static void stream(Stream& s, ::uavcan::tunnel::Protocol::ParameterType obj, const int level);
};

template <typename Stream>
void YamlStreamer< ::uavcan::tunnel::Protocol >::stream(Stream& s, ::uavcan::tunnel::Protocol::ParameterType obj, const int level)
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
    s << "protocol: ";
    YamlStreamer< ::uavcan::tunnel::Protocol::FieldTypes::protocol >::stream(s, obj.protocol, level + 1);
}

}

namespace uavcan
{
namespace tunnel
{

template <typename Stream>
inline Stream& operator<<(Stream& s, ::uavcan::tunnel::Protocol::ParameterType obj)
{
    ::uavcan::YamlStreamer< ::uavcan::tunnel::Protocol >::stream(s, obj, 0);
    return s;
}

} // Namespace tunnel
} // Namespace uavcan

#endif // UAVCAN_TUNNEL_PROTOCOL_HPP_INCLUDED