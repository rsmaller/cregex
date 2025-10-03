#!/usr/bin/python3
import os, pathlib

folder_name = "pyUnitTestBuild"
filepath = "unit_test_output.txt"
testCases = [
    "(\\w{3})",
    "[a-z]{2}",
    "\\d{2}",
    "(\\w{3})( \\d{3})",
    "\\d{2}(?= )",
    "(?<!a)\\w{2}",
    "[aaaaaaaaa",
    "aaaaaaa]",
    "a{aaaaa",
    "a}",
    "{3}",
    "\\w$",
    "\\w{2}$"
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
    os.system("cmake ..")
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
