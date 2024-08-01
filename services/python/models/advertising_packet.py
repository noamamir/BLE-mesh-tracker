from dataclasses import dataclass


@dataclass
class AdvertisingPacket:
    uuid: str
    rssi: int
    addr: str
