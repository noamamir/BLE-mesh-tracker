# main.py

import threading
import time

import matplotlib.pyplot as plt
import os
from models.Boat import Boat
from models.nrf_comunicator import NRFUARTCommunicator
from ui_communication_server import UICommunicationServer


def main():
    # Create Boat instance
    boat = Boat()

    # Create UICommunicationServer instance
    com_port_communication = NRFUARTCommunicator('COM15', boat)
    ui_server = UICommunicationServer(boat, com_port_communication)

    ui_server.nrfCommunicator.connect()

    # # Set up callbacks for the Boat instance
    boat.set_tag_message_callback(ui_server.emit_tag_message)
    boat.set_heartbeat_message_callback(ui_server.emit_heartbeat_message)
    #
    # # Start other threads
    threading.Thread(target=boat.handle_beacons_keepalive).start()

    # Run the Flask app
    web_server_thread = threading.Thread(target=ui_server.run)
    web_server_thread.daemon = True
    web_server_thread.start()

    com_port_communication.read_from_uart_loop()


if __name__ == "__main__":
    main()
