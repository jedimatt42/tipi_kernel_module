#!/bin/bash

# Requires gpiod tools... gpiomon
. ./pinmap.sh


while true; do
  gpiomon -f -s -n 1 -B pull-up $PIN_37_CHIP $PIN_37
  if [ $? -eq 0 ]; then
    echo "4A reset"
    sleep 1
  fi
done

