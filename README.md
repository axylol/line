# line
elf loader that only loads specific arcade games 

IMPORTANT NOTE: I will not be liable for any damages that happened while using line as it is a hacky software. EXPECT BUGS AND CRASHES!



# Building
Requires msys2 32-bit

### Configure
```sh
mkdir build
cd build
cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Release
```
### Build
```sh
cmake --build .
```

## Special Thanks
- BroGamer for exception handling support
- doomer for mental support & testing