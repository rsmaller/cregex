$mingwpath = gcc -print-search-dirs | findstr install:.
$mingwpath = $mingwpath.Substring(9) + "include/"
gcc -I../include -c -fpic ../src/cregex_impl.c -o cregex_impl.o
gcc -shared -o cregex.dll cregex_impl.o "-Wl,--out-implib,cregex.dll.a"
Remove-Item cregex_impl.o
mkdir -Force "C:\Program Files\libcregex\"
Copy-Item cregex.dll "C:\Program Files\libcregex\"
$prevPath = [System.Environment]::GetEnvironmentVariable("Path", "Machine")
$newPath = "$prevPath;C:\Program Files\libcregex;"
[System.Environment]::SetEnvironmentVariable("Path", $newPath, "Machine")
Copy-Item ..\include\cregex.h $mingwpath