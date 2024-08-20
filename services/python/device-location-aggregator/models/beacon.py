import time
from dataclasses import dataclass
from typing import Dict


class Beacon:
    ID: str
    rssi: int
    lastUpdated: int

    def __init__(self, ID: str, rssi: int):
        self.ID = ID
        self.rssi = rssi
        self.lastUpdated = int(time.time())

    def UpdateBeacon(self, rssi: int):
        self.rssi = rssi
        self.lastUpdated = int(time.time())

    def to_dict(self) -> Dict:
        return {
            "ID": self.ID,
            "rssi": self.rssi,
            "lastUpdated": self.lastUpdated
        }
