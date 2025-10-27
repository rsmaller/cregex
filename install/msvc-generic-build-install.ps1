$msvcver = (Get-ChildItem "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC" -Name | Sort-Object Name -Descending)[0]
$msvcpath = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\" + $msvcver + "\include"
Copy-Item ../include/cregex.h $msvcpath
cl /LD ../src/cregex_impl.c /Fe:cregex.dll
mkdir -Force "C:\Program Files\libcregex\"
Copy-Item cregex.dll "C:\Program Files\libcregex\"
$prevPath = [System.Environment]::GetEnvironmentVariable("Path", "Machine")
$newPath = "$prevPath;C:\Program Files\libcregex;"
[System.Environment]::SetEnvironmentVariable("Path", $newPath, "Machine")
Remove-Item cregex.exp
Remove-Item cregex_impl.obj