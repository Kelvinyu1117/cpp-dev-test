# cpp-dev-test

## Requirement

* C++ 20
* Clang++ 15
* CMake 3.14.0 or above

## Build and execute

Ninja build

```bash
mkdir build;
cd build;
cmake ../cpp-dev-test -G Ninja -DCMAKE_BUILD_TYPE=Debug
ninja install -j 4
```

Make build

```bash
mkdir build;
cd build;
cmake ../cpp-dev-test -DCMAKE_BUILD_TYPE=Debug
make install -j 4
```

The binaries and the binary schema will be in the source folder, under `cpp-dev-test/bin/bin`

Run the TCP Server

```bash
cd cpp-dev-test/bin/bin
./TCPServer <port-number> <binary-schema-file>
```

Run the TCP Client

```bash
cd cpp-dev-test/bin/bin
./TCPClient <port-number> <binary-schema-file>
```
