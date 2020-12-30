#/usr/bin/bash

printf "\n**** removing old AFL build\n"
rm -Rf ../../build-afl
rm -Rf ../../bin/AFL


printf "**** compile\n"
cd ../../
cmake -DCMAKE_C_COMPILER=afl-gcc -DCMAKE_CXX_COMPILER=afl-g++ -DCMAKE_BUILD_TYPE=release -S src -B build-afl
cmake --build build-afl

printf "**** preparing test cases\n"
mkdir bin/AFL
mkdir bin/AFL/in
mkdir bin/AFL/out

split -l 1 src/test/all.pump bin/AFL/in/all_
cp src/test/delay-24.pump bin/AFL/in


printf "**** prepare test binary\n"
cp bin/tcppump bin/AFL
sudo setcap cap_net_raw+eip bin/AFL/tcppump

printf "**** configure environment\n"
export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1
#echo performance | sudo tee /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

cd /sys/devices/system/cpu
echo performance | sudo tee cpu*/cpufreq/scaling_governor
cd -
cd bin/AFL

echo "that's it"
echo "try: afl-fuzz -t500 -i ./in -o ./out -- ./tcppump -i lo -s @@"
echo "**************************************************************"

