#!/usr/bin/env python3
from scapy.all import *

ipmsg=IP()
ipmsg.dst='10.9.0.6'
ipmsg.src='10.9.0.5'
icmpmsg=ICMP()
m=ipmsg/icmpmsg
send(m)
