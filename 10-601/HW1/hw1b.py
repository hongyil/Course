import sys
 
filename = sys.argv[1]
with open(filename, 'r') as f:
 	text = f.readlines()

text.reverse()
for content in text:
	print content,
