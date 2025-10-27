#!/usr/bin/python3
from sys import platform
import os
if platform == "linux" or platform == "linux2" or platform == "darwin":
    path = "/usr/local/lib"
else:
    path = "" #not implemented yet

os.system("sudo mkdir -p " + path)
os.system("sudo cp -r ../pyModule/cregex " + path + "/")