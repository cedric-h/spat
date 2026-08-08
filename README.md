`cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build && cd build && make && ./Debug/spat`

`cmake -DCMAKE_BUILD_TYPE=Release -DSDL_STATIC=ON -DSDL_SHARED=OFF -S . -B build && cd build && make && ./Release/spat`
