import sys

#attributes seperated by \t,lines seperated by \r\n
#source: my file -- "partA.py"
def read_input(filename):

	tmp=[]
	attrList=[]
	result=[]
	trueValue=["Male","Young","Yes","high\r\n"]

	fp1=open(filename,"r")
	lines=fp1.readlines()
	#for each case
	for line in lines:
		tmp.append(line.split('\t'))
	for items in tmp:
		for item in items:
			attrList.append(1 if item.split(" ")[1] in trueValue else 0)
		result.append(attrList)
		attrList=[]
	return result

def hash_data(data):

	tmp=0
	hashTable=[]
	limit=len(data[0])-1
	
	for attrList in data:
		for i in range(limit):
			tmp+=attrList[i]*(2**limit)
		if tmp not in hashTable:
			hashTable.append(tmp)

	return hashTable

def get_version_size(inputSpace,hashTable):

	return 2**(inputSpace-len(hashTable))

def main():

	#problem 1-2 hardcoded problem
	inputSpace_b1=2**4
	conceptSpace_b2=2**inputSpace_b1

	#problem 3-4 computation-based problem
	filename_b1="4Cat-Train.labeled"
	data_b=read_input(filename_b1)
	hashTable_b=hash_data(data_b)
	versionSize=get_version_size(inputSpace_b1,hashTable_b)

	print inputSpace_b1
	print conceptSpace_b2
	print versionSize

if __name__=="__main__":
	main()