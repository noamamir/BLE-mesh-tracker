import logging
import threading
import time
from typing import Dict, Callable

import numpy
from matplotlib import pyplot as plt

from models.advertising_packet import AdvertisingPacket
from models.beacon import Beacon
from models.hallway import Hallway
from models.receivers import Receiver


class Boat:
    registered_receivers: Dict[str, Receiver] = {}
    beacons_keepalive_interval = 10 * 1000  # in millies
    boat_drawing: Hallway = Hallway()
    emit_tag_message_callback: Callable = None
    emit_heartbeat_callback: Callable = None

    def set_tag_message_callback(self, callback: Callable[[dict], None]):
        self.emit_tag_message_callback = callback

    def set_heartbeat_message_callback(self, callback: Callable[[dict], None]):
        self.emit_heartbeat_callback = callback

    def handle_incoming_tag_messages(self, packet: AdvertisingPacket):
        if packet.uuid in self.registered_receivers:
            beacon = Beacon(packet.addr, packet.rssi)
            self.registered_receivers[packet.uuid].set_beacon(beacon)
        else:
            self.register_receiver(packet.uuid)
        tag_message = {
            'uuid': packet.uuid,
            'addr': packet.addr,
            'rssi': packet.rssi
        }

        self.emit_tag_message_callback(tag_message)

    def handle_incoming_heartbeat(self, uuid, device_id, time_sent):
        heartbeat_message = {
            'uuid': uuid,
            'device_id': device_id,
            'time_sent': time_sent
        }
        emit_heartbeat_message(heartbeat_message)

    def register_receiver(self, receiver_name: str):
        self.registered_receivers[receiver_name] = Receiver(receiver_name)

    def print_receiver_state(self):
        for receiver in self.registered_receivers.values():
            print(f"Receiver #{receiver.uuid}, Beacons: ")
            for beacon in receiver.beacons.values():
                print(f"Beacon {beacon.ID} RSSI: {beacon.rssi}")

    """
        runs each 10s and deletes beacons that havent been updated.
        meaning the beacon is too far and is no longer scanned by the receiver 
    """

    def handle_beacons_keepalive(self):
        logging.info("Starting beacons keepalive thread")
        now = time.time()
        while True:
            for receiver in self.registered_receivers.values():
                for [key, beacon] in receiver.beacons:
                    if beacon.lastUpdated + 10 * 1000 > now:
                        del receiver.beacons[key]

            time.sleep(10)

    def draw_beacons(self):
        while True:
            self.boat_drawing.clear_dots_and_redraw_rooms()
            for beacon_id in self.get_beacon_list():
                min_rssi_abs = 99999999
                closest_receiver_id: str = " "
                for receiver in self.registered_receivers.values():
                    for beacon in receiver.beacons.values():
                        if numpy.abs(min_rssi_abs) > numpy.abs(beacon.rssi):
                            min_rssi_abs = numpy.abs(beacon.rssi)
                            closest_receiver_id = receiver.uuid

                self.boat_drawing.draw_beacon_in_room(closest_receiver_id, beacon_id)
                self.boat_drawing.show_drawing()
            time.sleep(0.2)

    def get_beacon_list(self):
        beacon_dict: Dict[str, Beacon] = {}
        for receiver in self.registered_receivers.values():
            for beacon in receiver.beacons.values():
                beacon_dict[beacon.ID] = beacon

        return beacon_dict.keys()
