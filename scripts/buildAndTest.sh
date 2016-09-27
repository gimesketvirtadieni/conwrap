#!/bin/bash

mkdir -p  build

gcc -std=c++1y -Isrc/include -Icontrib/gmock/include -Icontrib/gmock -Icontrib/gtest/include -Icontrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o build/gmock.o contrib/gmock/src/gmock.cc 
g++ -std=c++1y -Isrc/include -Icontrib/gmock/include -Icontrib/gmock -Icontrib/gtest/include -Icontrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o build/gtest-test-part.o contrib/gtest/src/gtest-test-part.cc 
g++ -std=c++1y -Isrc/include -Icontrib/gmock/include -Icontrib/gmock -Icontrib/gtest/include -Icontrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o build/gmock-internal-utils.o contrib/gmock/src/gmock-internal-utils.cc 
g++ -std=c++1y -Isrc/include -Icontrib/gmock/include -Icontrib/gmock -Icontrib/gtest/include -Icontrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o build/gtest-port.o contrib/gtest/src/gtest-port.cc 
g++ -std=c++1y -Isrc/include -Icontrib/gmock/include -Icontrib/gmock -Icontrib/gtest/include -Icontrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o build/gtest-printers.o contrib/gtest/src/gtest-printers.cc 
g++ -std=c++1y -Isrc/include -Icontrib/gmock/include -Icontrib/gmock -Icontrib/gtest/include -Icontrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o build/gmock-spec-builders.o contrib/gmock/src/gmock-spec-builders.cc 
g++ -std=c++1y -Isrc/include -Icontrib/gmock/include -Icontrib/gmock -Icontrib/gtest/include -Icontrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o build/gtest.o contrib/gtest/src/gtest.cc 
g++ -std=c++1y -Isrc/include -Icontrib/gmock/include -Icontrib/gmock -Icontrib/gtest/include -Icontrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o build/ProcessorAsio.o test/conwrap/ProcessorAsio.cpp 
g++ -std=c++1y -Isrc/include -Icontrib/gmock/include -Icontrib/gmock -Icontrib/gtest/include -Icontrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o build/gtest-typed-test.o contrib/gtest/src/gtest-typed-test.cc 
g++ -std=c++1y -Isrc/include -Icontrib/gmock/include -Icontrib/gmock -Icontrib/gtest/include -Icontrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o build/ProcessorQueue.o test/conwrap/ProcessorQueue.cpp 
g++ -std=c++1y -Isrc/include -Icontrib/gmock/include -Icontrib/gmock -Icontrib/gtest/include -Icontrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o build/gmock-cardinalities.o contrib/gmock/src/gmock-cardinalities.cc 
g++ -std=c++1y -Isrc/include -Icontrib/gmock/include -Icontrib/gmock -Icontrib/gtest/include -Icontrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o build/gtest-filepath.o contrib/gtest/src/gtest-filepath.cc 
g++ -std=c++1y -Isrc/include -Icontrib/gmock/include -Icontrib/gmock -Icontrib/gtest/include -Icontrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o build/main.o test/main.cpp 
g++ -std=c++1y -Isrc/include -Icontrib/gmock/include -Icontrib/gmock -Icontrib/gtest/include -Icontrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o build/gmock-matchers.o contrib/gmock/src/gmock-matchers.cc 
g++ -std=c++1y -Isrc/include -Icontrib/gmock/include -Icontrib/gmock -Icontrib/gtest/include -Icontrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o build/Processor.o test/conwrap/Processor.cpp 
g++ -std=c++1y -Isrc/include -Icontrib/gmock/include -Icontrib/gmock -Icontrib/gtest/include -Icontrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o build/gtest-death-test.o contrib/gtest/src/gtest-death-test.cc 

g++ -o conwrap build/gmock-cardinalities.o build/gmock-internal-utils.o build/gmock-matchers.o build/gmock-spec-builders.o build/gmock.o build/gtest-death-test.o build/gtest-filepath.o build/gtest-port.o build/gtest-printers.o build/gtest-test-part.o build/gtest-typed-test.o build/gtest.o build/Processor.o build/ProcessorAsio.o build/ProcessorQueue.o test/main.o -lboost_system -lpthread 

