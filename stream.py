import sys
import serial
import struct
import time
import command
import datetime
import argparse
from threading import Event, Thread
import matplotlib.pyplot as plt

finger = 0
joint = 0
pos = 90
minpos = 30
setpos = 90
portin = 'com6'
interval = 10 #seconds
c = None

data = [0]*500
refdata = [0]*500

def my_print(text):
	sys.stdout.write(str(text))
	sys.stdout.flush()


class Graph:
	def __init__(self, interval):
		self.minpos = minpos
		self.maxpos = maxpos
		self.setpos = maxpos
		self.thread = Thread(target=self._target)
		self.thread.start()
		self.data = [0]*500
		self.refdata = [0]*500

	def _target(self):
		fig = plt.figure()
		ax = fig.add_subplot(111)
		line1 = ax.plot(self.data)
		line2 = ax.plot(self.refdata)
		ax.set_ylim([0,100])
		plt.ion()
		plt.show()
		while 1:
			my_print('dsadsa')
			plt.pause(10)
			line1.set_ydata(self.data)
			line2.set_ydata(self.refdata)
			plt.draw()

class PosistionUpdate:
	def __init__(self, interval, c, finger, joint, minpos, maxpos):
		self.interval = interval
		self.c = c
		self.finger = finger
		self.joint = joint
		self.minpos = minpos
		self.maxpos = maxpos
		self.setpos = maxpos
		self.thread = Thread(target=self._target)
		self.thread.start()

	def _target(self):
		#while not self.event.wait(self._time):
		while 1:
			if self.setpos == self.minpos:
				self.c.SetFingerJoint(self.finger, self.joint, self.maxpos)
				self.setpos = self.maxpos
			else:
				self.c.SetFingerJoint(self.finger, self.joint, self.minpos)
				self.setpos = self.minpos
			time.sleep(self.interval)
	
	def _time(self):
		return self.interval - ((time.time() - self.start) % self.interval)

	def refpos(self):
		return self.setpos

def stream():
	filename = 'Finger-' + str(finger) + ' Joint-' + str(joint) + ' Pos-' + str(pos) + ' ' + datetime.datetime.now().strftime("%Y%m%d-%H%M%S") + '.txt'
	my_print(filename)
	with open(filename, "a") as f:
		f.write(filename + '\n')
	c.StreamStart(finger, joint)
	posistion = PosistionUpdate(10, c, finger, joint, minpos, pos)	
	#graph = Graph(1)	
	plt.show()

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
					f.write(str(val) + ' ' + str(posistion.refpos()) + '\n')			
				data.append(val)
				refdata.append(posistion.refpos())
		
				del data[0]
				del refdata[0]

			except:
				pass
					

if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument('Finger', metavar='F', type=int)
	parser.add_argument('Joint', metavar='J', type=int)
	parser.add_argument('Posistion', metavar='P', type=int)
	parser.add_argument('--port', default = 'com6')
	args = parser.parse_args()
	finger = args.Finger
	joint = args.Joint
	pos = args.Posistion
	portin = args.port
	c = command.SCICommandInterface(port = portin)
	
	#stream()
	thread = Thread(target = stream)
	thread.start()
	time.sleep(3)	
	fig = plt.figure()
	ax = fig.add_subplot(111)
	line1, = ax.plot(data)
	line2, = ax.plot(refdata)
	ax.set_ylim([0,100])
	plt.ion()
	plt.show()
	while 1:
		plt.pause(.1)
		line1.set_ydata(data)
		line2.set_ydata(refdata)
		plt.draw()
