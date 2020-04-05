import matplotlib.pyplot as plt
import random

xs = list(range(10000))
counts = [0] * 10000
for i in range(10000):
   x = random.randint(0,99)
   y = random.randint(0,99)
   counts[x*100+y] += 1
   counts[y*100+x] += 1

plt.bar(xs,counts)
plt.show()
