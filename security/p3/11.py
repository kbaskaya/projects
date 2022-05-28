#!/usr/bin/env python3
from scapy.all import *
def print_pkt(pkt):
	pkt.show()

pkt = sniff(iface='br-2a0d781115d4', filter='icmp', prn=print_pkt)
#pkt = sniff(iface='br-2a0d781115d4', filter='tcp dst port 23 and src host 1.2.3.4', prn=print_pkt)
#pkt = sniff(iface='br-2a0d781115d4', filter='net 128.230.0.0/16', prn=print_pkt)
