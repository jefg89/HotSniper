from ctypes import sizeof
from gzip import READ


sent = open("sent_raw.log", "r")
received = open("received.out", "r")

sent_fixed = []
# Cleaninig raw file
raw = sent.readlines()
for p in raw:
    sent_fixed.append(p[16:32].encode('utf8'))

# Formating received
received_fixed = []
rex = received.readlines()
for x in rex:
    if x[len(x)-1] == '\n':
        received_fixed.append(x[:-1].encode('utf8'))
    else:
        received_fixed.append(x.encode('utf8'))

# Compute xors to get error bits
errs = 0
for i in range(0, len(received_fixed)):
    res = int.from_bytes(sent_fixed[i], "big") ^ int.from_bytes(received_fixed[i], "big")
    #print (str(i) + " " + str(res))
    errs+= bin(res).count("1")

total_bits = 64*len(sent_fixed)
print ("Total received bits : " + str(total_bits))
print ("Total bit with errors: " + str(errs))
print ("BER : {0:.2f} %".format(100*errs/total_bits))


