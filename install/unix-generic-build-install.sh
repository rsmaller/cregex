gcc -I../include -c -fpic ../src/cregex_impl.c
gcc -shared -o libcregex.so cregex_impl.o
rm cregex_impl.o
sudo mkdir -p /usr/local/include/cregex
sudo mkdir -p /usr/local/lib
sudo cp libcregex.so /usr/local/lib/
sudo cp ../include/cregex.h /usr/local/include/cregex
sudo ldconfig &> /dev/null