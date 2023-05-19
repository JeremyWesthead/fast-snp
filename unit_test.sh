#!/bin/bash

set -xe

#Make sure binaries are built
echo "Building..."
./build.sh
echo "Done!"

echo


#Actually run the tests
echo "Testing..."
cd test
mkdir -p cases/dummy/saves
./run_tests
cd ..
echo "Done!"