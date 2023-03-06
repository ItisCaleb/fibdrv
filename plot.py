import matplotlib.pyplot as plt
import numpy as np

plt.xlabel('n')
plt.ylabel('nanoseconds (ns)')
plt.title('Execution time')

def ext(filename,name):
    times = []
    for line in open(filename,'r').readlines():
        start = line.find('ns:')
        arr = line[start+3:]
        arr = np.array(eval(arr))
    
        z = np.abs((arr-arr.mean())/ arr.std())
        processed = arr[z < 2]

        avg = np.average(processed)
        times.append(avg)
    n = [i for i in range(0,len(times))]
    plt.plot(n,times,label=name)

ext('kernel_ext.txt','kernel')
ext('user_ext.txt','user')
ext('transfer_ext.txt','kernel to user')

plt.legend()
plt.show()

