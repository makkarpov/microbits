from typing import NamedTuple, Optional
from native import ExecutionResult


class TestResult(NamedTuple):
    executable_sz: int
    execution: ExecutionResult


class ResultMessage(NamedTuple):
    name: str
    success: bool
    failure: Optional[str]
    result: Optional[TestResult]
