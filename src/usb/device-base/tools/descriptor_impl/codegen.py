from typing import TextIO, List, Optional, Tuple, NamedTuple

from descriptor_impl.wrappers import FunctionWrapper, InterfaceWrapper
from descriptor_impl.endpoint_alloc import EndpointAllocator, EndpointState, EndpointStatus


DESCRIPTOR_VERSION = 2023110201

_LINE_LENGTH = 120

_DEVICE_DESCRIPTOR_NAME = 'ub_deviceDescriptor'
_CONFIG_DESCRIPTOR_NAME = 'ub_configDescriptor'
_STRING_DATA_NAME = 'ub_stringData%d'
_STRING_DESCRIPTORS_NAME = 'ub_stringDescriptors'
_ENDPOINT_DATA = 'ub_endpoints'
_FUNCTION_DATA = 'ub_functions'
_FUNCTION_EXTRA_DATA = 'ub_functionExtra%d'


class EndpointMappingData(NamedTuple):
    ep_in: Tuple[int, int]  # (base, cnt)
    ep_out: Tuple[int, int]
    itf: Tuple[int, int]


class StaticDescriptorWriter:
    _str_descriptor_cnt: int
    _str_serial: Optional[int]
    _ep_mapping: List[str]
    _ep_cnt: int
    _fn_cnt: int

    def __init__(self, f: TextIO):
        self._f = f

    def write(self, s: str):
        self._f.write(s)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self._f.close()

    def write_header(self, includes: List[str]):
        f = self._f

        f.write('#include <ub/usbd/impl/static-descriptor.hpp>\n')
        f.write('#include <ub/usbd/config.hpp>\n')

        for ii in includes:
            f.write('#include <%s>\n' % str(ii))

    def write_header_comment(self, text: str):
        text = text.strip()
        if len(text) == 0:
            return

        f = self._f
        f.write('/*\n')

        for i, s in enumerate(text.split('\n')):
            f.write(' * ' + s + '\n')

        f.write(' */\n\n')

    def _write_config_check(self, param: str, min_value: int):
        f = self._f

        f.write('\n#if %s < %d\n' % (param, min_value))
        f.write('#error "Configuration value %s is too low. Required at least %d"\n' % (param, min_value))
        f.write('#endif\n')

    def write_static_checks(
            self,
            num_functions: int,
            num_in_endpoints: int,
            num_out_endpoints: int,
            num_fn_endpoints: int,
            num_interfaces: int,
            num_fn_interfaces: int,
            max_control_packet: int
    ):
        f = self._f

        f.write('\n#if UB_USBD_STATIC_DESCRIPTOR_VERSION != %d\n' % DESCRIPTOR_VERSION)
        f.write('#error "Generated descriptor code does not match current library version"\n')
        f.write('#endif\n')

        self._write_config_check('UB_USBD_MAX_IN_ENDPOINTS', num_in_endpoints)
        self._write_config_check('UB_USBD_MAX_OUT_ENDPOINTS', num_out_endpoints)
        self._write_config_check('UB_USBD_MAX_INTERFACES', num_interfaces)
        self._write_config_check('UB_USBD_MAX_FUNCTIONS', num_functions)
        self._write_config_check('UB_USBD_MAX_FUNC_ENDPOINTS', num_fn_endpoints)
        self._write_config_check('UB_USBD_MAX_FUNC_INTERFACES', num_fn_interfaces)

        f.write('\n#if UB_USBD_MAX_CONTROL_PACKET != %d\n' % max_control_packet)
        f.write('#error "Configuration value UB_USBD_MAX_CONTROL_PACKET is inconsistent: must be exactly %d"\n' %
                max_control_packet)
        f.write('#endif\n')

    def write_extra_check(self, s: Optional[str]):
        f = self._f

        if s is None:
            return

        f.write('\n')
        f.write(s)

        if not s.endswith('\n'):
            f.write('\n')

    def write_device_descriptor(self, data: bytes):
        self._write_bytes(_DEVICE_DESCRIPTOR_NAME, data)

    def write_config_descriptor(self, data: bytes):
        self._write_bytes(_CONFIG_DESCRIPTOR_NAME, data)

    def write_string_descriptors(self, data: List[Tuple[int, bytes]], serial_number_idx: Optional[int]):
        for i, s in data:
            self._write_bytes(_STRING_DATA_NAME % i, s)

        f = self._f
        f.write('\nstatic const ::ub::usbd::descriptor::StringDescriptor %s[] {\n' % _STRING_DESCRIPTORS_NAME)

        for i, s in data:
            f.write('  { %d, %s },\n' % (i, _STRING_DATA_NAME % i))

        f.write('};\n')

        self._str_descriptor_cnt = len(data)
        self._str_serial = serial_number_idx

    @staticmethod
    def _format_bytes(arr) -> str:
        return '{' + ', '.join('0x%02X' % x for x in arr) + '}'

    @staticmethod
    def _format_mapping(arr: List[Optional[Tuple[int, int]]]) -> str:
        r = []
        for x in arr:
            if x is None:
                r.append(0)
            else:
                f, v = x
                r.append(((f + 1) << 4) | (v & 0xF))

        return StaticDescriptorWriter._format_bytes(r)

    @staticmethod
    def _format_endpoint_map(st: List[EndpointState], ep_cnt: int) -> str:
        r = []

        for ep in st[:ep_cnt]:
            if ep.status != EndpointStatus.DATA:
                r.append(None)
                continue

            r.append((ep.assignment.func_id, ep.assignment.endpoint.logical_addr))

        return StaticDescriptorWriter._format_mapping(r)

    def _write_endpoint_config(self, ep: EndpointState):
        if ep.status != EndpointStatus.DATA:
            return

        self._ep_cnt += 1
        self._f.write('  { 0x%02X, ::ub::usbd::EndpointType::%s, %d },\n' %
                      (ep.addr, ep.type.name, ep.assignment.endpoint.max_packet))

    def write_endpoint_mapping(self, alloc: EndpointAllocator, intf: List[InterfaceWrapper]):
        in_map = self._format_endpoint_map(alloc.st_in, alloc.num_in)
        out_map = self._format_endpoint_map(alloc.st_out, alloc.num_out)
        itf_map = self._format_mapping([(x.func_id, x.logical_id) for x in intf])

        self._ep_mapping = [
            '{',
            '  /* in */  ' + in_map + ',',
            '  /* out */ ' + out_map + ',',
            '  /* itf */ ' + itf_map,
            '},'
        ]

        f = self._f

        self._ep_cnt = 0

        f.write('\n#if UB_USBD_HAVE_DATA_ENDPOINTS\n')
        f.write('static const ::ub::usbd::EndpointConfig %s[] {\n' % _ENDPOINT_DATA)

        for ep in alloc.st_in:
            self._write_endpoint_config(ep)

        for ep in alloc.st_out:
            self._write_endpoint_config(ep)

        f.write('};\n')
        f.write('#endif // UB_USBD_HAVE_DATA_ENDPOINTS\n')

    def _write_fn_endpoints(self, alloc: EndpointAllocator, fn: FunctionWrapper):
        f = self._f

        eps = alloc.st_in + alloc.st_out
        eps = [x for x in eps if x.status == EndpointStatus.DATA and x.assignment.func_id == fn.id]
        if len(eps) == 0:
            f.write('    {},\n')
            return

        max_addr = max(x.assignment.endpoint.logical_addr for x in eps) + 1
        eps = {x.assignment.endpoint.logical_addr: x for x in eps}

        arr = []
        for i in range(max_addr):
            if i not in eps:
                arr.append(0xFF)

            arr.append(~eps[i].addr & 0xFF)

        f.write('    %s,\n' % self._format_bytes(arr))

    def write_functions(self, alloc: EndpointAllocator, fns: List[FunctionWrapper]):
        from hashlib import sha256
        f = self._f

        have_extra_data = set()
        fn: FunctionWrapper

        for i, fn in enumerate(fns):
            fn_e = fn.fn.extra_function_data(_FUNCTION_EXTRA_DATA % i)
            if fn_e is None:
                continue

            f.write('\n')
            f.write(fn_e)
            if not fn_e.endswith('\n'):
                f.write('\n')

            have_extra_data.add(i)

        f.write('\nstatic const ::ub::usbd::descriptor::StaticFunction %s[] {\n' % _FUNCTION_DATA)

        for i, fn in enumerate(fns):
            fn_type = int.from_bytes(sha256(fn.fn.function_id.encode('utf8')).digest()[:4], 'big')

            f.write('  /* %d */ {\n' % i)
            f.write('    0x%08X, /* \'%s\' */\n' % (fn_type, fn.fn.function_id))

            if i in have_extra_data:
                f.write('    &%s,\n' % (_FUNCTION_EXTRA_DATA % i))
            else:
                f.write('    nullptr, /* function extra data */\n')

            self._write_fn_endpoints(alloc, fn)
            f.write('    %d,\n' % fn.interface_base)

            f.write('  },\n')

        f.write('};\n')

        self._fn_cnt = len(fns)

    def write_descriptor_defn(self, name: str):
        f = self._f
        [*namespace_parts, leaf_name] = name.split('::')

        namespace = '::'.join(namespace_parts)
        prefix = ''

        f.write('\n')
        if len(namespace) != 0:
            f.write('namespace %s {\n' % namespace)
            prefix = '  '

        f.write(prefix + 'const ::ub::usbd::descriptor::StaticDescriptor %s {\n' % leaf_name)
        f.write(prefix + '  %s,\n' % _DEVICE_DESCRIPTOR_NAME)
        f.write(prefix + '  %s,\n' % _CONFIG_DESCRIPTOR_NAME)
        f.write(prefix + '  %d, /* function cnt */\n' % self._fn_cnt)
        f.write(prefix + '  %d, /* string descriptor count */\n' % self._str_descriptor_cnt)
        f.write(prefix + '  %d, /* serial number index */\n' % (self._str_serial or 0))
        f.write('#if UB_USBD_HAVE_DATA_ENDPOINTS\n')
        f.write(prefix + '  %d, /* endpoint cnt */\n' % self._ep_cnt)
        f.write('#endif // UB_USBD_HAVE_DATA_ENDPOINTS\n')
        f.write(prefix + '  %s,\n' % _STRING_DESCRIPTORS_NAME)
        f.write('#if UB_USBD_HAVE_DATA_ENDPOINTS\n')
        for x in self._ep_mapping:
            f.write(prefix + '  %s\n' % x)
        f.write(prefix + '  %s,\n' % (_ENDPOINT_DATA if self._ep_cnt != 0 else 'nullptr'))
        f.write('#endif // UB_USBD_HAVE_DATA_ENDPOINTS\n')
        f.write(prefix + '  %s\n' % _FUNCTION_DATA)
        f.write(prefix + '};\n')

        if len(namespace) != 0:
            f.write('} // namespace %s\n' % namespace)

    def _write_byte_range(self, name: str, data: bytes):
        f = self._f

        self._write_bytes(name + '_data', data)

        f.write('\nstatic const ::ub::usbd::descriptor::ByteRange %s {\n' % name)
        f.write('  %s_data, %d\n' % (name, len(data)))
        f.write('};\n')

    def _write_bytes(self, name: str, data: bytes):
        f = self._f

        f.write('\nstatic const uint8_t %s[%d] {\n' % (name, len(data)))
        self._write_array_data(data, '0x%02X', 6)
        f.write('};\n')

    def _write_array_data(self, arr, fmt: str, item_len: int, prefix: str= '  '):
        per_line = (_LINE_LENGTH - len(prefix)) // item_len

        i = 0
        while i < len(arr):
            il = min(len(arr) - i, per_line)
            self._f.write(prefix + ', '.join(fmt % x for x in arr[i:i+il]) + ',\n')
            i += il

