import json
import logging
import csv
from models.Boat import Boat
from models.beacon import Beacon
from models.tag_message import TagMessage
from models.heartbeat import Heartbeat
from datetime import datetime
from collections import defaultdict
import serial
import time


class NRFUARTCommunicator:
    def __init__(self, port, boat: Boat, baudrate=115200, timeout=3, max_attempts=5):
        self.boat = boat
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.max_attempts = max_attempts
        self.ser = None
        self.logger = self.setup_logger()
        self.csv_file = None
        self.csv_writer = None

    def setup_logger(self):
        logger = logging.getLogger('NRFUARTCommunicator')
        logger.setLevel(logging.DEBUG)
        handler = logging.StreamHandler()
        formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
        handler.setFormatter(formatter)
        logger.addHandler(handler)
        return logger

    def connect(self):

        self.logger.info(f"Trying to connect to nrf device at {self.port}")
        for attempt in range(self.max_attempts):
            try:
                self.ser = serial.Serial(self.port, self.baudrate, timeout=self.timeout)
                time.sleep(2)  # Allow some time for the connection to establish
                self.logger.info(f"Successfully connected to {self.port}")
                return
            except serial.SerialException as e:
                self.logger.error(
                    f"Attempt {attempt + 1}/{self.max_attempts}: Failed to connect to {self.port}. Error: {str(e)}")
                if attempt < self.max_attempts - 1:
                    self.logger.info(f"Retrying in 5 seconds...")
                    time.sleep(5)
                else:
                    self.logger.error("Max attempts reached. Unable to establish connection.")
                    raise

    def read_from_uart_loop(self):
        today = datetime.now().date()

        with open(f'log_{today.strftime("%Y-%m-%d")}.csv', 'a', newline='') as self.csv_file:
            self.csv_writer = csv.writer(self.csv_file)

            while True:
                try:
                    line = self.read_response()  # Read a line from the serial port
                    if line:
                        parsed = parse_line(line)
                        if parsed:
                            receiver_address, receiver_timestamp, beacons = parsed
                            heartbeat = Heartbeat(receiver_address, receiver_timestamp, 0)
                            self.boat.handle_incoming_heartbeat(heartbeat)

                            for beacon in beacons:
                                tag_msg = TagMessage(receiver_address, beacon.rssi, beacon.ID, beacon.lastUpdated, 0)
                                self.boat.handle_incoming_tag_messages(tag_msg)
                                self.csv_writer.writerow(
                                    [time.strftime('%Y-%m-%d %H:%M:%S'), 'RCV', f"{receiver_address}", f"{beacon.ID}",
                                     f"{beacon.rssi}"])
                                self.csv_file.flush()

                    time.sleep(0.5)
                except json.JSONDecodeError as e:
                    self.logger.warning("Received invalid JSON data")
                    self.csv_writer.writerow([time.strftime('%Y-%m-%d %H:%M:%S'), 'JSONDecodeError', str(e)])
                    self.csv_file.flush()
                except serial.SerialException as e:
                    self.logger.error(f"Serial port error: {str(e)}")
                    self.csv_writer.writerow([time.strftime('%Y-%m-%d %H:%M:%S'), 'SerialException', str(e)])
                    self.csv_file.flush()
                except KeyError as e:
                    self.logger.error(f"Invalid data format: {str(e)}")
                    self.csv_writer.writerow([time.strftime('%Y-%m-%d %H:%M:%S'), 'KeyError', str(e)])
                    self.csv_file.flush()
                except Exception as e:
                    self.logger.error(f"Unexpected error: {str(e)}")
                    self.csv_writer.writerow([time.strftime('%Y-%m-%d %H:%M:%S'), 'Exception', str(e)])
                    self.csv_file.flush()

    def reconnect(self):
        self.logger.info("Attempting to reconnect...")
        self.close()
        try:
            self.connect()
        except serial.SerialException as e:
            self.logger.error(f"Failed to reconnect: {str(e)}")

    def send_current_time(self):
        current_time_ms = int(time.time() * 1000)
        self.send_command(current_time_ms)

    def send_command(self, command):
        try:
            self.ser.write(f"{command}\n".encode())
            time.sleep(0.1)  # Small delay to ensure command is sent
        except serial.SerialException as e:
            self.logger.error(f"Error sending command: {str(e)}")

    def read_response(self):
        try:
            return self.ser.readline().decode('utf-8').strip()
        except serial.SerialException as e:
            self.logger.error(f"Error reading response: {str(e)}")
            return ""

    def sync_time(self):
        current_time = int(time.time() * 1000)  # Current time in milliseconds
        self.send_command(f"SYNC_TIME:{current_time}")
        self.logger.info(f"Sent sync_time: {current_time}")

    def close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()
            self.logger.info(f"Closed connection to {self.port}")

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()


def parse_hex(hex_str):
    return int(hex_str, 16)


def parse_line(line):
    pos = line.find('RCV ')
    if pos == -1:
        return None
    line = line[pos + 4:]

    data = line.split(',')
    receiver_address = str(data[0])
    receiver_timestamp = int(data[1])

    print(line)

    beacons: list[Beacon] = []
    for i in range(2, len(data), 3):
        if i + 2 < len(data):
            beacon_address = data[i]
            rssi = int(data[i + 1])
            last_seen = int(data[i + 2])
            if beacon_address != '0000':
                beacons.append(Beacon(beacon_address, rssi, last_seen))

    return receiver_address, receiver_timestamp, beacons
