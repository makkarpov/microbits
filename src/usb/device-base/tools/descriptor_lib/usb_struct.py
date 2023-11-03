from typing import Tuple, NamedTuple, List, Union
from struct import pack as _pack


__all__ = [
    'EP_IN', 'EP_OUT', 'DescriptorType', 'serialize_descriptor', 'serialize_tuple', 'DeviceDescriptor',
    'ConfigurationDescriptor', 'InterfaceDescriptor', 'EndpointDescriptor', 'EndpointAttributes',
    'InterfaceAssociationDescriptor', 'StringLangDescriptor', 'StringDescriptor'
]


EP_IN = 0x80
EP_OUT = 0x00


class DescriptorType:
    DEVICE = 0x01
    CONFIGURATION = 0x02
    STRING = 0x03
    INTERFACE = 0x04
    ENDPOINT = 0x05
    DEVICE_QUALIFIER = 0x06
    OTHER_SPEED = 0x07
    INTERFACE_PWR = 0x08
    INTERFACE_ASSOC = 0x0B


def serialize_descriptor(descriptor_type: int, data: bytes) -> bytes:
    # Each descriptor has common header fields:
    #  uint8_t bLength (including header)
    #  uint8_t bDescriptorType
    return _pack('<2B', len(data) + 2, descriptor_type) + data


def serialize_tuple(descriptor_type: int, fmt: str, *data: Union[Tuple, int]) -> bytes:
    data_flat = []
    for x in data:
        if isinstance(x, int):
            data_flat.append(x)
        else:
            data_flat.extend(x)

    data_bytes = _pack('<' + fmt, *data_flat)
    return serialize_descriptor(descriptor_type, data_bytes)


class DeviceDescriptor(NamedTuple):
    bcdUSB: int                 # uint16_t bcdUsb               - USB version number in BCD format
    bDeviceClass: int           # uint8_t  bDeviceClass         - Device class identifier
    bDeviceSubClass: int        # uint8_t  bDeviceSubClass      - Device subclass identifier
    bDeviceProtocol: int        # uint8_t  bDeviceProtocol      - Device protocol identifier
    bMaxPacketSize0: int        # uint8_t  bMaxPacketSize0      - Maximum packet size for control endpoint
    idVendor: int               # uint16_t idVendor             - Vendor identifier
    idProduct: int              # uint16_t idProduct            - Product identifier
    bcdDevice: int              # uint16_t bcdDevice            - Device version number in BCD format
    iManufacturer: int          # uint8_t  iManufacturer        - Index of manufacturer string descriptor
    iProduct: int               # uint8_t  iProduct             - Index of product string descriptor
    iSerialNumber: int          # uint8_t  iSerialNumber        - Index of serial number string descriptor
    iNumConfigurations: int     # uint8_t  iNumConfigurations   - Number of configurations in the device

    def __bytes__(self):
        return serialize_tuple(DescriptorType.DEVICE, 'H4B3H4B', self)


class ConfigurationDescriptor(NamedTuple):
    wTotalLength: int           # uint16_t wTotalLength         - Total length of configuration descriptor data
    bNumInterfaces: int         # uint8_t  bNumInterfaces       - Number of interfaces in the device
    bConfigurationValue: int    # uint8_t  bConfigurationValue  - Value to use when selecting this configuration
    iConfiguration: int         # uint8_t  iConfiguration       - Index of configuration's string descriptor
    bmAttributes: int           # uint8_t  bmAttributes         - Attributes bitmask
    bMaxPower: int              # uint8_t  bMaxPower            - Maximum power specified in 2mA units

    SERIALIZED_LENGTH = 9  # serialized length of this descriptor, including common header

    def __bytes__(self):
        return serialize_tuple(DescriptorType.CONFIGURATION, 'H5B', self)


class InterfaceDescriptor(NamedTuple):
    bInterfaceNumber: int       # uint8_t  bInterfaceNumber     - Index of interface being described
    bAlternateSetting: int      # uint8_t  bAlternateSetting
    bNumEndpoints: int          # uint8_t  bNumEndpoints        - Number of endpoints in the interface
    bInterfaceClass: int        # uint8_t  bInterfaceClass      - Interface class identifier
    bInterfaceSubClass: int     # uint8_t  bInterfaceSubClass   - Interface subclass identifier
    bInterfaceProtocol: int     # uint8_t  bInterfaceProtocol   - Interface protocol identifier
    iInterface: int             # uint8_t  iInterface           - Index of interface's string descriptor

    def __bytes__(self) -> bytes:
        return serialize_tuple(DescriptorType.INTERFACE, '7B', self)


class EndpointAttributes:
    TYPE_CONTROL = 0x00         # Control endpoint type
    TYPE_ISOCHRONOUS = 0x01     # Isochronous endpoint type
    TYPE_BULK = 0x02            # Bulk endpoint type
    TYPE_INTERRUPT = 0x03       # Interrupt endpoint type

    SYNC_NONE = 0x00
    SYNC_ASYNC = 0x01
    SYNC_ADAPTIVE = 0x02
    SYNC_SYNC = 0x03

    USAGE_DATA = 0x00
    USAGE_FEEDBACK = 0x01
    USAGE_IMPLICIT_FB = 0x02

    _TYPE_MASK = 0x03
    _SYNC_MASK = 0x03
    _SYNC_SHIFT = 2
    _USAGE_MASK = 0x03
    _USAGE_SHIFT = 4

    @staticmethod
    def make(ep_type: int, sync: int = 0, usage: int = 0) -> int:
        if (ep_type & ~EndpointAttributes._TYPE_MASK) != 0:
            raise ValueError('ep_type field is out of range')

        if (sync & ~EndpointAttributes._SYNC_MASK) != 0:
            raise ValueError('sync field is out of range')

        if (usage & ~EndpointAttributes._USAGE_MASK) != 0:
            raise ValueError('usage field is out of range')

        return ep_type | (sync << EndpointAttributes._SYNC_SHIFT) | (usage << EndpointAttributes._USAGE_SHIFT)


class EndpointDescriptor(NamedTuple):
    bEndpointAddress: int       # uint8_t  bEndpointAddress     - Address of described endpoint (with direction flag)
    bmAttributes: int           # uint8_t  bmAttributes         - Endpoint attribute bitmask
    wMaxPacketSize: int         # uint16_t wMaxPacketSize       - Maximum packet size for this endpoint
    bInterval: int              # uint8_t  bInterval            - Polling interface for interrupt endpoints

    @staticmethod
    def make_in(addr: int, ep_type: int, packet_size: int) -> 'EndpointDescriptor':
        return EndpointDescriptor(EP_IN | addr, EndpointAttributes.make(ep_type), packet_size, 0)

    @staticmethod
    def make_out(addr: int, ep_type: int, packet_size: int) -> 'EndpointDescriptor':
        return EndpointDescriptor(EP_OUT | addr, EndpointAttributes.make(ep_type), packet_size, 0)

    def __bytes__(self) -> bytes:
        return serialize_tuple(DescriptorType.ENDPOINT, '2BHB', self)


class InterfaceAssociationDescriptor(NamedTuple):
    bFirstInterface: int        # uint8_t  bFirstInterface      - Index of the first interface in association
    bInterfaceCount: int        # uint8_t  bInterfaceCount      - Number of associated interfaces
    bFunctionClass: int         # uint8_t  bFunctionClass       - Function class identifier
    bFunctionSubClass: int      # uint8_t  bFunctionSubClass    - Function subclass identifier
    bFunctionProtocol: int      # uint8_t  bFunctionProtocol    - Function protocol identifier
    iFunction: int              # uint8_t  iFunction            - Index of function's string descriptor

    def __bytes__(self) -> bytes:
        return serialize_tuple(DescriptorType.INTERFACE_ASSOC, '6B', self)


class StringLangDescriptor(NamedTuple):
    wLanguageIds: List[int]     # uint16_t wLanguageIds[]       - Language identifiers available for string descriptors

    def __bytes__(self) -> bytes:
        data = b''.join(_pack('<H', x) for x in self.wLanguageIds)
        return serialize_descriptor(DescriptorType.STRING, data)


class StringDescriptor(NamedTuple):
    wString: str                # uint16_t wString[]            - Actual UTF-16LE encoded string contents

    def __bytes__(self) -> bytes:
        return serialize_descriptor(DescriptorType.STRING, self.wString.encode('utf-16le'))
