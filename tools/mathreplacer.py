import os

#files = []
math_funcs = ["cgc_pow", "cgc_log10", "cgc_rint", "cgc_sin", "cgc_fabs"]
path = "../challenges/ASCII_Content_Server/lib/"

for fname in os.listdir(path):
	if ".c" in fname:
		f = open(path+fname, "r")
		print("Check ", fname)
		lines = f.readlines()
		print(lines)
		idx = 0 
		for lno, line in enumerate(lines):
			print(lno, line)
			if  idx == 0 and "#include" in line:
				lines.insert(lno, "#include <math.h>")
				print("put include in", fname,":", lno)
				idx = 1
			for func in math_funcs:
				if func in line:
					line.replace(func, func[4:])
					print("replace ", func, "with ", func[4:], "in ", fname,":", lno)
		f.close()
		fw = open(path+fname, "w")
		contents = ""
		contents = "".join(contents)
		print(contents)
		fw.write(contents)
		fw.close()

#if __name__== "__main__":
#main()
#findFilese()
