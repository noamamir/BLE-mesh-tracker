from dataclasses import dataclass
from typing import Dict

from models.beacon import Beacon


class Receiver:
    def __init__(self, uuid: str):
        self.uuid: str = uuid
        self.beacons: Dict[str, Beacon] = {}

    def set_beacon(self, beacon: Beacon):
        if beacon.ID in self.beacons:
            self.beacons[beacon.ID].UpdateBeacon(beacon.rssi)
        else:
            self.beacons[beacon.ID] = beacon
