import sys
import serial
import struct
import matplotlib.pyplot as plt
import numpy as np
import time

port = 'com6'
i = 0

def my_print(text):
	sys.stdout.write(str(text))
	sys.stdout.flush()



def main():
	ser = serial.Serial(port)
	ydata = [0]*100
	plt.ion()
	fig = plt.figure()
	ax = fig.add_subplot(111)
	line1, = ax.plot(ydata)
	ax.set_ylim([0,4])
	plt.show()
	fig.canvas.draw()
	plt.pause(3)
	ser.flush()
	while 1:
		line = ser.readline()

		if line[0] == ":":
			line = line.replace(":", "")
			line = line.replace("\r\n", "")
			try:
				val = struct.unpack('>f', line)[0]
				#my_print(val)
			except:
				pass
				
			ydata.append(val)
			del ydata[0]
			#line1.set_xdata(np.arrange(len(ydata)))
			line1.set_ydata(ydata)
			plt.pause(0.0001)
		else:
			my_print(line)

if __name__ == "__main__":
	main()
	plt.waitforbuttonpress(timeout=-1)
