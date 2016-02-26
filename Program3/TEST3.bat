#!/bin/bash

rm testOuts3/*.txt

echo "No Client Error Tests - Different Windows/Buffers" 
echo "- Test 1: Buffer: 500 Error: 0 Window: 10 -"
./rcopy tmp/numbers.txt testOuts3/numbers1Out.txt 500 0 10 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers1Out.txt

echo "- Test 2: Buffer: 500 Error: 0 Window: 5 -"
./rcopy tmp/numbers.txt testOuts3/numbers2Out.txt 500 0 5 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers2Out.txt

echo "- Test 3: Buffer: 500 Error: 0 Window: 1 -"
./rcopy tmp/numbers.txt testOuts3/numbers3Out.txt 500 0 1 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers3Out.txt

echo "- Test 4: Buffer: 1000 Error: 0 Window: 5 -"
./rcopy tmp/numbers.txt testOuts3/numbers4Out.txt 1000 0 5 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers4Out.txt

echo "- Test 5: Buffer: 1000 Error: 0 Window: 100 -"
./rcopy tmp/numbers.txt testOuts3/numbers5Out.txt 1000 0 100 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers5Out.txt

echo 




echo "Client Error Tests - Different Error percents" 
echo "- Test 1: Buffer: 500 Error: .1 Window: 5 -"
./rcopy tmp/numbers.txt testOuts3/numbers1Out.txt 500 .1 5 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers1Out.txt

echo "- Test 2: Buffer: 500 Error: .2 Window: 5 -"
./rcopy tmp/numbers.txt testOuts3/numbers2Out.txt 500 .2 5 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers2Out.txt

echo "- Test 3: Buffer: 500 Error: .3 Window: 5 -"
./rcopy tmp/numbers.txt testOuts3/numbers3Out.txt 500 .3 5 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers3Out.txt

echo "- Test 4: Buffer: 500 Error: .5 Window: 5 -"
./rcopy tmp/numbers.txt testOuts3/numbers4Out.txt 500 .5 5 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers4Out.txt

echo "- Test 5: Buffer: 500 Error: .6 Window: 5 -"
./rcopy tmp/numbers.txt testOuts3/numbers5Out.txt 500 .6 5 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers5Out.txt

echo "- Test 6: Buffer: 500 Error: .1 Window: 1000 -"
./rcopy tmp/numbers.txt testOuts3/numbers1Out.txt 500 .1 1000 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers1Out.txt

echo "- Test 7: Buffer: 1200 Error: .2 Window: 50 -"
./rcopy tmp/numbers.txt testOuts3/numbers2Out.txt 1200 .2 50 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers2Out.txt

echo "- Test 8: Buffer: 1000 Error: .3 Window: 1000 -"
./rcopy tmp/numbers.txt testOuts3/numbers3Out.txt 1000 .3 5 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers3Out.txt

echo "- Test 9: Buffer: 1000 Error: .5 Window: 15 -"
./rcopy tmp/numbers.txt testOuts3/numbers4Out.txt 500 .5 15 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers4Out.txt

echo "- Test 10: Buffer: 400 Error: .6 Window: 1 -"
./rcopy tmp/numbers.txt testOuts3/numbers5Out.txt 400 .6 1 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers5Out.txt

echo 




echo "Client Error Tests - Different Windows/Buffers" 
echo "- Test 1: Buffer: 500 Error: .5 Window: 10 -"
./rcopy tmp/numbers.txt testOuts3/numbers1Out.txt 500 .5 10 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers1Out.txt

echo "- Test 2: Buffer: 400 Error: .5 Window: 5 -"
./rcopy tmp/numbers.txt testOuts3/numbers2Out.txt 400 .5 5 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers2Out.txt

echo "- Test 3: Buffer: 500 Error: .5 Window: 1 -"
./rcopy tmp/numbers.txt testOuts3/numbers3Out.txt 500 .5 1 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers3Out.txt

echo "- Test 4: Buffer: 1000 Error: .5 Window: 5 -"
./rcopy tmp/numbers.txt testOuts3/numbers4Out.txt 1000 .5 5 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers4Out.txt

echo "- Test 5: Buffer: 1000 Error: .5 Window: 100 -"
./rcopy tmp/numbers.txt testOuts3/numbers5Out.txt 1000 .5 100 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers5Out.txt

echo "- Test 6: Buffer: 1200 Error: .5 Window: 30 -"
./rcopy tmp/numbers.txt testOuts3/numbers1Out.txt 1200 .5 30 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers1Out.txt

echo "- Test 7: Buffer: 1300 Error: .5 Window: 2 -"
./rcopy tmp/numbers.txt testOuts3/numbers2Out.txt 1300 .5 2 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers2Out.txt

echo "- Test 8: Buffer: 450 Error: .5 Window: 1000 -"
./rcopy tmp/numbers.txt testOuts3/numbers3Out.txt 450 .5 1000 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers3Out.txt

echo "- Test 9: Buffer: 1300 Error: .5 Window: 1 -"
./rcopy tmp/numbers.txt testOuts3/numbers4Out.txt 1300 .5 1 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers4Out.txt

echo "- Test 10: Buffer: 1393 Error: .5 Window: 10 -"
./rcopy tmp/numbers.txt testOuts3/numbers5Out.txt 1393 .5 10 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers5Out.txt

echo "- Test 11: Buffer: 1400 Error: .5 Window: 10 -"
./rcopy tmp/numbers.txt testOuts3/numbers5Out.txt 1400 .5 10 unix2 35939 

diff tmp/numbers.txt testOuts3/numbers5Out.txt

echo 



echo "Client Error Tests - Large Files"

echo "- Test 1: 2000 Buffer: 500 Error: .5 Window: 10 -"
./rcopy tmp/numbers2000.txt testOuts3/numbers2000NumbersOut.txt 500 .5 10 unix2 35939 

diff tmp/numbers2000.txt testOuts3/numbers2000NumbersOut.txt

echo "- Test 2: 10000 Buffer: 500 Error: .5 Window: 10 -"
./rcopy tmp/largeFile.txt testOuts3/largeFileNumbers.txt 500 .5 10 unix2 35939 

diff tmp/largeFile.txt testOuts3/largeFileNumbers.txt

echo "Done" 
