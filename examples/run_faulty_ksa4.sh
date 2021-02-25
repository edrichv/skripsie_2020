./configure_release.sh
cd build/Release
make
cd ../../examples
clear
cd ./ex_data
../../build/Release/rsfq_analyzer -a 1 -v 0 tp.csv ksa4_faulty.cir