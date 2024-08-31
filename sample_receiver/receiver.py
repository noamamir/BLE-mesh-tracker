import datetime
from collections import defaultdict
import serial
import time

def parse_hex(hex_str):
    return int(hex_str, 16)

def twos_complement(hex_str, bits=8):
    value = int(hex_str, 16)
    if value & (1 << (bits-1)):
        value -= 1 << bits
    return value

def parse_line(line):
    pos = line.find('RCV ')
    if pos == -1:
        return None
    line = line[pos + 4:]

    data = line.split(',')
    receiver_address = parse_hex(data[0])
    receiver_timestamp = int(data[1])
    
    print(line)

    beacons = []
    for i in range(2, len(data), 3):
        if i+2 < len(data):
            beacon_address = data[i]
            rssi = int(data[i+1])
            last_seen = int(data[i+2])
            if (beacon_address != '0000'):
                beacons.append((beacon_address, rssi, last_seen))

    return receiver_address, receiver_timestamp, beacons

def process_data(serial_port):
    data = defaultdict(lambda: defaultdict(lambda: {'rssi': None, 'last_seen': None}))
    receiver_last_seen = {}

    while True:
        try:
            line = serial_port.readline().decode('utf-8').strip()
            if line:
                parsed = parse_line(line)
                if parsed:
                    receiver_address, receiver_timestamp, beacons = parsed
                    current_time = time.time()
                    receiver_last_seen[receiver_address] = current_time
                    for beacon_address, rssi, last_seen in beacons:
                        time_diff = receiver_timestamp - last_seen
                        last_seen_time = current_time - time_diff
                        data[beacon_address][receiver_address] = {
                            'rssi': rssi,
                            'last_seen': last_seen_time                     }
                    if (receiver_address == 0x99):
                        print_table(data, receiver_last_seen)
                        print("\n" + "="*80 + "\n")  # Separator between table updates
        except KeyboardInterrupt:
            print("Stopping data collection.")
            break
        except Exception as e:
            print(f"Error: {e}")
            time.sleep(1)  # Wait a bit before trying again

def print_table(data, receiver_last_seen):
    receivers = sorted(set(receiver for beacon in data.values() for receiver in beacon))
    
    # Calculate column widths
    beacon_width = max(len("Beacon"), max(len(beacon) for beacon in data))
    receiver_width = max(max(len(str(receiver)) + 8 for receiver in receivers), 25)  # +8 for "(XXXs)" part
    
    # Print header
    print(f"NUM  {'Beacon':<{beacon_width+2}}", end='')
    current_time = time.time()
    for receiver in receivers:
        last_seen = receiver_last_seen.get(receiver, 0)
        elapsed = int(current_time - last_seen) if last_seen > 0 else 0
        header = f"{receiver} ({elapsed:3}s)"
        print(f"{header:^{receiver_width}}", end='')
    print()
    
    # Print separator
    print("-" * (beacon_width + len(receivers) * receiver_width))

    # Print data rows
    index = 0
    for beacon, receivers_data in sorted(data.items()):
        index += 1
        print(f"[{index:<2}] {beacon:<{beacon_width}}", end='')
        for receiver in receivers:
            if receiver in receivers_data:
                info = receivers_data[receiver]

                last_seen = int(current_time - info['last_seen'])
                rssi = info['rssi']
                print(f"{last_seen:>10} ({rssi:>4})", end='')
                padding = receiver_width - 16  # 16 is the length of "XXXXX (XXXX)"
                print(" " * padding, end='')
            else:
                print(f"{'-':^{receiver_width}}", end='')
        print()


def main():
    port = 'com9'  # Change this to match your serial port
    baud_rate = 115200     # Change this to match your baud rate

    try:
        with serial.Serial(port, baud_rate, timeout=1) as ser:
            print(f"Connected to {port} at {baud_rate} baud")
            process_data(ser)
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")

if __name__ == "__main__":
    main()