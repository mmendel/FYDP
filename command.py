import sys
import serial
import struct


port = 'com6'

def my_print(text):
	sys.stdout.write(str(text))
	sys.stdout.flush()

#Note: 0 index used as not applicable
class Joint:
	PIP = 1
	MCPV = 2
	MCPH = 3

class Finger:
	INDEX = 1
	MIDDLE = 2
	RING = 3
	PINKY = 4

class Commands:
	START = 1
	STOP = 2
	SET = 3
	GET = 4


class SCICommandInterface:
	ser = None

	def __init__ (self, port='com6', baudrate=9600, serIn = None):
		if serIn is not None:
			self.ser = serIn
		else:
			self.ser = serial.Serial(port, baudrate)
	
	def SendCommand(self, command, finger = 0, joint = 0, value = 0):
		msg = struct.pack('BBB', command, (finger << 4) + joint, value)
		self.ser.write(':')
		self.ser.write(msg)

	def AllStop(self):
		self.SendCommand(Commands.STOP)	
	
	def AllStart(self):
		self.SendCommand(Dommands.START)

	def SetFingerJoint(self, nFinger, nJoint, dVal):
		self.SendCommand(Commands.SET, nFinger, nJoint, dVal)

	def GetFingerJoint(self, nFinger, nJoint):
		self.SendCommand(Commands.GET, nFinger, nJoint)

