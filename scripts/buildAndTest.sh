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

mkdir -p  build
cd  build

g++ -std=c++1y -DASIO_STANDALONE -Isrc/include -Icontrib/asio/asio/include -Icontrib/googletest/googlemock/include -Icontrib/googletest/googletest/include -O0 -g3 -Wall -c -fmessage-length=0 -o main.o test/main.cpp 
g++ -std=c++1y -DASIO_STANDALONE -Isrc/include -Icontrib/asio/asio/include -Icontrib/googletest/googlemock/include -Icontrib/googletest/googletest/include -O0 -g3 -Wall -c -fmessage-length=0 -o ProcessorQueue.o test/conwrap/ProcessorQueue.cpp 
g++ -std=c++1y -DASIO_STANDALONE -Isrc/include -Icontrib/asio/asio/include -Icontrib/googletest/googlemock/include -Icontrib/googletest/googletest/include -O0 -g3 -Wall -c -fmessage-length=0 -o ProcessorAsio.o test/conwrap/ProcessorAsio.cpp 
g++ -std=c++1y -DASIO_STANDALONE -Isrc/include -Icontrib/asio/asio/include -Icontrib/googletest/googlemock/include -Icontrib/googletest/googletest/include -O0 -g3 -Wall -c -fmessage-length=0 -o Processor.o test/conwrap/Processor.cpp 

g++ -Lcontrib/googletest/build/googlemock -Lcontrib/googletest/build/googletest -o conwrap Processor.o ProcessorAsio.o ProcessorQueue.o main.o -lpthread -lgmock -lgtest 

./conwrap
