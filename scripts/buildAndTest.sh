#!/bin/bash

mkdir -p  build
cd build

g++ -std=c++1y -I../src/include -I../contrib/gmock/include -I../contrib/gmock -I../contrib/gtest/include -I../contrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o contrib/gmock/src/gmock.o ../contrib/gmock/src/gmock.cc 
g++ -std=c++1y -I../src/include -I../contrib/gmock/include -I../contrib/gmock -I../contrib/gtest/include -I../contrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o contrib/gtest/src/gtest-test-part.o ../contrib/gtest/src/gtest-test-part.cc 
g++ -std=c++1y -I../src/include -I../contrib/gmock/include -I../contrib/gmock -I../contrib/gtest/include -I../contrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o contrib/gmock/src/gmock-internal-utils.o ../contrib/gmock/src/gmock-internal-utils.cc 
g++ -std=c++1y -I../src/include -I../contrib/gmock/include -I../contrib/gmock -I../contrib/gtest/include -I../contrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o contrib/gtest/src/gtest-port.o ../contrib/gtest/src/gtest-port.cc 
g++ -std=c++1y -I../src/include -I../contrib/gmock/include -I../contrib/gmock -I../contrib/gtest/include -I../contrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o contrib/gtest/src/gtest-printers.o ../contrib/gtest/src/gtest-printers.cc 
g++ -std=c++1y -I../src/include -I../contrib/gmock/include -I../contrib/gmock -I../contrib/gtest/include -I../contrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o contrib/gmock/src/gmock-spec-builders.o ../contrib/gmock/src/gmock-spec-builders.cc 
g++ -std=c++1y -I../src/include -I../contrib/gmock/include -I../contrib/gmock -I../contrib/gtest/include -I../contrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o contrib/gtest/src/gtest.o ../contrib/gtest/src/gtest.cc 
g++ -std=c++1y -I../src/include -I../contrib/gmock/include -I../contrib/gmock -I../contrib/gtest/include -I../contrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o test/conwrap/ProcessorAsio.o ../test/conwrap/ProcessorAsio.cpp 
g++ -std=c++1y -I../src/include -I../contrib/gmock/include -I../contrib/gmock -I../contrib/gtest/include -I../contrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o contrib/gtest/src/gtest-typed-test.o ../contrib/gtest/src/gtest-typed-test.cc 
g++ -std=c++1y -I../src/include -I../contrib/gmock/include -I../contrib/gmock -I../contrib/gtest/include -I../contrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o test/conwrap/ProcessorQueue.o ../test/conwrap/ProcessorQueue.cpp 
g++ -std=c++1y -I../src/include -I../contrib/gmock/include -I../contrib/gmock -I../contrib/gtest/include -I../contrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o contrib/gmock/src/gmock-cardinalities.o ../contrib/gmock/src/gmock-cardinalities.cc 
g++ -std=c++1y -I../src/include -I../contrib/gmock/include -I../contrib/gmock -I../contrib/gtest/include -I../contrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o contrib/gtest/src/gtest-filepath.o ../contrib/gtest/src/gtest-filepath.cc 
g++ -std=c++1y -I../src/include -I../contrib/gmock/include -I../contrib/gmock -I../contrib/gtest/include -I../contrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o test/main.o ../test/main.cpp 
g++ -std=c++1y -I../src/include -I../contrib/gmock/include -I../contrib/gmock -I../contrib/gtest/include -I../contrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o contrib/gmock/src/gmock-matchers.o ../contrib/gmock/src/gmock-matchers.cc 
g++ -std=c++1y -I../src/include -I../contrib/gmock/include -I../contrib/gmock -I../contrib/gtest/include -I../contrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o test/conwrap/Processor.o ../test/conwrap/Processor.cpp 
g++ -std=c++1y -I../src/include -I../contrib/gmock/include -I../contrib/gmock -I../contrib/gtest/include -I../contrib/gtest -O0 -g3 -Wall -c -fmessage-length=0 -o contrib/gtest/src/gtest-death-test.o ../contrib/gtest/src/gtest-death-test.cc 

g++ -o conwrap contrib/gmock/src/gmock-cardinalities.o contrib/gmock/src/gmock-internal-utils.o contrib/gmock/src/gmock-matchers.o contrib/gmock/src/gmock-spec-builders.o contrib/gmock/src/gmock.o contrib/gtest/src/gtest-death-test.o contrib/gtest/src/gtest-filepath.o contrib/gtest/src/gtest-port.o contrib/gtest/src/gtest-printers.o contrib/gtest/src/gtest-test-part.o contrib/gtest/src/gtest-typed-test.o contrib/gtest/src/gtest.o test/conwrap/Processor.o test/conwrap/ProcessorAsio.o test/conwrap/ProcessorQueue.o test/main.o -lboost_system -lpthread 

