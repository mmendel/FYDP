import sys
import serial
import struct


port = 'com6'

def my_print(text):
	sys.stdout.write(str(text))
	sys.stdout.flush()

class Joint:
	PIP = 0
	MCPV = 1
	MCPH = 2

class Finger:
	INDEX = 0
	MIDDLE = 1
	RING = 2
	PINKY = 3

class Commands:
	STOP = 1
	START = 2
	SET = 3
	GET = 4
	STREAMSTART = 5
	STREAMSTOP = 6


class SCICommandInterface:
	ser = None

	def __init__ (self, port='com6', baudrate=9600, serIn = None, timeoutIn = 5):
		if serIn is not None:
			self.ser = serIn
		else:
			self.ser = serial.Serial(port, baudrate, timeout = timeoutIn)
	
	def SendCommand(self, command, finger = 0, joint = 0, value = 0):
		msg = struct.pack('BBB', command, (finger << 4) + joint, value)
		self.ser.write(':')
		self.ser.write(msg)

	def AllStop(self):
		self.SendCommand(Commands.STOP)	
	
	def AllStart(self):
		self.SendCommand(Commands.START)

	def SetFingerJoint(self, nFinger, nJoint, dVal):
		self.SendCommand(Commands.SET, nFinger, nJoint, dVal)

	def GetFingerJoint(self, nFinger, nJoint):
		self.SendCommand(Commands.GET, nFinger, nJoint)
		print struct.unpack('B', self.ser.read())
	
	def StreamStart(self, nFinger, nJoint):
		self.SendCommand(Commands.STREAMSTART, nFinger, nJoint)
	
	def StreamStop(self, nFinger, nJoint):
		self.SendCommand(Commands.STREAMSTOP, nFinger, nJoint)


