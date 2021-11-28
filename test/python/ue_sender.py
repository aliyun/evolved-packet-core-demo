# -*- coding: UTF-8 -*-
import time
import threading
import socket
import random
from scapy.all import *
from scapy.contrib.gtp import GTP_U_Header


class UeSender:
    def __init__(self, epc_ip, srv_ip):
        self.gtp_type = 0xff
        self.srv_ip = srv_ip
        self.srv_port = 61001
        self.epc_ip = epc_ip
        self.epc_gtpu_port = 2152
        self.ue = []
        self.data = None
        self.ip_id = random.randint(10000,20000)

    def add_ue(self, ue):
        ue.srv_port = 61001
        #ue.srv_port = 61000 + len(self.ue) + 1
        self.ue.append(ue)

    def gtp_payload(self, teid, ue_ip, sport, dport, raw_len):
        self.ip_id += 1
        return (GTP_U_Header(teid=teid, gtp_type=self.gtp_type, length=raw_len+28)/
                IP(src=ue_ip, dst=self.srv_ip, id=self.ip_id)/
                UDP(sport=sport, dport=dport)/
                Raw('x'*raw_len))
    
    def ul_one_ue_test(self):
        ue_ip = "45.45.0.2"
        up_teid = 1
        data = str(self.gtp_payload(up_teid, ue_ip, 61001, 1422))
        UDPSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        for i in range(1001):
            UDPSock.sendto(data,(self.epc_ip, self.epc_gtpu_port))
        UDPSock.close()

    def send_udp_pkt(self, dip, dport, data):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 1024*1024*256)
        for i in range(1001):
            sock.sendto(data,(dip, dport))

        sock.close()

    def direct_udp_test(self, dip, dport, is_gtp, n):
        th = [ None for i in range(n)]
        data = bytearray([0x78 for i in range(1422)])
        if is_gtp:
            ue_ip = "45.45.0.2"
            up_teid = 1
            data = str(self.gtp_payload(up_teid, ue_ip, 61001, 1422))

        for i in range(n):
            th[i] = threading.Thread(target=self.send_udp_pkt, args=(dip, dport, data,))

        for i in range(n):
            th[i].start()

        for i in range(n):
            th[i].join()
        
    def ue_ul_flow(self, s1u_ul_teid, pdn_addr, ue_port):
        #data = str(self.gtp_payload(s1u_ul_teid, pdn_addr, ue_port, 1422))
        UDPSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        for i in range(1001):
            data = str(self.gtp_payload(s1u_ul_teid, pdn_addr, ue_port, 1422))
            UDPSock.sendto(data,(self.epc_ip, 2152))
        UDPSock.close()

    def ue_ul_flow_2(self, ue_idx):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        for i in range(101):
            sock.sendto(self.data[ue_idx][i],(self.epc_ip, 2152))
        sock.close()

    def ul_start(self):
        num = len(self.ue)
        print "ul_start: %d"%(num)
        th = [ None for i in range(num)]
        self.data = [[ None for j in range(1001)] for i in range(num)]

        for i in range(num):
            client_port = random.randint(20000, 30000)
            for j in range(101):
                self.data[i][j] = str(self.gtp_payload(self.ue[i].s1u_ul_teid,
                                                       self.ue[i].pdn_addr,
                                                       client_port,
                                                       self.ue[i].srv_port, 1422))

        for i in range(num):
            #th[i] = threading.Thread(target=self.ue_ul_flow, args=(self.ue[i].s1u_ul_teid, self.ue[i].pdn_addr, self.ue[i].ue_port,))
            th[i] = threading.Thread(target=self.ue_ul_flow_2, args=(i,))

        for i in range(num):
            th[i].start()

        for i in range(num):
            th[i].join()

    def report_ue_addr_to_srv(self):
            addr_list = ""
            num = len(self.ue)
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            for i in range(num):
                addr_list = addr_list + self.ue[i].pdn_addr
                data = socket.inet_pton(socket.AF_INET, self.ue[i].pdn_addr) + str(bytearray(self.ue[i].s1u_dl_teid))
                sock.sendto(data, (self.srv_ip, 61001))
            sock.close()
            print "addrs: %d, %s"%(num, addr_list)

    def send_to_ue(self, ue_ip):
        r = random.randint(1,9)
        print "send_to_ue: %s"%(ue_ip)
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 1500*1001)
        for i in range(1001):
            sock.sendto(self.data,(ue_ip, 61001))
            time.sleep(0.001*r)

        time.sleep(5)
        sock.close()

    def dl_start(self):
        num = len(self.ue)
        print "dl_start: %d"%(num)
        self.data = bytearray([0x78 for i in range(1422)])
        th = [ None for i in range(num)]

        for i in range(num):
            th[i] = threading.Thread(target=self.send_to_ue, args=(self.ue[i].pdn_addr,))

        for i in range(num):
            th[i].start()

        for i in range(num):
            th[i].join()

if __name__ == '__main__':
    ueSender = UeSender("30.12.20.16", "30.12.22.0")
    #ueSender = UeSender("127.0.0.1")
    #ueSender.ul_one_ue_test()
    #ueSender.direct_udp_test("30.12.22.0", 61001, False, 1)
    #ueSender.ori_udp_test()
