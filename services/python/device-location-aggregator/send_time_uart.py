import serial
import time

# Configure the serial port
ser = serial.Serial('COM3', 460800, timeout=1)

# Get the current time in milliseconds
current_time_ms = int(time.time() * 1000)

# Send the current time in milliseconds
ser.write(f"SYNC_TIME:{current_time_ms}\n".encode())

print(f"Sent initial time: {current_time_ms} ms")

# Close the serial connection
ser.close()
