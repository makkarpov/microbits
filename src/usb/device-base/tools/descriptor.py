import datetime
from typing import List, Any, Optional, Set, Union

from descriptor_lib.endpoints import EndpointConstraints, EpConstraintFlag
from descriptor_lib.usb_struct import *

from descriptor_impl.codegen import StaticDescriptorWriter
from descriptor_impl.endpoint_alloc import WrappedEndpoint, EndpointAllocator
from descriptor_impl.wrappers import PluginWrapper, FunctionWrapper, InterfaceWrapper


class _SpecException(Exception):
    def __init__(self, msg: str, *args):
        super().__init__(msg % args)


class _ParsedArguments:
    out: str  # Output file
    lib: List[str]  # Library files to use
    spec: str  # YAML specification file

    def __repr__(self) -> str:
        return repr({x: getattr(self, x) for x in dir(self) if not x.startswith('_')})


class DescriptorGenerator:
    _args: _ParsedArguments
    _spec: dict
    _spec_device: dict
    _plugins: List[PluginWrapper]

    _functions: List[FunctionWrapper]
    _interfaces: List[InterfaceWrapper]
    _ep_alloc: EndpointAllocator

    _dev_descriptor: DeviceDescriptor
    _config_descriptors: List[Any]
    _string_descriptors: List[Union[StringDescriptor, StringLangDescriptor, None]]
    _serial_string_idx: Optional[int]

    def _parse_args(self):
        from argparse import ArgumentParser
        parser = ArgumentParser()

        parser.add_argument('--out', required=True, help='Output descriptor file')

        parser.add_argument('--lib', action='extend', nargs='+',
                            help='Python library files with specific device classes')

        parser.add_argument('--spec', required=True, help='Descriptor specification file')

        ret = _ParsedArguments()
        ret.lib = []
        self._args = parser.parse_args(namespace=ret)

    def _parse_specifications(self):
        from yaml import safe_load
        from os import path

        spec = self._args.spec

        if not (path.exists(spec) and path.isfile(spec)):
            raise RuntimeError('specification file not found: %s' % spec)

        with open(spec, 'r', encoding='utf8') as f:
            self._spec = safe_load(f)

        spec_device = self._spec.get('device')
        if spec_device is None or not isinstance(spec_device, dict):
            raise _SpecException('\'device\' is missing or is not an object')

        self._spec_device = spec_device

    def _load_plugins(self):
        from hashlib import sha256
        from os import path
        import importlib.util

        self._plugins = []

        for lib_name in self._args.lib:
            if not (path.exists(lib_name) and path.isfile(lib_name)):
                raise RuntimeError('library file does not exists: %s' % lib_name)

            plugin_name = sha256(lib_name.encode('utf8')).hexdigest()[:8]
            spec = importlib.util.spec_from_file_location(plugin_name, lib_name)
            plugin_mod = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(plugin_mod)

            if not hasattr(plugin_mod, 'DescriptorPlugin'):
                raise RuntimeError('No \'DescriptorPlugin\' class defined in module %s' % lib_name)

            plugin_impl = plugin_mod.DescriptorPlugin()
            self._plugins.append(PluginWrapper(lib_name, plugin_impl))

    def _resolve_functions(self):
        self._functions = []

        for i, fn in enumerate(self._spec.get('functions', [])):
            if not isinstance(fn, dict):
                raise _SpecException('function #%d is not an object', i)

            fn_type = fn.get('type')
            if fn_type is None or not isinstance(fn_type, str):
                raise _SpecException('function #%d \'type\' field is missing or has invalid type', i)

            matched_plugin = None
            for pl in self._plugins:
                if fn_type not in pl.function_types:
                    continue

                matched_plugin = pl
                break

            if matched_plugin is None:
                raise _SpecException('function #%d has invalid type %s - plugin not found', i, repr(fn_type))

            self._functions.append(matched_plugin.create_function(i, fn_type, fn))

    def _allocate_interfaces(self):
        self._interfaces = []

        for fn in self._functions:
            fn.interface_base = len(self._interfaces)
            fn.interfaces = {}

            for i, itf in enumerate(fn.fn.interfaces):
                physical_id = len(self._interfaces)
                fn.interfaces[itf.name] = physical_id
                self._interfaces.append(InterfaceWrapper(fn.id, i, physical_id, itf))

    def _allocate_endpoints(self):
        all_endpoints = []

        for fn in self._functions:
            eps = fn.fn.endpoints

            name_set = set(x.name for x in eps)
            if len(name_set) != len(eps):
                raise RuntimeError(f'function #{fn.id} has endpoints with duplicate names: {repr(eps)}')

            fn.fn.set_environment(fn)
            fn.endpoints = [WrappedEndpoint(fn.id, fn.name, x) for x in eps]
            fn.named_endpoints = {x.endpoint.name: x for x in fn.endpoints}

            if len(fn.endpoints) == 0:
                fn.n_logical_endpoints = 0
            else:
                fn.n_logical_endpoints = max(x.endpoint.logical_addr for x in fn.endpoints) + 1

            all_endpoints.extend(fn.endpoints)

        self._max_func_endpoints = max(f.n_logical_endpoints for f in self._functions)
        self._max_func_interfaces = max(len(f.interfaces) for f in self._functions)

        constraints = EndpointConstraints(
            flags=EpConstraintFlag.IN_OUT_SAME_TYPE | EpConstraintFlag.DBL_BUF_UNIDIRECTIONAL,
            max_in_ep=6,
            max_out_ep=6
        )

        self._ep_alloc = EndpointAllocator(constraints, all_endpoints)

    def _prepare_config_descriptor(self):
        for fn in self._functions:
            self._config_descriptors.extend(fn.fn.config_descriptors)

        total_len = ConfigurationDescriptor.SERIALIZED_LENGTH + sum(len(bytes(x)) for x in self._config_descriptors)
        self._config_descriptors.insert(0, ConfigurationDescriptor(
            wTotalLength=total_len,
            bNumInterfaces=len(self._interfaces),
            bConfigurationValue=1,
            iConfiguration=0,
            bmAttributes=0,
            bMaxPower=250
        ))

    def _prepare_device_descriptor(self):
        max_packet_size0 = int(self._spec_device.get('maxControlPacket', 64))
        if max_packet_size0 not in {8, 16, 32, 64}:
            raise _SpecException('\'device.maxControlPacket\' must be 8, 16, 32 or 64 bytes per USB specification')

        id_vendor = self._spec_device.get('vendorId')
        if id_vendor is None or not isinstance(id_vendor, int):
            raise _SpecException('\'device.vendorId\' is undefined or is not an integer')

        id_product = self._spec_device.get('productId')
        if id_product is None or not isinstance(id_product, int):
            raise _SpecException('\'device.productId\' is undefined or is not an integer')

        self._string_descriptors.append(StringLangDescriptor([0x0409]))  # hardcode English for now

        manufacturer_str = self._spec_device.get('manufacturer')
        i_manufacturer = 0
        if manufacturer_str is not None:
            i_manufacturer = len(self._string_descriptors)
            self._string_descriptors.append(StringDescriptor(str(manufacturer_str)))

        product_str = self._spec_device.get('product')
        i_product = 0
        if product_str is not None:
            i_product = len(self._string_descriptors)
            self._string_descriptors.append(StringDescriptor(str(product_str)))

        serial_str = self._spec_device.get('serial')
        self._serial_string_idx = None
        i_serial = 0
        if serial_str is not None:
            i_serial = len(self._string_descriptors)
            serial_str = str(serial_str)
            if serial_str.lower() in {'dynamic', 'runtime'}:
                self._serial_string_idx = i_serial
                self._string_descriptors.append(None)
            else:
                self._string_descriptors.append(StringDescriptor(serial_str))

        have_iads = any(isinstance(x, InterfaceAssociationDescriptor) for x in self._config_descriptors)

        self._dev_descriptor = DeviceDescriptor(
            bcdUSB=0x0200,
            bDeviceClass=0xEF if have_iads else 0x00,
            bDeviceSubClass=0x02 if have_iads else 0x00,
            bDeviceProtocol=0x01 if have_iads else 0x00,
            bMaxPacketSize0=max_packet_size0,
            idVendor=id_vendor,
            idProduct=id_product,
            bcdDevice=0x0200,
            iManufacturer=i_manufacturer,
            iProduct=i_product,
            iSerialNumber=i_serial,
            iNumConfigurations=1
        )

    def _generate_header_comment(self) -> str:
        from datetime import datetime
        from yaml import safe_dump

        separator = '\n' + '=' * 117 + '\n\n'

        time_str = datetime.now().astimezone().strftime('%d.%m.%Y %H:%M:%S %Z')
        r = 'Auto-generated by USB descriptor compiler at %s.\n' % time_str
        r += 'Do not modify this file, your changes will be overwritten.\n\n'

        r += 'SUGGESTED CONFIGURATION PARAMETERS'
        r += separator

        r += '#define UB_USBD_MAX_IN_ENDPOINTS          %d\n' % self._ep_alloc.num_in
        r += '#define UB_USBD_MAX_OUT_ENDPOINTS         %d\n' % self._ep_alloc.num_out
        r += '#define UB_USBD_MAX_INTERFACES            %d\n' % len(self._interfaces)
        r += '#define UB_USBD_MAX_FUNCTIONS             %d\n' % len(self._functions)
        r += '#define UB_USBD_MAX_FUNC_ENDPOINTS        %d\n' % self._max_func_endpoints
        r += '#define UB_USBD_MAX_FUNC_INTERFACES       %d\n' % self._max_func_interfaces
        r += '#define UB_USBD_MAX_CONTROL_PACKET        %d\n' % self._dev_descriptor.bMaxPacketSize0
        r += '\n\n'

        r += 'INTERFACE NUMBER ASSIGNMENT'
        r += separator

        for i, itf in enumerate(self._interfaces):
            func_name = self._functions[itf.func_id].name
            r += '%-3d -> %s.%s\n' % (i, func_name, itf.itf.name)
        r += '\n\n'

        r += 'ENDPOINT ASSIGNMENT'
        r += separator

        r += self._ep_alloc.pretty_print()

        return r

    def _write_output_file(self):
        if 'name' not in self._spec:
            raise _SpecException('\'name\' field is missing')

        includes = list(self._spec.get('includes', []))
        for fn in self._functions:
            includes.extend(fn.fn.extra_includes)

        with StaticDescriptorWriter(open(self._args.out, 'w', encoding='utf8')) as w:
            w.write_header_comment(self._generate_header_comment())

            w.write_header(includes)

            w.write_static_checks(
                num_functions=len(self._functions),
                num_in_endpoints=self._ep_alloc.num_in,
                num_out_endpoints=self._ep_alloc.num_out,
                num_fn_endpoints=self._max_func_endpoints,
                num_interfaces=len(self._interfaces),
                num_fn_interfaces=self._max_func_interfaces,
                max_control_packet=self._dev_descriptor.bMaxPacketSize0
            )

            for f in self._functions:
                w.write_extra_check(f.fn.extra_static_checks)

            w.write_device_descriptor(bytes(self._dev_descriptor))
            w.write_config_descriptor(b''.join(bytes(x) for x in self._config_descriptors))

            str_indexed = [(i, bytes(x)) for i, x in enumerate(self._string_descriptors) if x is not None]
            w.write_string_descriptors(str_indexed, self._serial_string_idx)

            w.write_endpoint_mapping(self._ep_alloc, self._interfaces)
            w.write_functions(self._ep_alloc, self._functions)
            w.write_descriptor_defn(self._spec['name'])

    def run(self):
        self._string_descriptors = []
        self._config_descriptors = []

        self._parse_args()
        self._parse_specifications()
        self._load_plugins()
        self._resolve_functions()

        self._allocate_interfaces()
        self._allocate_endpoints()

        self._prepare_config_descriptor()
        self._prepare_device_descriptor()
        self._write_output_file()


if __name__ == '__main__':
    DescriptorGenerator().run()
