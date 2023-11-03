from typing import Set, NamedTuple, List, Optional, Any

from descriptor_lib.plugin import AbstractFunction, AbstractPlugin
from descriptor_lib.usb_struct import *
from descriptor_lib.endpoints import *
from descriptor_lib.interfaces import InterfaceDefinition


class _C:
    CLASS_CDC = 0x02
    SUBCLS_CDC_ACM = 0x02
    CLASS_CDC_DATA = 0x0A

    CDC_INTERFACE_DESCRIPTOR = 0x24
    CDC_DESC_HEADER = 0x00
    CDC_DESC_CALL_MGMT = 0x01
    CDC_DESC_ACM = 0x02
    CDC_DESC_UNION = 0x03


class CDCHeaderDescriptor(NamedTuple):
    bcdCDC: int                 # uint16_t bcdCDC           - CDC specification version

    def __bytes__(self) -> bytes:
        return serialize_tuple(_C.CDC_INTERFACE_DESCRIPTOR, 'BH', _C.CDC_DESC_HEADER, self)


class CDCCallManagementDescriptor(NamedTuple):
    bmCapabilities: int         # uint8_t  bmCapabilities   - Call management capabilities
    bDataInterface: int         # uint8_t  bDataInterface   - Index of data interface

    def __bytes__(self) -> bytes:
        return serialize_tuple(_C.CDC_INTERFACE_DESCRIPTOR, '3B', _C.CDC_DESC_CALL_MGMT, self)


class CDCACMDescriptor(NamedTuple):
    bmCapabilities: int         # uint8_t  bmCapabilities   - Device capabilities

    def __bytes__(self) -> bytes:
        return serialize_tuple(_C.CDC_INTERFACE_DESCRIPTOR, '2B', _C.CDC_DESC_ACM, self)


class CDCUnionDescriptor(NamedTuple):
    bMasterInterface: int       # uint8_t  bMasterInterface - Index of master interface
    bSlaveInterface: int        # uint8_t  bSlaveInterface  - Index of slave interface

    def __bytes__(self) -> bytes:
        return serialize_tuple(_C.CDC_INTERFACE_DESCRIPTOR, '3B', _C.CDC_DESC_UNION, self)


class CDCFunction(AbstractFunction):
    def __init__(self, max_packet_length: int):
        super().__init__()
        self._max_packet_length = max_packet_length

        self.ITF_CONTROL = InterfaceDefinition('control')
        self.ITF_DATA = InterfaceDefinition('data')

        self.EP_NOTIFICATION = EndpointDefinition(
            name='notifications',
            dir=EndpointDirection.IN,
            logical_addr=0,
            type=EndpointType.INTERRUPT,
            max_packet=10
        )

        self.EP_DATA_IN = EndpointDefinition(
            name='dataIn',
            dir=EndpointDirection.IN,
            logical_addr=1,
            type=EndpointType.BULK,
            max_packet=self._max_packet_length
        )

        self.EP_DATA_OUT = EndpointDefinition(
            name='dataOut',
            dir=EndpointDirection.OUT,
            logical_addr=2,
            type=EndpointType.BULK,
            max_packet=self._max_packet_length
        )

    @property
    def function_id(self) -> str:
        return 'microbits.cdc-acm.v1'

    @property
    def extra_includes(self) -> List[str]:
        return ['ub/usbd/serial-config.hpp']

    @property
    def extra_static_checks(self) -> Optional[str]:
        r = ('#if UB_USBD_SERIAL_PACKET_LENGTH < {0}\n'
             '#error "UB_USBD_SERIAL_PACKET_LENGTH is too low: must be at least {0}"\n'
             '#endif')

        return r.format(self._max_packet_length)

    @property
    def endpoints(self) -> List[EndpointDefinition]:
        return [self.EP_NOTIFICATION, self.EP_DATA_IN, self.EP_DATA_OUT]

    @property
    def interfaces(self) -> List[InterfaceDefinition]:
        return [self.ITF_CONTROL, self.ITF_DATA]

    @property
    def config_descriptors(self) -> List[Any]:
        return [
            InterfaceAssociationDescriptor(
                bFirstInterface=self.env.interface_num(self.ITF_CONTROL),
                bInterfaceCount=len(self.interfaces),
                bFunctionClass=_C.CLASS_CDC,
                bFunctionSubClass=_C.SUBCLS_CDC_ACM,
                bFunctionProtocol=0,
                iFunction=0
            ),
            InterfaceDescriptor(
                bInterfaceNumber=self.env.interface_num(self.ITF_CONTROL),
                bAlternateSetting=0,
                bNumEndpoints=1,
                bInterfaceClass=_C.CLASS_CDC,
                bInterfaceSubClass=_C.SUBCLS_CDC_ACM,
                bInterfaceProtocol=0,
                iInterface=0
            ),
            CDCHeaderDescriptor(
                bcdCDC=0x0111
            ),
            CDCCallManagementDescriptor(
                bmCapabilities=0,
                bDataInterface=self.env.interface_num(self.ITF_DATA)
            ),
            CDCACMDescriptor(
                bmCapabilities=0
            ),
            CDCUnionDescriptor(
                bMasterInterface=self.env.interface_num(self.ITF_CONTROL),
                bSlaveInterface=self.env.interface_num(self.ITF_DATA)
            ),
            self.env.ep_descriptor(self.EP_NOTIFICATION),
            InterfaceDescriptor(
                bInterfaceNumber=self.env.interface_num(self.ITF_DATA),
                bAlternateSetting=0,
                bNumEndpoints=2,
                bInterfaceClass=_C.CLASS_CDC_DATA,
                bInterfaceSubClass=0,
                bInterfaceProtocol=0,
                iInterface=0
            ),
            self.env.ep_descriptor(self.EP_DATA_IN),
            self.env.ep_descriptor(self.EP_DATA_OUT)
        ]


class DescriptorPlugin(AbstractPlugin):
    def function_types(self) -> Set[str]:
        return {'cdc-acm', 'ub.cdc-acm', 'serial'}

    def create_function(self, _fn_type: str, spec: dict):
        max_packet_length = int(spec.get('maxPacketLength', 64))
        return CDCFunction(max_packet_length)
