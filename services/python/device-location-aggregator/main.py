import os
import threading
import time

import matplotlib.pyplot as plt
import serial
import json
import matplotlib

from com_port_listener import listen_to_com_port
from models.advertising_packet import AdvertisingPacket
from models.Boat import Boat
from models.hallway import Hallway


def main():
    # Configure the serial port
    boat = Boat()

    threading.Thread(target=boat.handle_beacons_keepalive).start()
    threading.Thread(target=listen_to_com_port, args=[boat]).start()

    plt.ion()
    plt.show(block=False)
    boat.draw_beacons()


if __name__ == "__main__":
    main()
