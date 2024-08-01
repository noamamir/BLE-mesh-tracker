import time
from dataclasses import dataclass


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
