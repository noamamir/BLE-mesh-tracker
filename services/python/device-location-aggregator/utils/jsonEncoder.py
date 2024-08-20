from dataclasses import is_dataclass, asdict
from typing import Any, Dict
from collections.abc import Iterable


def to_dict(obj: Any) -> Dict:
    if is_dataclass(obj):
        return {k: to_dict(v) for k, v in asdict(obj).items()}
    elif isinstance(obj, dict):
        return {k: to_dict(v) for k, v in obj.items()}
    elif isinstance(obj, list):
        return [to_dict(v) for v in obj]
    elif isinstance(obj, tuple):
        return tuple(to_dict(v) for v in obj)
    elif isinstance(obj, set):
        return {to_dict(v) for v in obj}
    elif hasattr(obj, '__dict__'):
        return to_dict(vars(obj))
    else:
        return obj
