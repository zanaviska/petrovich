# petrovuch

pre-build command:
```
sudo apt-get install g++ make binutils cmake libssl-dev libboost-system-dev zlib1g-dev
sudo apt-get install libssl-dev libcurl4-openssl-dev
git clone https://github.com/reo7sp/tgbot-cpp
cd tgbot-cpp
cmake .
make -j4
sudo make install
```

build itself
```
cd /project/root/dir
mkdir build
cd build
cmake ..
cd ..
cmake --build build
bin/project
```