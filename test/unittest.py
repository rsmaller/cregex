#!/usr/bin/python3
import os, pathlib

folder_name = "pyUnitTestBuild"
filepath = "unit_test_output.txt"
testCases = [
    r"(ay|be|ce)",
    r"(?=a{3})b",
    r"b(?<=a{3})",
    r"(?<=\w{3})b",
    r"(\w{3})( \d{3})",
    r"(\w{3})(?= )",
    r"(|b)",
    r"(|b|c)",
    r"(a|b)",
    r"(a|)",
    r"(a|b|)",
    r"(a|b|c)",
    r"a{,3}",
    r"a{3,}",
    r"a{,3}",
    r"a{3,3}",
    r"a{3}",
    r"a{,}",
    r"[]",
    r"[^]",
    r"[a\-z]",
    r"[a-z",
    r"(abc",
    r"{}",
    r"a{}",
    r"a{3{2}}",
    r"",
    r"[\\]",
    r"[\\]]",
    r"(|a)",
    r"[a|b]",
    r"\(abc\)",
    r"(abc\))",
    r"abc\\",
    r"(\w{1,4}|\d{1,4})",
    r"(\w{4}|\d{4})",
    r"y(b|e)",
    r"\d{1,4}",
    r"(?<=\n)\w{4} \d{2}",
    r"^\w{3}",
    r"\d ?\d",
    r"\d*",
    r"\d+",
    r"\d\d?\d",
    r"(\w{3})",
    r"[a-z]{2}",
    r"[^a-z]{2}",
    r"[a^z]{2}",
    r"\d{2}",
    r"\d{2}(?= )",
    r"(?<!a)\w{2}",
    r"[aaaaaaaaa",
    r"aaaaaaa]",
    r"a{aaaaa",
    r"a}",
    r"{3}",
    r"\w$",
    r"\w$a",
    r"\w$b",
    r"\w (?=6)\d{2}",
    r"\d",
    r"\w{2}",
    r"\A\w{2}"
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
    os.system("cmake .. -DCMAKE_BUILD_TYPE=Debug")
    os.system("cmake --build .")
    potentials = sorted(pathlib.Path('.').glob("**/exampleregex*"))
    to_use = ""
    for item in potentials:
        if os.path.basename(item) == "exampleregex" or os.path.basename(item) == "exampleregex.exe":
            to_use = str(item)
    if to_use == "":
        raise FileNotFoundError("Example regex executable could not be located")
    exec_path = os.getcwd() + "/" + to_use
    return exec_path

def test(executable, arg, outfile):
    os.system(executable + " \"" + arg + "\" >> " + outfile + " 2>&1")

def main():
    example = fetch_executable()
    for pattern in testCases:
        test(example, pattern, filepath)

main()
