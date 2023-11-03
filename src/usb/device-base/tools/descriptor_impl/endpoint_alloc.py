from typing import List, Optional
from enum import Enum

from descriptor_lib.endpoints import *
from descriptor_lib.usb_struct import *


_EP_IN = 0x80


class WrappedEndpoint:
    func_id: int
    func_name: str
    endpoint: EndpointDefinition
    address: Optional[int]

    def __init__(self, func_id: int, func_name: str, endpoint: EndpointDefinition):
        self.func_id = func_id
        self.func_name = func_name
        self.endpoint = endpoint
        self.address = None

    @property
    def name(self) -> str:
        return '%s.%s' % (self.func_name, self.endpoint.name)

    @property
    def descriptor(self) -> EndpointDescriptor:
        if self.address is None:
            raise RuntimeError('address is not yet assigned: %s' % self.name)

        return EndpointDescriptor(
            bEndpointAddress=self.address,
            bmAttributes=EndpointAttributes.make(
                ep_type=self.endpoint.type.value,
                sync=int((self.endpoint.sync or EndpointSync.NONE).value),
                usage=int((self.endpoint.usage or EndpointUsage.DATA).value)
            ),
            wMaxPacketSize=self.endpoint.max_packet,
            bInterval=self.endpoint.poll_interval
        )

    def __repr__(self):
        return (f'WrappedEndpoint(\'{self.name}\', type={self.endpoint.type.name}, '
                f'dir={self.endpoint.dir.name})')

    def __hash__(self) -> int:
        return (hash(self.endpoint.name) * 31 + self.func_id) & ((1 << 64) - 1)


class EndpointAllocationException(RuntimeError):
    def __init__(self, msg: str):
        super().__init__(msg)


class EndpointStatus(Enum):
    EMPTY = 0       # Endpoint is free to be used
    CONTROL = 1     # Endpoint is used for control transfers
    INVALID = 2     # Endpoint cannot be used due to constraints
    DATA = 3        # Endpoint is used for data


class EndpointState:
    addr: int
    status: EndpointStatus
    assignment: Optional[WrappedEndpoint]
    type: Optional[EndpointType]

    def __init__(self, addr: int):
        self.addr = addr
        self.status = EndpointStatus.EMPTY
        self.assignment = None
        self.type = None

    def __repr__(self):
        t_name = self.type.name if self.type is not None else 'NONE'
        assgn = self.assignment.name if self.assignment is not None else 'NONE'

        return 'EP(addr=%02X, st=%s, tp=%s, a=%s)' % (self.addr, self.status.name, t_name, assgn)


class EndpointAllocator:
    def __init__(self, constraints: EndpointConstraints, endpoints: List[WrappedEndpoint]):
        self._constraints = constraints

        self.st_in = [EndpointState(_EP_IN | x) for x in range(constraints.max_in_ep)]
        self.st_out = [EndpointState(x) for x in range(constraints.max_out_ep)]

        self.st_in[0].status = EndpointStatus.CONTROL
        self.st_in[0].type = EndpointType.CONTROL

        self.st_out[0].status = EndpointStatus.CONTROL
        self.st_out[0].type = EndpointType.CONTROL

        for x in endpoints:
            self._allocate_endpoint(x)

        self.num_in = max(i + 1 for i in range(len(self.st_in)) if self.st_in[i].status != EndpointStatus.EMPTY)
        self.num_out = max(i + 1 for i in range(len(self.st_out)) if self.st_out[i].status != EndpointStatus.EMPTY)

    def _allocate_endpoint(self, ep: WrappedEndpoint):
        arr, arr_rev = (self.st_in, self.st_out) if ep.endpoint.dir == EndpointDirection.IN else (self.st_out, self.st_in)

        for i in range(len(arr)):
            ep_t = arr[i]
            ep_r = arr_rev[i] if i < len(arr_rev) else None

            if ep_t.status != EndpointStatus.EMPTY:
                continue

            if (EpConstraintFlag.IN_OUT_SAME_TYPE in self._constraints.flags and ep_r is not None and
               (ep_r.status != EndpointStatus.EMPTY and (ep_r.status != EndpointStatus.DATA or
                                                         ep_r.type != ep.endpoint.type))):
                continue

            ep_t.status = EndpointStatus.DATA
            ep_t.type = ep.endpoint.type
            ep_t.assignment = ep

            ep.address = ep_t.addr
            return

        raise EndpointAllocationException('failed to allocate endpoint %s - no suitable slot found\n\n%s' %
                                          (ep.name, self.pretty_print()))

    @staticmethod
    def _print_endpoint(ep: EndpointState) -> str:
        ret = '%-7s' % ep.status.name

        if ep.status == EndpointStatus.DATA:
            ret += ' type=%-12s name=%s' % (ep.type.name, repr(ep.assignment.name))

        return ret

    def pretty_print(self) -> str:
        ret = ''

        for i in range(max(self.num_in, self.num_out)):
            if i < len(self.st_out):
                ret += 'OUT [0x%02X]: ' % self.st_out[i].addr
                ret += self._print_endpoint(self.st_out[i])
                ret += '\n'

            if i < len(self.st_in):
                ret += 'IN  [0x%02X]: ' % self.st_in[i].addr
                ret += self._print_endpoint(self.st_in[i])
                ret += '\n'

            ret += '\n'

        return ret

