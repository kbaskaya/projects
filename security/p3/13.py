#!/usr/bin/env python3
from scapy.all import *
from time import *

ipmsg=IP()
ipmsg.dst='172.217.169.132' #google
icmpmsg=ICMP()
t=0
while t<30:
	t+=1
	ipmsg.ttl=t
	rep=sr1(ipmsg/icmpmsg,timeout=5,verbose=0)
	if rep is None:
		print(t,":\t Timeout")
	else:
		print(t,":\t",rep[ICMP].type,"from",rep[IP].src)
		if rep[ICMP].type==0:
			break;
