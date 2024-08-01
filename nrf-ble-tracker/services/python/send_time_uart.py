import serial
import time

# Configure the serial port
ser = serial.Serial('COM8', 115200, timeout=1)  # Adjust the port as necessary

# Get the current time in seconds since epoch
current_time = int(time.time())

# Send the time over UART
ser.write(f"{current_time}\n".encode())

print(f"Sent time: {current_time}")

# Close the serial port
ser.close()