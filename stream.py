import sys
import serial
import struct
import time
import command
import datetime

def my_print(text):
	sys.stdout.write(str(text))
	sys.stdout.flush()



_port = 'com6'
pos = 70 
finger = 0
joint = 0

filename = 'Finger-' + str(finger) + ' Joint-' + str(joint) + ' Pos-' + str(pos) + ' ' + datetime.datetime.now().strftime("%Y%m%d-%H%M%S") + '.txt'
my_print(filename)
with open(filename, "a") as f:
	f.write(filename + '\n')
c = command.SCICommandInterface(port = _port)

c.SetFingerJoint(finger, joint, pos)
c.StreamStart(finger, joint)

c.AllStart()
c.ser.flush()
while 1:
	line = c.ser.readline()
	if line[0] == ":":
		line = line.replace(":", "")
		line = line.replace("\r\n", "")
		try:
			val = struct.unpack('>f', line)[0]
			my_print(str(val) + '\n')
			with open(filename, "a") as f:
				f.write(str(val) + '\n')			
		except:
			pass
				



