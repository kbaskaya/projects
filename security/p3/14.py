#!/usr/bin/env python3
from scapy.all import *
def print_pkt(pkt):
	if ICMP in pkt and pkt[ICMP].type==8:
		newip=IP(src=pkt[IP].dst,dst=pkt[IP].src,ihl=pkt[IP].ihl,ttl=pkt[IP].ttl)
		newicmp=ICMP(type=0,id=pkt[ICMP].id,seq=pkt[ICMP].seq)
		data=pkt[Raw].load
		newpkt=newip/newicmp/data
		newpkt.show()
		send(newpkt)

pkt = sniff(iface='br-2a0d781115d4', filter='icmp', prn=print_pkt)
