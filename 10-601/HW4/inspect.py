import sys,csv,math

def read_data(filename):

	data = []
	fp = open(filename,"r")
	spam = csv.reader(fp,delimiter=" ",quotechar='|')
	for row in spam:
		data.append("".join(row).split(","))
	return data

def process_data(data):

	pos = 0.0
	error = 0.0
	limit = len(data)
	decision = {"A":1,"y":1,"democrat":1,"before1950":1,"yes":1,
			"morethan3min":1,"fast":1,"expensive":1,"high":1,
			"Two":1,"large":1}
	#skip first row:attribute name
	for i in range(1,limit):
		cur = data[i]
		if cur[-1] in decision:
			pos += decision[cur[-1]]

	prob=float(pos)/float(limit-1)

	if prob == 0.0 or prob == 1.0:
		error = 0.0
		entropy = 0.0

	error = prob
	entropy = calculate_entropy(prob)

	#not majority
	if error > 0.5:
		error = 1-error

	return entropy,error

def calculate_entropy(pos):

	return (-pos*math.log(pos,2)-(1-pos)*math.log((1-pos),2))

def main():
	
	if len(sys.argv)!=2:
		print "please enter correct file"
		exit(1)

	data = read_data(sys.argv[1])
	entropy,error = process_data(data)
	print "entropy:",entropy
	print "error:",error

if __name__ == "__main__":
	main()