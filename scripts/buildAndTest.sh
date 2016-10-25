#!/bin/bash

cd contrib/googletest
mkdir -p  build
cd  build
mkdir -p  googlemock
mkdir -p  googletest

cd  googlemock
cmake ../../googlemock
make

cd  ../googletest
cmake ../../googletest
make

cd  ../../../..
mkdir -p  build

g++ -std=c++1y -DASIO_STANDALONE -Isrc/include -Icontrib/asio/asio/include -Icontrib/googletest/googlemock/include -Icontrib/googletest/googletest/include -O0 -g3 -Wall -c -fmessage-length=0 -o build/ProcessorQueue.o test/conwrap/ProcessorQueue.cpp 
g++ -std=c++1y -DASIO_STANDALONE -Isrc/include -Icontrib/asio/asio/include -Icontrib/googletest/googlemock/include -Icontrib/googletest/googletest/include -O0 -g3 -Wall -c -fmessage-length=0 -o build/main.o test/main.cpp 
g++ -std=c++1y -DASIO_STANDALONE -Isrc/include -Icontrib/asio/asio/include -Icontrib/googletest/googlemock/include -Icontrib/googletest/googletest/include -O0 -g3 -Wall -c -fmessage-length=0 -o build/Processor.o test/conwrap/Processor.cpp 
g++ -std=c++1y -DASIO_STANDALONE -Isrc/include -Icontrib/asio/asio/include -Icontrib/googletest/googlemock/include -Icontrib/googletest/googletest/include -O0 -g3 -Wall -c -fmessage-length=0 -o build/ProcessorAsio.o test/conwrap/ProcessorAsio.cpp 
g++ -std=c++1y -DASIO_STANDALONE -Isrc/include -Icontrib/asio/asio/include -Icontrib/googletest/googlemock/include -Icontrib/googletest/googletest/include -O0 -g3 -Wall -c -fmessage-length=0 -o build/ProcessorCommon.o test/conwrap/ProcessorCommon.cpp 
g++ -Lcontrib/googletest/build/googlemock -Lcontrib/googletest/build/googletest -o build/conwrap build/Processor.o build/ProcessorAsio.o build/ProcessorCommon.o build/ProcessorQueue.o build/main.o -lpthread -lgmock -lgtest 

./build/conwrap
