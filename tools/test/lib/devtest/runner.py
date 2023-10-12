from abc import ABC, abstractmethod
from typing import List, Union, Optional

from debug_link import DebugLink


class TestEnvironment(ABC):
    @property
    @abstractmethod
    def link(self) -> DebugLink:
        """ Retrieve debug link instance """

    @property
    @abstractmethod
    def scratchpad_ptr(self) -> int:
        """ Address of the scratchpad RAM area """

    @property
    @abstractmethod
    def scratchpad_sz(self) -> int:
        """ Length of the scratchpad RAM area """

    @abstractmethod
    def run(self, result_len: int = 0):
        """ Run test function on the target """

    @abstractmethod
    def write(self, location: Union[str, int], data: Union[int, bytes]):
        """ Write target's memory """

    @abstractmethod
    def read(self, location: Union[str, int], length: Optional[int] = None) -> bytes:
        """ Read target's memory """


class TestRunner(ABC):
    _env: TestEnvironment
    _args: List[str]

    def __init__(self, env: TestEnvironment, args: List[str]):
        self._env = env
        self._args = args

    @abstractmethod
    def run(self):
        raise RuntimeError('TestRunner.run is not implemented')
