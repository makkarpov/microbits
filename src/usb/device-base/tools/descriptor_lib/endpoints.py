from typing import NamedTuple, Optional
from enum import Flag, Enum


__all__ = [
    'EpConstraintFlag', 'EndpointConstraints', 'EndpointType', 'EndpointDirection', 'EndpointDefinition',
    'EndpointSync', 'EndpointUsage'
]


class EpConstraintFlag(Flag):
    IN_OUT_SAME_TYPE = 0x01         # IN and OUT endpoints with same number must share same type
    DBL_BUF_UNIDIRECTIONAL = 0x02   # Double-buffered endpoints must be unidirectional


class EndpointConstraints(NamedTuple):
    flags: EpConstraintFlag         # Constraint flags that must be fulfilled by placement algorithm
    max_in_ep: int                  # Maximum supported number of IN endpoints
    max_out_ep: int                 # Maximum supported number of OUT endpoints


class EndpointType(Enum):
    CONTROL = 0                     # Control endpoint
    ISOCHRONOUS = 1                 # Isochronous endpoint
    BULK = 2                        # Bulk endpoint
    INTERRUPT = 3                   # Interrupt endpoint


class EndpointSync(Enum):
    NONE = 0
    ASYNCHRONOUS = 1
    ADAPTIVE = 2
    SYNCHRONOUS = 3


class EndpointUsage(Enum):
    DATA = 0
    FEEDBACK = 1
    IMPLICIT_FEEDBACK = 2


class EndpointDirection(Enum):
    OUT = 0                         # OUT endpoint direction (from host to device)
    IN = 1                          # IN endpoint direction (from device to host)


class EndpointDefinition(NamedTuple):
    name: str                               # Human-readable endpoint name, must be unique among plugin's endpoints
    dir: EndpointDirection                  # Endpoint direction
    logical_addr: int                       # Logical number of endpoint in the function implementation
    type: EndpointType                      # Endpoint type
    max_packet: int                         # Maximum packet
    poll_interval: int = 1                  # Polling interval for interrupt endpoints
    sync: Optional[EndpointSync] = None     # Endpoint synchronization type for isochronous endpoints
    usage: Optional[EndpointUsage] = None   # Endpoint usage type
