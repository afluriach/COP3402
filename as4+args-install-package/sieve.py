def mod(a,b):
     return a - (a/b)*b

def isPrime(l, n):
     i=0
     while i < len(l):
             m = mod(n, l[i])
             if m == 0:
                     return 0
             i += 1
     return 1

l=[2]
i=3
lastPrint=0
while len(l) < 9001:
     if len(l) - lastPrint >= 100:
             print len(l)
             lastPrint = len(l)
     if isPrime(l, i):
             l.append(i)
     i += 1

for i in l:
	print i
