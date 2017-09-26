from scipy.optimize import minimize
from scipy.stats import norm
import numpy as np 
from scipy.stats import truncnorm
from scipy.stats import kstest
from scipy.stats import beta
from scipy.stats import gamma
from scipy.stats import zscore
import seaborn as sns
import matplotlib.pyplot as plt
import pylab 
import scipy.stats as stats
import random

true_score = []
false_score = []
data = np.loadtxt('result.mat')
f = open('result.name','r')
name = []
line = f.readline()
filenum1 = int(line.split(' ')[0])
filenum2 = int(line.split(' ')[1])
thresh = float(line.split(' ')[2][:-1])

while True:
	line = f.readline()
	if not line:
		break
	else:
		name.append(line[:-1])

cnt = 0
p_value = []

for i in range(len(data)):
	cnt += 1
	d1 = data[i]+0.000000000000000001
	ppvp = []
	rl = []
	for j in range(len(d1)):
		if name[j].split('-')[0] == name[i].split('-')[0]:
			rl.append(j)
	d2 = []
	for k in range(len(d1)):
		if k in rl:
			continue
		else:
			d2.append(d1[k])
	#d2 = np.array(d2)*np.array(d2)
		
	param = beta.fit(d2,floc=0, fscale=1)
	rv = beta(param[0],param[1],0,1)

	'''
	if i == 466:
		d3 = list(d1[:])
		del d3[466]
		param = beta.fit(d3)
		rv = beta(param[0],param[1],param[2],param[3])
	'''

	if len(d2) == len(d1) - 1:
		for j in range(len(d1)):
				#true_score.append(rv.cdf(d1[j]))
			if j!=i:
				#false_score.append(rv.cdf(d1[j]))
				ppvp.append(1-rv.cdf(d1[j]))
			else:
				ppvp.append(1)
	else:
		for j in range(len(d1)):
			if j!=i:
				ppvp.append(1-rv.cdf(d1[j]))
			else:
				ppvp.append(1)
	
	p_value.append(ppvp)
	#print ppvp.count(1)

p_value =np.array(p_value)#np.transpose()

#np.savetxt('p_values2.txt',p_value)
#print p_value
print "p-value is calculated......"
#print "q_value is calculated......"

print "p_value threshold is: " + str(thresh)

p_sort = np.argsort(p_value, axis = 0)
[n, m] = p_value.shape

sum = 0.0
q_value = p_value
for i in range(n):
	sum += 1.0/(i+1)

for i in range(m):
	for j in range(n):
		q_value[p_sort[j,i],i] = min(sum*n*p_value[p_sort[j,i], i]/ (p_sort[j,i]+1), 1.0)

#print q_value
q_sort = np.argsort(q_value, axis = 0)
#print q_value
#print q_sort
np.savetxt('p_values.txt', p_value)

fo = open('similar.txt', 'w')
if (filenum2 == 0):
	np.savetxt('q_values.txt', q_value)
	for i in range(m):
		sname = []
		svalue = []
		for j in range(n):
			if (q_value[q_sort[j,i],i] < thresh):
				sname.append(name[q_sort[j,i]])
				svalue.append(q_value[q_sort[j,i],i])
		fo.write(name[i]+': '+str(len(sname))+'\n')
		for j in range(len(sname)):
			fo.write(sname[j]+' '+str(svalue[j])+'\n')
		fo.write('\n')
else:
	np.savetxt('q_values.txt', q_value[:,filenum1:(filenum2+filenum1)])
	for i in range(filenum1, m):
		sname = []
		svalue = []
		for j in range(n):
			if (q_value[q_sort[j,i],i] < thresh):
				sname.append(name[q_sort[j,i]])
				svalue.append(q_value[q_sort[j,i],i])
		fo.write(name[i]+': '+str(len(sname))+'\n')
		for j in range(len(sname)):
			fo.write(sname[j]+' '+str(svalue[j])+'\n')
		fo.write('\n')
fo.close()

print "the program is done......"