import sys
import struct
import matplotlib.pyplot as plt
import numpy as np
import time

i = 0
filename = 'Finger-0 Joint-0 Pos-45 20160224-181434.txt'
sample = 0.01 

def my_print(text):
	sys.stdout.write(str(text))
	sys.stdout.flush()



def main():
	ydata = []
	with open(filename) as f:
    		first = f.readline()
    		lines = f.readlines()

		for line in lines:
			ydata.append(float(line.rstrip('\n')))	
	ydata = np.array(ydata)	
	t = np.linspace(0, sample*ydata.size, ydata.size)
	fig = plt.figure()
	plt.plot(t, ydata)
	plt.ylim(0,100)
	plt.title(filename)
	plt.show()
	
if __name__ == "__main__":
	main()
