import matplotlib.pyplot as plt
import numpy as np

plt.xlabel('n')
plt.ylabel('nanoseconds (ns)')
plt.title('Execution time')

times = []

for line in open('extime.txt','r').readlines():
    start = line.find('ns:')
    arr = line[start+3:]
    arr = np.array(eval(arr))
    
    z = np.abs((arr-arr.mean())/ arr.std())
    processed = arr[z < 1]

    avg = np.average(processed)
    times.append(avg)

n = [i for i in range(0,len(times))]

plt.plot(n,times)
plt.show()

