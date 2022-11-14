#!/bin/bash

# Requires gpiod tools... gpiomon

while true; do
  gpiomon -f -s -n 1 -B pull-up gpiochip0 26
  if [ $? -eq 0 ]; then
    echo "4A reset"
    sleep 1
  fi
done

