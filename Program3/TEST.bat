#!/bin/bash

echo "No Client Error Tests" 
echo "- Test 1: Buffer: 500 Error: 0 Window: 10 -"
./rcopy tmp/numbers.txt testOuts/numbers1Out.txt 500 0 10 unix2 46721 >> testOuts/test1Out.txt 2>&1

diff tmp/numbers.txt testOuts/numbers1Out.txt

echo "- Test 2: Buffer: 500 Error: 0 Window: 5 -"
./rcopy tmp/numbers.txt testOuts/numbers2Out.txt 500 0 5 unix2 46721 >> testOuts/test2Out.txt 2>&1

diff tmp/numbers.txt testOuts/numbers2Out.txt

echo "- Test 3: Buffer: 500 Error: 0 Window: 1 -"
./rcopy tmp/numbers.txt testOuts/numbers3Out.txt 500 0 1 unix2 46721 >> testOuts/test3Out.txt 2>&1

diff tmp/numbers.txt testOuts/numbers3Out.txt

echo "- Test 4: Buffer: 1000 Error: 0 Window: 5 -"
./rcopy tmp/numbers.txt testOuts/numbers4Out.txt 1000 0 5 unix2 46721 >> testOuts/test4Out.txt 2>&1

diff tmp/numbers.txt testOuts/numbers4Out.txt

echo "- Test 5: Buffer: 1000 Error: 0 Window: 100 -"
./rcopy tmp/numbers.txt testOuts/numbers5Out.txt 1000 0 100 unix2 46721 >> testOuts/test5Out.txt 2>&1

diff tmp/numbers.txt testOuts/numbers5Out.txt

echo 

echo "Client Error Tests" 
echo "- Test 1: Buffer: 500 Error: .1 Window: 5 -"
./rcopy tmp/numbers.txt testOuts/numbers1Out.txt 500 .1 5 unix2 46721 >> testOuts/test1Out.txt 2>&1

diff tmp/numbers.txt testOuts/numbers1Out.txt

echo "- Test 2: Buffer: 500 Error: .2 Window: 5 -"
./rcopy tmp/numbers.txt testOuts/numbers2Out.txt 500 .2 5 unix2 46721 >> testOuts/test2Out.txt 2>&1

diff tmp/numbers.txt testOuts/numbers2Out.txt

echo "- Test 3: Buffer: 500 Error: .3 Window: 5 -"
./rcopy tmp/numbers.txt testOuts/numbers3Out.txt 500 .3 5 unix2 46721 >> testOuts/test3Out.txt 2>&1

diff tmp/numbers.txt testOuts/numbers3Out.txt

echo "- Test 4: Buffer: 500 Error: .5 Window: 5 -"
./rcopy tmp/numbers.txt testOuts/numbers4Out.txt 500 .5 5 unix2 46721 >> testOuts/test4Out.txt 2>&1

diff tmp/numbers.txt testOuts/numbers4Out.txt

echo "- Test 5: Buffer: 500 Error: .6 Window: 5 -"
./rcopy tmp/numbers.txt testOuts/numbers5Out.txt 500 .6 5 unix2 46721 >> testOuts/test5Out.txt 2>&1

diff tmp/numbers.txt testOuts/numbers5Out.txt

echo 

echo "Done" 
