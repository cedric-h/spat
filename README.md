```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSDL_SHARED=OFF -DSDL_STATIC=ON
```

```
find ../src/*.c ../src/*.h | entr -rs 'make && ./Debug/spat'
```
