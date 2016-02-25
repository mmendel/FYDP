import sys
import serial
import struct
import time
import command
import datetime
import argparse

def my_print(text):
	sys.stdout.write(str(text))
	sys.stdout.flush()



def stream(finger, joint, pos, portin):
	filename = 'Finger-' + str(finger) + ' Joint-' + str(joint) + ' Pos-' + str(pos) + ' ' + datetime.datetime.now().strftime("%Y%m%d-%H%M%S") + '.txt'
	my_print(filename)
	with open(filename, "a") as f:
		f.write(filename + '\n')
	c = command.SCICommandInterface(port = portin)

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
					


if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument('Finger', metavar='F', type=int)
	parser.add_argument('Joint', metavar='J', type=int)
	parser.add_argument('Posistion', metavar='P', type=int)
	parser.add_argument('--port', default = 'com6')
	args = parser.parse_args()
	stream(args.Finger, args.Joint, args.Posistion, args.port)

