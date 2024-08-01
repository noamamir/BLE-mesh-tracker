import logging

import matplotlib.pyplot as plt
import matplotlib.patches as patches

from models.beacon import Beacon
from models.room import Room


class Hallway:
    rooms: dict[str, Room] = {
        '0009': Room('eran', '0009', (-1, -1), 1, 1, 'purple'),
        '0008': Room('lab', '0008', (-2, 1), 1, 1, 'blue'),
        '0007': Room('maor', '0007', (-1, 1), 1, 1, 'yellow'),
        '0006': Room('yoav', '0006', (0, 1), 1, 1, 'red'),
        '0004': Room('noam', '0004',  (0, -1), 1, 1, 'green'),
    }

    fig, ax = plt.subplots()

    def __int__(self, rooms: dict):
        self.rooms = rooms

    def draw_hallway_with_rooms(self):
        # Create a new figure and axis

        hallway = Room('Hallway', '0001', (-2, 0), 4, 1, 'gray')

        # Add the hallway to the plot
        rect = patches.Rectangle(hallway.bottom_left, hallway.width, hallway.height, linewidth=1, edgecolor='black',
                                 facecolor=hallway.color)
        self.ax.add_patch(rect)
        plt.text(hallway.bottom_left[0] + hallway.width / 2, hallway.bottom_left[1] + hallway.height / 2, hallway.id,
                 color='black', ha='center', va='center')

        # Add rooms to the plot
        for room in self.rooms.values():
            rect = patches.Rectangle(room.bottom_left, room.width, room.height, linewidth=1, edgecolor='black',
                                     facecolor=room.color)
            self.ax.add_patch(rect)
            plt.text(room.bottom_left[0] + room.width / 2, room.bottom_left[1] + room.height / 2, room.name,
                     color='black',
                     ha='center', va='center')

        # Set the x and y axis limits
        self.ax.set_xlim(-3, 5)
        self.ax.set_ylim(-1, 3)

        # Set the aspect of the plot to be equal
        self.ax.set_aspect('equal')

        # Add grid lines
        plt.grid(True)



    def draw_beacon_in_room(self, room_id: str, beacon: str):
        # Add the label above the dot
        if room_id not in self.rooms:
            logging.error(f"Failed to draw beacon in room id {room_id} room doesnt exist")
            return

        room = self.rooms[room_id]

        plt.text(room.bottom_left[0] + room.width / 2, room.bottom_left[1] + room.height / 2, beacon[-6:], color='black',
                 ha='center')

    def clear_dots_and_redraw_rooms(self):
        # Clear the current plot
        self.ax.cla()

        # Redraw the original rooms
        self.draw_hallway_with_rooms()

    def show_drawing(self):
        plt.draw()
        plt.pause(0.001)
