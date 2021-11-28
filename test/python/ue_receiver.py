# -*- coding: UTF-8 -*-
import time
import datetime
import thread
from scapy.all import *
from scapy.contrib.gtp import GTP_U_Header
import socket
from ue import UE
from ue_sender import UeSender



class UeReceiver:
    def __init__(self):
        self.start = None
        self.cnt = 0
        self.bcnt = 0
        self.sock = []
        self.stop = False

    def msg_to_hexString(self, msg):
        length = len(msg)
        msg_str = ''
        for i in range(length):
            msg_str = '%s%02x'%(msg_str, ord(msg[i]))
            if i%8 == 7:
                msg_str = msg_str + ' '
        return msg_str    

    def srv_wait_ue_addrs(self, ip, port):
        ueSender = UeSender("30.12.20.16", "30.12.22.0")
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind((ip, port))
        while True:
            data, addr = sock.recvfrom(1500)
            ueSender.add_ue(UE(None,None,data[0:4]))
            #print self.msg_to_hexString(data)
            self.cnt += 1
            if self.cnt == 200:
                break
        sock.close()
        ueSender.dl_start()


    def srv_start_instance(self, ip, port):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1500*1024*256)
        sock.bind((ip, port))
        self.sock.append(sock)
        while True:
            data, addr = sock.recvfrom(1500)
            self.cnt += 1
            if self.cnt == 1:
                print "1\t%d, self.cnt=%d"%(port, self.cnt)
                self.start = datetime.now()
            elif self.cnt == 1001:
                delta = datetime.now() - self.start
                print "%d,\trecv 1.5KBytes in %d.%lds"%(port, delta.seconds, delta.microseconds)
                self.cnt = 0
                print "1001\t%d, self.cnt=%d"%(port, self.cnt)


    def srv_start_new(self, ip, start_port, n):
        th = [ None for i in range(n) ]

        for i in range(n):
            th[i] = threading.Thread(target=self.srv_start_instance, args=(ip, start_port+i, ))

        for i in range(n):
            th[i].start()

        for i in range(n):
            th[i].join()

        time.sleep(60)
        for i in range(n):
            self.sock[i].close()
            th[i].stop()


    
if __name__ == '__main__':
    ueReceiver = UeReceiver()
    #ueReceiver = UeReceiver("30.12.22.0", 61008, 0)
    #ueReceiver.srv_start_new("30.12.22.0", 61001, 1)
    ueReceiver.srv_wait_ue_addrs("30.12.22.0", 61001)

