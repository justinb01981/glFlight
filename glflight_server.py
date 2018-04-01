## python udp echo/forward

import socket
import datetime

## gamemessage
class GameMessage:

    MAX_LEN = 1024

    def __init__(self, data):
        self.data = data

## peer
class GamePlayer:
    
    def __init__(self, sock, address):
        self.address = address
        self.time_last_active = gametime()
        self.sock = sock

    def send(self, game_message):
        return self.sock.sendto(game_message, self.address)

    def receive(self):
        pair = self.sock.recvfrom()
        return pair[1]

def gametime():
    return datetime.datetime.now()


## main

UDP_IP = "0.0.0.0"
UDP_PORT = 52000

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

sock.bind((UDP_IP, UDP_PORT))

all_players = []

while 1:

    data, addr = sock.recvfrom(GameMessage.MAX_LEN)
    msg = GameMessage(data)
    
    player = None    
    for p in all_players:
        if p.address == addr:
            player = p
    if player == None:
        player = GamePlayer(sock, addr)
        all_players.append(player)
        print("added player" + str(player.address))
    
    for p in all_players:
        if p.address != addr:
            if gametime() - p.time_last_active > datetime.timedelta(seconds=60):
                all_players.remove(p)
                print("timed out player" + str(p.address))
            else:
                p.send(msg.data)
        else:
            p.time_last_active = gametime()


