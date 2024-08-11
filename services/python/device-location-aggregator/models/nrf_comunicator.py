import json
import logging
import os
import time

import serial

from models.Boat import Boat
from models.advertising_packet import AdvertisingPacket


class NRFUARTCommunicator:
    def __init__(self, port, boat: Boat, baudrate=460800, timeout=1):
        self.boat = boat
        self.ser = serial.Serial(port, baudrate, timeout=timeout)
        time.sleep(2)  # Allow some time for the connection to establish

    def read_from_uart_loop(self):
        while True:
            try:
                line = self.read_response()  # Read a line from the serial port

                if line.find("TAG:") != -1:
                    json_line = line.split("TAG:")[1]  # Parse the JSON data for TAG message
                    data = json.loads(json_line)

                    adv_packet = AdvertisingPacket(data['uuid'], data['rssi'], data['addr'])
                    self.boat.handle_incoming_tag_messages(adv_packet)

                elif line.find("HEARTBEAT:") != -1:
                    json_line = line.split("HEARTBEAT:")[1]  # Parse the JSON data for HEARTBEAT message
                    data = json.loads(json_line)

                    # Process the HEARTBEAT data
                    uuid = data['uuid']
                    device_id = data['device_id']
                    time_sent = data['time_sent']

                    # Call the appropriate function to handle the HEARTBEAT message
                    self.boat.handle_incoming_heartbeat(uuid, device_id, time_sent)

                os.system('cls')
                self.boat.print_receiver_state()

            except json.JSONDecodeError:
                pass  # print("Received invalid JSON data")
            except serial.SerialException:
                print("Serial port error")
                break
            except KeyError:
                print("Invalid data format")

    def send_current_time(self):
        # Get the current time in milliseconds
        current_time_ms = int(time.time() * 1000)

        # Send the current time in milliseconds
        self.send_command(current_time_ms)

    def send_command(self, command):
        self.ser.write(f"{command}\n".encode())
        time.sleep(0.1)  # Small delay to ensure command is sent

    def read_response(self):
        return self.ser.readline().decode('utf-8').strip()

    def sync_time(self):
        current_time = int(time.time() * 1000)  # Current time in milliseconds
        self.send_command(f"SYNC_TIME:{current_time}")
        print(f"sent sync_time: {current_time}")

    def close(self):
        self.ser.close()
