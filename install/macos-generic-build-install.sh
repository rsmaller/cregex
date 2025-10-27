gcc -I../include -c -fpic ../src/cregex_impl.c
gcc -dynamiclib -shared -o libcregex.dylib cregex_impl.o -install_name @rpath/libcregex.dylib
rm cregex_impl.o
sudo mkdir -p /usr/local/include/cregex
sudo mkdir -p /usr/local/lib
sudo cp libcregex.dylib /usr/local/lib/
sudo cp ../include/cregex.h /usr/local/include/cregex