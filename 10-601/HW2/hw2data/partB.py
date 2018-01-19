import sys

#attributes seperated by \t,lines seperated by \r\n
#source: my file -- "partA.py"
def read_input_b(filename):

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

def hash_size(data):

	tmp,cnt=0,0
	hashTable=[]
	limit=len(data[0])-1
	
	for attrList in data:
		for i in range(limit):
			tmp+=attrList[i]*(2**limit)	
		if tmp not in hashTable:
			hashTable.append(tmp)
			cnt+=1
	return cnt

def hash_data(data):

	tmp=0
	hash1,hash2=[],[]
	limit=len(data[0])-1

	for attrList in data:
		for i in range(limit):
			tmp+=attrList[i]*(2**limit)
		if attrList[-1]==1:
			if tmp not in hash1:
				hash1.append(tmp)
		else:
			if tmp not in hash2:
				hash2.append(tmp)
	return hash1,hash2

def get_version_size(data,inputSpace):

	return 2**(inputSpace-hash_size(data))

def classify(data,hash1,hash2):
	
	tmp=0
	versionSize=32
	limit=len(data[0])-1

	for attrList in data:
		
		for i in range(limit):
			tmp+=attrList[i]*(2**limit)
		pos,neg=0,0
		if tmp in hash1:
			pos=versionSize
		elif tmp in hash2:
			neg=versionSize
		else:
			pos,neg=versionSize/2,versionSize/2
		print pos,neg

def main():
	#problem 1-2 hardcoded problem
	inputSpace_b1=2**4
	conceptSpace_b2=2**inputSpace_b1

	#problem 3-4 computation-based problem
	filename_b1="4Cat-Train.labeled"
	data_b1=read_input_b(filename_b1)
	versionSize=get_version_size(data_b1,inputSpace_b1)
	#hash1,hash2 are positive/negative results,respectively
	hash1,hash2=hash_data(data_b1)

	print inputSpace_b1
	print conceptSpace_b2
	print versionSize

	data_b2=read_input_b(sys.argv[1])
	classify(data_b2,hash1,hash2)

if __name__=="__main__":
	main()



