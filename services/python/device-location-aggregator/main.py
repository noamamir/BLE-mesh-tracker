# main.py

import threading
import matplotlib.pyplot as plt
import os
from models.Boat import Boat
from models.nrf_comunicator import NRFUARTCommunicator
from ui_communication_server import UICommunicationServer


def main():
    os.environ.setdefault('FLASK_ENV', 'development')

    # Create Boat instance
    boat = Boat()

    # Create UICommunicationServer instance
    com_port_communication = NRFUARTCommunicator('COM8', boat)
    ui_server = UICommunicationServer(boat, com_port_communication)

    # Set up callbacks for the Boat instance
    boat.set_tag_message_callback(ui_server.emit_tag_message)
    boat.set_heartbeat_message_callback(ui_server.emit_heartbeat_message)

    # Start other threads
    threading.Thread(target=boat.handle_beacons_keepalive).start()
    threading.Thread(target=com_port_communication.read_from_uart_loop).start()

    # Run the Flask app
    ui_server.run(host='0.0.0.0', port=5000)
    # Draw beacons (if this should run after the Flask app)
    plt.ion()
    plt.show(block=False)
    boat.draw_beacons()


if __name__ == "__main__":
    main()
