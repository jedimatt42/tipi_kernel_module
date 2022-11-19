#!/bin/bash

function outputRpiMap {
  echo PIN_31=6
  echo PIN_31_CHIP=gpiochip0
  echo PIN_32=12
  echo PIN_32_CHIP=gpiochip0
  echo PIN_33=13
  echo PIN_33_CHIP=gpiochip0
  echo PIN_35=19
  echo PIN_35_CHIP=gpiochip0
  echo PIN_36=16
  echo PIN_36_CHIP=gpiochip0
  echo PIN_37=26
  echo PIN_37_CHIP=gpiochip0
  echo PIN_38=20
  echo PIN_38_CHIP=gpiochip0
  echo PIN_40=21
  echo PIN_40_CHIP=gpiochip0
}

function searchLePotatoPin {
  CHIP=$1; shift 
  PIN=$1; shift
  NUM=`gpioinfo $CHIP | grep "\"7J1 Header Pin$PIN\"" | cut -f 1 -d':' | sed 's#^\s*line\s*##'`
  if [ ! -z ${NUM:-} ]; then
    echo "PIN_$PIN=$NUM"
    echo "PIN_${PIN}_CHIP=$CHIP"
  fi
}

# enumerate each gpio chip and map available pins

chips=`gpiodetect | cut -f1 -d' '`

for chip in $chips; do
  gpioinfo $chip | grep '7J1 Header Pin' >/dev/null 2>&1
  if [ $? -eq 0 ]; then
    for pin in 31 32 33 35 36 37 38 40; do
      searchLePotatoPin $chip $pin
    done
  else 
    gpioinfo $chip | grep 'GPIO26' >/dev/null 2>&1
    if [ $? -eq 0 ]; then
      outputRpiMap
    fi
  fi
done
