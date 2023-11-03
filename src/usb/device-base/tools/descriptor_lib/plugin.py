from abc import ABC, abstractmethod
from typing import Union, Optional, List, Any, Set

from descriptor_lib.endpoints import EndpointDefinition
from descriptor_lib.interfaces import InterfaceDefinition
from descriptor_lib.usb_struct import EndpointDescriptor

__all__ = [
    'FunctionEnvironment', 'AbstractFunction', 'AbstractPlugin'
]


class FunctionEnvironment(ABC):
    @abstractmethod
    def ep_address(self, ep: Union[EndpointDefinition, str]) -> int:
        """ :return: Endpoint address allocated for a specific endpoint """
        pass

    @abstractmethod
    def ep_descriptor(self, ep: Union[EndpointDefinition, str]) -> EndpointDescriptor:
        """ :return: Endpoint descriptor object for a specific endpoint """
        pass

    @abstractmethod
    def interface_num(self, itf: Union[InterfaceDefinition, str]) -> int:
        """ :return: Interface number """
        pass


class AbstractFunction(ABC):
    __environment: Optional[FunctionEnvironment]

    def __init__(self):
        self.__environment = None

    def set_environment(self, alloc: FunctionEnvironment):
        self.__environment = alloc

    @property
    @abstractmethod
    def function_id(self) -> str:
        """ :return: Function identifier to include in generated descriptor """
        pass

    @property
    def env(self) -> FunctionEnvironment:
        if self.__environment is None:
            raise RuntimeError('Function environment is not yet set')

        return self.__environment

    @property
    def extra_static_checks(self) -> Optional[str]:
        """ :return: Extra checks to be included in generated code's header """
        return None

    @property
    def extra_includes(self) -> List[str]:
        """ :return: Extra includes for generated code """
        return []

    def extra_function_data(self, var_name: str) -> Optional[str]:
        """ :return: Extra function metadata to be included in generated descriptor """
        return None

    @property
    @abstractmethod
    def config_descriptors(self) -> List[Any]:
        """ :return: Set of configuration-related descriptors for this function """
        pass

    @property
    @abstractmethod
    def interfaces(self) -> List[InterfaceDefinition]:
        """ :return: Definition of all interfaces """
        pass

    @property
    @abstractmethod
    def endpoints(self) -> List[EndpointDefinition]:
        """ :return: Definitions of all endpoints """
        pass


class AbstractPlugin(ABC):
    @abstractmethod
    def function_types(self) -> Set[str]:
        pass

    @abstractmethod
    def create_function(self, fn_type: str, spec: dict) -> AbstractFunction:
        pass
