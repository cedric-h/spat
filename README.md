```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cd build && make && ./Debug/spat
```

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSDL_SHARED=OFF -DSDL_STATIC=ON
```

```
cd build && find ../src/*.c ../src/*.h | entr -rs 'make && ./Debug/spat'
```
