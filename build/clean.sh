#!/bin/bash

find . -name "*.o" && rm -rf *.o
find . -name "*.run" && rm -rf *.run
find . -name "*.dSYM" && rm -rf *.dSYM
