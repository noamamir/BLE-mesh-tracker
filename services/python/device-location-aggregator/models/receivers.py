from dataclasses import dataclass
from typing import Dict

from dataclasses_json import dataclass_json

from models.beacon import Beacon


@dataclass
class Receiver:
    def __init__(self, uuid: str):
        self.uuid: str = uuid
        self.beacons: Dict[str, Beacon] = {}

    def set_beacon(self, beacon: Beacon):
        if beacon.ID in self.beacons:
            self.beacons[beacon.ID].UpdateBeacon(beacon.rssi)
        else:
            self.beacons[beacon.ID] = beacon

    def to_dict(self) -> Dict:
        return {
            "uuid": self.uuid,
            "beacons": {k: v.to_dict() for k, v in self.beacons.items()}
        }
