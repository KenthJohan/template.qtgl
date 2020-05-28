```
git clone https://github.com/nanomsg/nng
cd nng
mkdir build
cd build
cmake .. -G"MSYS Makefiles" -DCMAKE_INSTALL_PREFIX=$MINGW_PREFIX
make
make install
```