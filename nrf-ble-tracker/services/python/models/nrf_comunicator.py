import logging
import time

import serial


class NRFUARTCommunicator:
    def __init__(self, port, baudrate=460800, timeout=1):
        self.ser = serial.Serial(port, baudrate, timeout=timeout)
        time.sleep(2)  # Allow some time for the connection to establish

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