from sys import argv
# Please install pandas before running
# pip install requirements.txt

import pandas as pd
import sys

out_file_c0 = open('c0.out', 'a')
out_file_c1 = open('c1.out', 'a')

sum_file_c0 = open ('packets.out', 'a')

file_name = str(sys.argv[1])
columns = ['Core0-TP', 'Core1-TP']
data = pd.read_table(file_name)
c0 = data[columns[0]].values
c1 = data[columns[1]].values

for x in c0:
    out_file_c0.write(str(x) + "\n")
for x in c1:
    out_file_c1.write(str(x) + "\n")


#Extracting packet value from name
p_values = file_name[-9:-6]
#print (p_values)
sum_file_c0.write(p_values + "\n")  
out_file_c0.close()
out_file_c1.close()
#print (c0)