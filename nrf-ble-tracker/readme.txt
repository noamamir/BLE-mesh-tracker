- One board will act as a server (connected to the PC) with the address set to 0x99.
- Client boards will send data to the server as private messages.
- Each client will maintain a list of devices. Each entry in the list will include: BLE address, last seen timestamp, and RSSI.
- The timestamp represents the board's uptime in seconds.
- Every 3 seconds, each client will send a private message to the server, containing the current timestamp and the list of 10 devices, sorted by the most recent timestamp.
- This ensures a consistent payload size without unexpected variations.
- The server will receive these private messages and forward them to the UART shell in the following format:
	[timestamp] RCV client_id,client_timestamp, addr_1, rssi_1, last_seen_1,..., addr_n,rssi_n,last_seen_n
	
	example:
	
	[00794] RCV 0004,00851,6045,048,00851,6046,060,00851,6072,051,00851,C9A1,054,00851,604C,051,00851,6048,055,00851,6077,051,00849,6049,050,00847,6068,060,00847,608A,087,00847