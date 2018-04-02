## python udp echo/forward

import socket
import datetime
import struct

## gamemessage
    #    unsigned int vers;
    #    unsigned int game_id;
    #    unsigned int cmd;
    #    unsigned int player_id;
    #    unsigned int rebroadcasted:1;
    #    unsigned int needs_stream:1;
    #    unsigned int needs_ack:1;
    #    unsigned int is_ack:1;
    #    unsigned int elem_id_net;
    #    unsigned int msg_seq;
    #    game_timeval_t timestamp;

class GameMessage:

    MAX_LEN = 1024
    PLAYER_ID_HOST = 16000

    def __init__(self, data):
        self.data = data
        self.player_id = 0
        self.cmd = 0
        if len(data) > 16:
            self.cmd = struct.unpack("<i", data[8:12])[0]
            self.player_id = struct.unpack("<i", data[12:16])[0]

## peer
class GamePlayer:
    
    def __init__(self, sock, address):
        self.address = address
        self.time_last_active = gametime()
        self.sock = sock
        self.player_id = 0

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
player_ignored = GamePlayer(sock, ("0.0.0.0", 0))

while 1:

    data, addr = sock.recvfrom(GameMessage.MAX_LEN)
    msg = GameMessage(data)
    
    player = None    
    for p in all_players:
        if p.address == addr:
            player = p
        if msg.player_id == p.player_id:
            ## id collision
            player = player_ignored
    if player == None:
        player = GamePlayer(sock, addr)
        player.player_id = msg.player_id
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


