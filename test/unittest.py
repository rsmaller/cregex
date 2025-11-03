#!/usr/bin/python3
import os, pathlib, shutil

script_dir = os.getcwd()
folder_name = "pyUnitTestBuild"
filepath = "unit_test_output.txt"
patternfile = open("./test/unittestpatterns.txt", "rt")
testCases = [line.rstrip("\n") for line in patternfile]

def fetch_executable():
	try:
		os.mkdir(folder_name)
	except FileExistsError:
		pass
	os.chdir(folder_name)
	try:
		os.remove(os.getcwd() + "/" + filepath)
	except FileNotFoundError:
		pass
	# os.system("cmake .. -DCMAKE_BUILD_TYPE=Release -DEXAMPLE_TYPE=Multi")
	# os.system("cmake .. -DCMAKE_BUILD_TYPE=Release -DEXAMPLE_TYPE=Single")
	os.system("cmake .. -DCMAKE_BUILD_TYPE=Debug -DEXAMPLE_TYPE=Single")
	os.system("cmake --build .")
	potentials = sorted(pathlib.Path('').glob("**/exampleregex*"))
	to_use = ""
	for item in potentials:
		if os.path.basename(item) == "exampleregex" or os.path.basename(item) == "exampleregex.exe":
			to_use = str(item)
	if to_use == "":
		raise FileNotFoundError("Example regex executable could not be located")
	exec_path = os.getcwd() + "/" + to_use
	return exec_path

def test(executable, arg, outfile):
	command = executable + " " + script_dir + "/example.txt \"" + arg + "\" >> " + outfile + " 2>&1"
	if shutil.which("valgrind"):
		command = "valgrind --leak-check=full --show-leak-kinds=all " + command
	elif shutil.which("leaks"):
		command = "leaks -atExit -- " + command
	os.system(command)

def main():
	example = fetch_executable()
	for pattern in testCases:
		test(example, pattern, filepath)

main()
