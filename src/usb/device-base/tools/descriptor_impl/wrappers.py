from typing import Set, Union, List, Dict, NamedTuple

from descriptor_impl.endpoint_alloc import WrappedEndpoint

from descriptor_lib.endpoints import EndpointDefinition
from descriptor_lib.interfaces import InterfaceDefinition
from descriptor_lib.plugin import AbstractFunction, AbstractPlugin, FunctionEnvironment
from descriptor_lib.usb_struct import EndpointDescriptor


class InterfaceWrapper(NamedTuple):
    func_id: int
    logical_id: int
    physical_id: int
    itf: InterfaceDefinition


class FunctionWrapper(FunctionEnvironment):
    id: int
    plugin: 'PluginWrapper'
    fn: AbstractFunction

    endpoints: List[WrappedEndpoint]
    named_endpoints: Dict[str, WrappedEndpoint]
    n_logical_endpoints: int

    interface_base: int
    interfaces: Dict[str, int]

    def __init__(self, fn_id: int, plugin: 'PluginWrapper', fn: AbstractFunction):
        self.id = fn_id
        self.name = 'fn' + str(fn_id)
        self.plugin = plugin
        self.fn = fn

    def ep_address(self, ep: Union[EndpointDefinition, str]) -> int:
        if isinstance(ep, EndpointDefinition):
            ep = ep.name

        addr = self.named_endpoints[ep].address
        if addr is None:
            raise RuntimeError('endpoint address is not yet allocated: %s' % ep)

        return addr

    def ep_descriptor(self, ep: Union[EndpointDefinition, str]) -> EndpointDescriptor:
        if isinstance(ep, EndpointDefinition):
            ep = ep.name

        return self.named_endpoints[ep].descriptor

    def interface_num(self, itf: Union[InterfaceDefinition, str]) -> int:
        if isinstance(itf, InterfaceDefinition):
            itf = itf.name

        return self.interfaces[itf]

    def __repr__(self) -> str:
        return '<%s at %s>' % (repr(self.fn), self.plugin.filename)


class PluginWrapper:
    _filename: str
    _plugin: AbstractPlugin
    _fn_types: Set[str]

    def __init__(self, filename: str, plugin):
        self._filename = filename
        self._plugin = plugin

        if not isinstance(plugin, AbstractPlugin):
            raise RuntimeError('plugin class does not implement AbstractPlugin interface: %s' % filename)

        self._fn_types = self._plugin.function_types()

    @property
    def filename(self) -> str:
        return self._filename

    @property
    def function_types(self) -> Set[str]:
        return self._fn_types

    def create_function(self, fn_id: int, fn_type: str, spec: dict) -> FunctionWrapper:
        fn = self._plugin.create_function(fn_type, spec)
        if not isinstance(fn, AbstractFunction):
            raise RuntimeError('function object %s does not implement AbstractFunction interface: plugin %s' %
                               (repr(fn), self._filename))

        return FunctionWrapper(fn_id, self, fn)
