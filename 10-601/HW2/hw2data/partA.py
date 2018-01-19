import sys

#attributes seperated by \t,lines seperated by \r\n
def read_input(filename):

	tmp=[]
	attrList=[]
	result=[]
	trueValue=["Male","Young","Yes","Long","House","high\r\n"]

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

#print the current hypothesis for every 30 training instances
def generalize_data(data):

	cnt=0
	first_found=False
	currentHypo=[0 for i in range(9)]
	result=[]

	for attrList in data:

		cnt+=1
		#found the first positive result
		if not first_found:
			if attrList[-1]==1:
				currentHypo=attrList[:-1]
				first_found=True
		else:
			if attrList[-1]==1:
				newHypo=attrList[:-1]
				currentHypo=update_hypo(newHypo,currentHypo)

		if cnt%30==0:
			result.append(currentHypo)

	return result

#to compare new cases with current hypothesis
def update_hypo(newHypo,currentHypo):

	i=0
	limit=len(newHypo)

	while i<limit:

		if newHypo[i]==currentHypo[i]:
			pass
		else:
			newHypo[i]="?"
		i+=1

	return newHypo

def translate_data(result):

	i=0
	limit=len(result)

	mask1={0:"Female",1:"Male"}
	mask2={0:"Old",1:"Young"}
	mask3={0:"No",1:"Yes"}
	mask4={0:"No",1:"Yes"}
	mask5={0:"Short",1:"Long"}
	mask6={0:"No",1:"Yes"}
	mask7={0:"Car",1:"House"}
	mask8={0:"No",1:"Yes"}
	mask9={0:"No",1:"Yes"}
	translationTable=[mask1,mask2,mask3,mask4,mask5,mask6,mask7,mask8,mask9]

	while i<limit:
		curList=result[i]
		for j in range(9):
			curMask=translationTable[j]
			curVal=curList[j]
			if curVal=="?":
				pass
			else:	
				result[i][j]=curMask[int(curVal)]
		i+=1

	return result

def print_summary(result):

	fp=open("partA6.txt","w")
	for lines in result:
		fp.writelines("\t".join(lines)+"\r\n")
	return

def get_miss_rate(data,hypo):

	miss=0.0
	total=0.0
	limit=len(hypo)

	for attrList in data:	
		case=1		
		for i in range(limit):
			if hypo[i]=="?":
				continue
			elif hypo[i]!=attrList[i]:
				case=0
		if case!=attrList[-1]:
			miss+=1		
		total+=1

	return miss/total

def predict(data,hypo):

	limit=len(hypo)

	for attrList in data:	
		case=1		
		for i in range(limit):
			if hypo[i]=="?":
				continue
			elif hypo[i]!=attrList[i]:
				case=0
		if case==1:
			print "high"
		else:
			print "low"

def main():
	#problem 1-5:hardcoded problems
	inputSpace_1=2**9
	conceptSpace_2=len(str(2**inputSpace_1))
	hypoSpace_3=3**9+1
	newHypoSpace_4=3**10+1
	newHypoSpace_5=4*(3**8)+1

	#problem 6-8 computation-based problems
	filename="9Cat-Train.labeled"
	data=read_input(filename)
	trainResult=generalize_data(data)
	finalHypo=list(trainResult[-1]) #shallow copy
	translate_data(trainResult)
	#print to partA6.txt
	print_summary(trainResult)
	filename1="9Cat-Dev.labeled"
	data1=read_input(filename1)
	missRate=get_miss_rate(data1,finalHypo)

	print inputSpace_1
	print conceptSpace_2
	print hypoSpace_3
	print newHypoSpace_4
	print newHypoSpace_5
	print missRate

	#problem 8
	data2=read_input(sys.argv[1])
	predict(data2,finalHypo)

if __name__=="__main__":
	main()



