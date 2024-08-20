from dataclasses import dataclass

from dataclasses_json import dataclass_json


@dataclass_json
@dataclass
class Heartbeat:
    uuid: str
    time_sent: str
    msg_counter: str
