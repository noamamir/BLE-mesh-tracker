from dataclasses import dataclass

import dataclasses_json
from dataclasses_json import dataclass_json


@dataclass_json
@dataclass
class TagMessage:
    uuid: str
    rssi: int
    addr: str
    time_sent: int
    msg_counter: int
