import sys,csv,math

class DecisionNode(object):

	def __init__(self,entropy):
		self.entropy = entropy
		self.left = None
		self.right = None	
		self.y = 0
		self.n = 0
		self.attr = 0
		self.next_attr = 0
		self.label = 0

	#for checking
	def __str__(self):
		return "attribute: %s, entropy: %d"%(self.attr,self.entropy)

# both read_data and process_data are from my "inspect.py"
def read_data(filename):

	data = []
	fp = open(filename,"r")
	spam = csv.reader(fp,delimiter=" ",quotechar='|')
	for row in spam:
		data.append("".join(row).split(","))
	return data

def process_data(data):

	limit = len(data)
	quan = len(data[0])
	decision = {"A":1,"y":1,"democrat":1,"before1950":1,"yes":1,
			"morethan3min":1,"fast":1,"expensive":1,"high":1,
			"Two":1,"large":1}
	#skip first row:attribute name
	for i in range(1,limit):
		cur = data[i]
		for j in range(quan):
			if cur[j] in decision:
				cur[j] = decision[cur[j]]
			else:
				cur[j] = 0
	return data

# for 0D data only
def calculate_entropy0(pos,neg):

	if pos == 0.0 or neg == 0.0:
		return 0

	prob = float(pos)/float(pos+neg)
	return -prob*math.log(prob,2)-(1-prob)*math.log(1-prob,2)

# for 1D list only
def calculate_entropy1(data):

	pos = float(sum(x for x in data if x))
	neg = len(data)-pos

	if pos == 0.0 or pos == len(data):
		return 0
	return calculate_entropy0(pos,neg)

# for root entropy or 2D list only
def calculate_entropy2(data):

	limit = len(data)
	decision = []

	for i in range(0,limit):
		decision.append(data[i][-1])
	pos = sum(x for x in decision if x)
	neg = len(decision)-pos

	if pos == 0.0 or pos == len(data):
		return 0
	return pos,neg,calculate_entropy0(pos,neg)

def add_node(entropy,dataList,attrList):

	limit = len(dataList)
	quan = len(attrList)
	maxInfoGain = 0
	attrChoice = 0
	#for each attribute
	for i in range(quan):	
		pos = []
		neg = []
		for j in range(0,limit):
			if dataList[j][i]:
				pos.append(dataList[j][-1])
			else:
				neg.append(dataList[j][-1])

		pos_entropy = calculate_entropy1(pos)
		pos_prob = float(len(pos))/(limit)
		neg_entropy = calculate_entropy1(neg)
		neg_prob = float(len(neg))/(limit)
		infoGain = entropy-(pos_prob*pos_entropy+neg_prob*neg_entropy)

		if infoGain>maxInfoGain:
			maxInfoGain = infoGain
			attrChoice = i
	return (attrChoice,maxInfoGain)

# build binary tree
def build_tree(dataList,attrList,attr):

	rootY = sum(x[-1] for x in dataList)
	rootN = len(dataList)-rootY

	if rootY == 0 or rootY == len(dataList):
		root = DecisionNode(0)
		root.y = rootY
		root.n = rootN
		root.attr = attr
		root.label = mark_label(rootY,dataList)
		return root

	initEntropy = calculate_entropy0(rootY,rootN)
	root = DecisionNode(initEntropy)
	root.y = rootY
	root.n = rootN
	root.attr = attr
	root.label = mark_label(rootY,dataList)

	if len(attrList) == 0:
		root.label = mark_label(rootY,dataList)
		return
	else:
		attr,entropy = add_node(root.entropy,dataList,attrList)
		root.next_attr = attr
		pos_data = []
		neg_data = []
		for inst in dataList:
			if inst[attr]:
				pos_data.append(inst)
			else:
				neg_data.append(inst)
		if len(pos_data) == 0 or len(neg_data) == 0:
			root.label = mark_label(rootY,dataList)
			return root
		attrList=list(filter(lambda attr1: attr1!=attr, attrList))
		if entropy>0.1:
			root.left=build_tree(pos_data,attrList,attr)
			root.right=build_tree(neg_data,attrList,attr)
	return root

def visualize_tree(attrList,root,left,right,depth):

	if root == None or depth>=3:
		return
	
	if depth == 0:
		print "[%d+/%d-]"%(root.y,root.n)
	else:
		if depth>1:
			print "|",
		if left:
			print attrList[root.attr],
			print "= y: [%d+/%d-]"%(root.y,root.n)
		if right:
			print attrList[root.attr],
			print "= n: [%d+/%d-]"%(root.y,root.n)
	visualize_tree(attrList,root.left,1,0,depth+1)
	visualize_tree(attrList,root.right,0,1,depth+1)

	return

def mark_label(rootY,dataList):

	if rootY<=len(dataList)/2:
		label=0
	else:
		label=1
	return label

def predict(label,root,dataList,depth):

	if root == None or depth>=3:
		return

	if dataList[root.next_attr]:
		label.append(root.label)
		predict(label,root.left,dataList,depth+1)
	else:
		label.append(root.label)
		predict(label,root.right,dataList,depth+1)
	return label

def main():
	
	if len(sys.argv)!=3:
		print "please enter trainFile and testFile"
		exit(0)
	
	train_error = 0.0
	train_file = sys.argv[1]
	train_raw_data = read_data(train_file)
	train_data = process_data(train_raw_data)
	train_attrList = train_data[0][:-1]
	train_dataList = train_data[1:]

	test_error = 0.0
	test_file = sys.argv[2]
	test_raw_data = read_data(test_file)
	test_data = process_data(test_raw_data)
	test_dataList = test_data[1:]

	root = build_tree(train_dataList,train_attrList,0)
	visualize_tree(train_attrList,root,0,0,0)

	for datalist in train_dataList:
		label = []
		predict(label,root,datalist,0)
		if label[-1]!=datalist[-1]:
			train_error += 1

	for datalist in test_dataList:
		label = []
		predict(label,root,datalist,0)
		if label[-1]!=datalist[-1]:
			test_error += 1

	print "error(train):",(train_error/len(train_dataList))
	print "error(test):",(test_error/len(test_dataList))

	return 

if __name__ == "__main__":
	main()