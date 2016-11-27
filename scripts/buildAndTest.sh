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

cd  ../../../../make
make tests

./conwrap
