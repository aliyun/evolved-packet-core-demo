import time
import struct
import threading
import socket
import random
from scapy.all import *
from scapy.contrib.gtp import GTP_U_Header


class UE:
    def __init__(self, ue_ip, ul_teid):
        self.ue_ip = copy.deepcopy(socket.inet_ntoa(struct.pack('I', socket.htonl(ue_ip))))
        self.ul_teid = ul_teid
        print self.ue_ip,self.ul_teid

class UlFlow:
    def __init__(self, epc_ip, srv_ip):
        self.gtp_type = 0xff        
        self.srv_ip = srv_ip
        self.epc_ip = epc_ip
        self.ip_id = random.randint(10000,20000)
        self.ue = []

    def gtp_payload(self, teid, ue_ip, ip_id, sport, dport, raw_len):
        #self.ip_id += 1
        return (GTP_U_Header(teid=teid, gtp_type=self.gtp_type, length=raw_len+28)/
                IP(src=ue_ip, dst=self.srv_ip, id=ip_id)/
                UDP(sport=sport, dport=dport)/
                Raw('x'*raw_len))

    def ue_ul_flow(self, ue_idx):
        ip_id = 1
        client_port = random.randint(20000, 30000)
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 1500*1024)
        data = str(self.gtp_payload(self.ue[ue_idx].ul_teid,
                                        self.ue[ue_idx].ue_ip,
                                        ip_id,
                                        client_port,
                                        61001, 1422))
        for i in range(1001):
            #sock.sendto(self.data[ue_idx][i],(self.epc_ip, 2152))
            sock.sendto(data, (self.epc_ip, 2152))
            ip_id += 1
        sock.close()

    def test(self, num):
        for i in range(num):
            self.ue.append(UE(0x2d2d0002+i, 1+i))
        
        th = [ None for i in range(num)]
        self.data = [[ None for j in range(1001)] for i in range(num)]

        '''
        for i in range(num):
            client_port = random.randint(20000, 30000)
            for j in range(501):
                self.data[i][j] = str(self.gtp_payload(self.ue[i].ul_teid,
                                                       self.ue[i].ue_ip,
                                                       client_port,
                                                       61001, 1422))
        '''

        for i in range(num):
            th[i] = threading.Thread(target=self.ue_ul_flow, args=(i,))

        for i in range(num):
            th[i].start()

        for i in range(num):
            th[i].join()


if __name__ == '__main__':
    ulFlow = UlFlow("30.12.20.16", "30.12.22.0")
    ulFlow.test(200)

