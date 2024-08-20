import serial
import json
import time


def read_uart_data(port='/dev/ttyACM0', baudrate=115200):
    try:
        with serial.Serial(port, baudrate, timeout=1) as ser:
            print(f"Connected to {port} at {baudrate} baud")
            buffer = ""
            while True:
                # Read data from UART
                data = ser.read(ser.in_waiting or 1)
                if data:
                    buffer += data.decode('utf-8')

                    # Process complete lines
                    while '\n' in buffer:
                        line, buffer = buffer.split('\n', 1)
                        process_line(line.strip())

                time.sleep(0.1)  # Small delay to prevent busy-waiting

    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
    except KeyboardInterrupt:
        print("Script terminated by user")


def process_line(line):
    try:
        # Check if the line starts with TAG: or HEARTBEAT:
        if line.startswith("TAG:") or line.startswith("HEARTBEAT:"):
            # Extract the JSON part
            json_str = line.split(':', 1)[1].strip()
            data = json.loads(json_str)

            if "TAG" in line:
                print(f"Received TAG data: UUID: {data['uuid']}, Address: {data['addr']}, "
                      f"RSSI: {data['rssi']}, Time: {data['time_sent']}, Counter: {data['msg_counter']}")
            elif "HEARTBEAT" in line:
                print(f"Received HEARTBEAT data: UUID: {data['uuid']}, "
                      f"Time: {data['time_sent']}, Counter: {data['msg_counter']}")
        else:
            print(f"Received unknown data: {line}")

    except json.JSONDecodeError:
        print(f"Failed to parse JSON from line: {line}")
    except KeyError as e:
        print(f"Missing expected key in JSON data: {e}")


if __name__ == "__main__":
    # Replace '/dev/ttyACM0' with your actual serial port
    read_uart_data('COM4', 460800)
