#!/bin/bash

sudo ./discover_pins.sh >pinmap.sh

make clean
make
