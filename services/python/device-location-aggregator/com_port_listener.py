import os

import serial
import json

from models.Boat import Boat
from models.advertising_packet import AdvertisingPacket
from models.nrf_comunicator import NRFUARTCommunicator


def listen_to_com_port(boat: Boat):
    nrf_comm = NRFUARTCommunicator('COM8')  # Adjust port name as needed

    while True:
        try:
            line = nrf_comm.read_response()
            # Read a line from the serial port

            if line.find("To-Service:") != -1:
                json_line = line.split("To-Service:")[1]
                # Parse the JSON data
                data = json.loads(json_line)

                # Process the data
                # print(f"Received: UUID={data['uuid']}, Address={data['addr']}, RSSI={data['rssi']}")
                adv_packet = AdvertisingPacket(data['uuid'], data['rssi'], data['addr'])
                boat.handle_incoming_adv_packet(adv_packet)
                # Here you can add your own processing logic
                os.system('cls')
                boat.print_receiver_state()
        except json.JSONDecodeError:
            pass
            # print("Received invalid JSON data")
        except serial.SerialException:
            print("Serial port error")
            break
        except KeyError:
            print("thats fucked meh")
