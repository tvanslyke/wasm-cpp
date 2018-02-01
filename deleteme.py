from sys import stdin

instrs = set()
for line in stdin:
	if "= 0x" in line:
		instrs.add(int(line.strip()[-5 : -1], 16))

noninstrs = []
for i in range(256):
	if i not in instrs:
		noninstrs.append(i)
noninstrs.sort()
print(noninstrs)
print(len(noninstrs))

# for item in instrs:
# 	print(item)
