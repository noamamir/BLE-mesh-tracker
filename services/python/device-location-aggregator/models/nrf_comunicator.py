import json
import logging
import os
import time
import serial
import csv
from models.Boat import Boat
from models.tag_message import TagMessage
from models.heartbeat import Heartbeat


class NRFUARTCommunicator:
    def __init__(self, port, boat: Boat, baudrate=460800, timeout=3, max_attempts=5):
        self.boat = boat
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.max_attempts = max_attempts
        self.ser = None
        self.logger = self.setup_logger()
        self.csv_filename = 'received_messages.csv'
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
        with open(self.csv_filename, 'a', newline='') as self.csv_file:
            self.csv_writer = csv.writer(self.csv_file)

            while True:
                try:
                    line = self.read_response()  # Read a line from the serial port

                    if line.find("TAG:") != -1:
                        json_line = line.split("TAG:")[1]  # Parse the JSON data for TAG message
                        data = json.loads(json_line)

                        self.logger.info(f"msg - {data['msg_counter']}")
                        tag_msg = TagMessage(data['uuid'], data['rssi'], data['addr'], data['time_sent'], data['msg_counter'])
                        self.boat.handle_incoming_tag_messages(tag_msg)

                        # Write the received tag message to the CSV file
                        self.csv_writer.writerow([time.strftime('%Y-%m-%d %H:%M:%S'), 'TAG', data['uuid'], data['rssi'], data['addr'], data['time_sent'], data['msg_counter']])
                        self.csv_file.flush()
                    elif line.find("HEARTBEAT:") != -1:
                        json_line = line.split("HEARTBEAT:")[1]  # Parse the JSON data for HEARTBEAT message
                        data = json.loads(json_line)

                        self.logger.info(f"msg - {data['msg_counter']}")
                        heartbeat = Heartbeat(data['uuid'], data['time_sent'], data['msg_counter'])
                        self.boat.handle_incoming_heartbeat(heartbeat)

                        self.csv_writer.writerow([time.strftime('%Y-%m-%d %H:%M:%S'), 'HEARTBEAT', data['uuid'], '', '', data['time_sent'], data['msg_counter']])
                        self.csv_file.flush()
                    time.sleep(0.1)
                except json.JSONDecodeError:
                    self.logger.warning("Received invalid JSON data")
                except serial.SerialException as e:
                    self.logger.error(f"Serial port error: {str(e)}")
                except KeyError as e:
                    self.logger.error(f"Invalid data format: {str(e)}")
                except Exception as e:
                    self.logger.error(f"Unexpected error: {str(e)}")

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
