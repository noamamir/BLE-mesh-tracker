# ui_communication_server.py
import json
from dataclasses import asdict

from flask import Flask, jsonify
from flask_socketio import SocketIO, emit
from flask_cors import CORS

from models.heartbeat import Heartbeat
from models.nrf_comunicator import NRFUARTCommunicator
from models.tag_message import TagMessage
from utils.jsonEncoder import to_dict


class UICommunicationServer:
    def __init__(self, boat, nrfCommunicator: NRFUARTCommunicator):
        self.boat = boat
        self.nrfCommunicator = nrfCommunicator
        self.app = Flask(__name__)
        CORS(self.app)
        self.socketio = SocketIO(self.app, cors_allowed_origins="*", async_mode='threading')
        self._setup_routes()
        self._setup_socketio_events()

    def _setup_routes(self):
        @self.app.route('/devices', methods=['GET'])
        def get_devices():
            devices_dict = self.boat.get_devices_as_dict()
            return jsonify(devices_dict)


        @self.app.route('/syncTime', methods=['POST'])
        def sync_time():
            self.nrfCommunicator.sync_time()
            return True

    def _setup_socketio_events(self):
        @self.socketio.on('connect')
        def handle_connect():
            print('Client connected')

    def emit_tag_message(self, tag_message: TagMessage):
        self.socketio.emit('tag_message', to_dict(tag_message))

    def emit_heartbeat_message(self, heartbeat_message: Heartbeat):
        self.socketio.emit('heartbeat_message', to_dict(heartbeat_message))

    def run(self, host='0.0.0.0', port=5000, use_reloader=False):
        self.socketio.run(self.app, host=host, port=port, debug=False, use_reloader=use_reloader, allow_unsafe_werkzeug=True)
