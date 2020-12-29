#/usr/bin/bash

rm -R ../../build-afl
rm -R ../../bin/AFL
cd ../../
CXX=afl-g++ cmake -DCMAKE_BUILD_TYPE=release -S src -B build-afl
cmake --build build-afl

mkdir bin/AFL
mkdir bin/AFL/in
mkdir bin/AFL/out

cp src/test/*.pump bin/AFL/in
cp bin/tcppump bin/AFL
sudo setcap cap_net_raw+eip bin/AFL/tcppump

export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1
#echo performance | sudo tee /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

cd /sys/devices/system/cpu
echo performance | sudo tee cpu*/cpufreq/scaling_governor
cd -
cd bin/AFL

echo "that's it"
echo "try: afl-fuzz -t500 -i ./in -o ./out -- ./tcppump -i lo -s @@"
