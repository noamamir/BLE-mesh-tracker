# ui_communication_server.py

from flask import Flask, jsonify
from flask_socketio import SocketIO, emit

from models.nrf_comunicator import NRFUARTCommunicator


class UICommunicationServer:
    def __init__(self, boat, nrfCommunicator: NRFUARTCommunicator):
        self.boat = boat
        self.nrfCommunicator = nrfCommunicator
        self.app = Flask(__name__)
        self.socketio = SocketIO(self.app, cors_allowed_origins="*")
        self._setup_routes()
        self._setup_socketio_events()

    def _setup_routes(self):
        @self.app.route('/devices', methods=['GET'])
        def get_devices():
            devices = []
            for receiver in self.boat.registered_receivers.values():
                device = {
                    'uuid': receiver.uuid,
                    'beacons': [beacon.__dict__ for beacon in receiver.beacons.values()]
                }
                devices.append(device)
            return jsonify(devices)

        @self.app.route('/syncTime', methods=['POST'])
        def sync_time():
            self.nrfCommunicator.sync_time()
            return True

    def _setup_socketio_events(self):
        @self.socketio.on('connect')
        def handle_connect():
            print('Client connected')

    def emit_tag_message(self, tag_message):
        self.socketio.emit('tag_message', tag_message)

    def emit_heartbeat_message(self, heartbeat_message):
        self.socketio.emit('heartbeat_message', heartbeat_message)

    def run(self, host='0.0.0.0', port=5000, debug=True, allow_unsafe_werkzeug=True):
        self.socketio.run(self.app, host=host, port=port, debug=debug, allow_unsafe_werkzeug=allow_unsafe_werkzeug)
