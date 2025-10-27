#!/usr/bin/python3
import os, pathlib, shutil

script_dir = os.getcwd()
folder_name = "pyUnitTestBuild"
filepath = "unit_test_output.txt"
testCases = [
	r"\b\w{3}", r"\w{3}\b",	r"\b\w{3}\b", r"(\w{3}$)", r"(\w{3})$", r"(^\w{3})", r"^(\w{3})",
	r"\w(?=$)",	r"(?<=^)\w", r"[Rr]eg(?:ular)? ?[Ee]x(?:p|pr|pression)?",
	r"\b[Rr]eg(?:ular)? ?[Ee]x(?:p|pr|pression)?\b",
	r"(?:\w{3}|\d{3})", r"(?:\w{3}){2}", r"\d(?=[a-z]|2)",
	r"\.(?=\d{3})", r"(\d| )?\w", r"(\d)?\w", r"(\d)*", r"(\d){1,3}", r"(?<=path)\w*", r"^\w{2}",
	r"\w{3}$", r"a{2}+",
	r"(1|9|2){3}", r"(\d|-){3}", r"(\d|a){3}",
	r"(\d){3}", r"\w{1,2}",
	r"\w{,2}", r"(\w){3}", r"(\w{,2}){3}", r"(\w{1,2}){3}", r"(\w{2}){3}",
    r"\d{1,3}(\.\d{1,3}){3}", r"\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}", r"(ay|be|ce)",
	r"(?=a{3})b", r"b(?<=a{3})", r"(?<=\w{3})b", r"(\w{3})( \d{3})", r"(\w{3})(?= )",
	r"(|b)", r"(|b|c)", r"(a|b)", r"(a|)", r"(a|b|)", r"(a|b|c)",
	r"a{,3}",
	r"a{3,}", r"a{,3}", r"a{3,3}", r"a{3}", r"a{,}", r"[]", r"[^]", r"[a\-z]",
	r"[a-z", r"(abc", r"{}", r"a{}", r"a{3{2}}", r"", r"[\\]", r"[\\]]", r"(|a)",
	r"[a|b]", r"\(abc\)", r"(abc\))", r"abc\\",
	r"(\w{1,4}|\d{1,4})", r"(\w{4}|\d{4})", r"y(b|e)",
	r"\d{1,4}", r"(?<=\n)\w{4} \d{2}", r"^\w{3}", r"\d ?\d", r"\d*", r"\d+",
	r"\d\d?\d", r"(\w{3})", r"[a-z]{2}", r"[^a-z]{2}", r"[a^z]{2}", r"\d{2}", r"\d{2}(?= )",
	r"(?<!a)\w{2}", r"[aaaaaaaaa", r"aaaaaaa]", r"a{aaaaa", r"a}", r"{3}", r"\w$", r"\w$a",
	r"\w$b", r"\w (?=6)\d{2}", r"\d", r"\w{2}", r"\A\w{2}",
	r"(\w{2}|\d){3}",
]

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
	# os.system("cmake .. -DCMAKE_BUILD_TYPE=Debug -DEXAMPLE_TYPE=Single -DSANITIZE=Address")
	os.system("cmake .. -DCMAKE_BUILD_TYPE=Release -DEXAMPLE_TYPE=Single")
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
	os.system(command)

def main():
	example = fetch_executable()
	for pattern in testCases:
		test(example, pattern, filepath)

main()
