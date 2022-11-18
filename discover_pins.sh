#!/bin/bash

chips=`gpiodetect | cut -f1 -d' '`

function searchPotatoPin {
  CHIP=$1; shift 
  PIN=$1; shift
  NUM=`gpioinfo $CHIP | grep "\"7J1 Header Pin$PIN\"" | cut -f 1 -d':' | sed 's#^\s*line\s*##'`
  if [ ! -z ${NUM:-} ]; then
    echo "PIN_$PIN=$NUM"
    echo "PIN_${PIN}_CHIP=$CHIP"
  fi
}

for chip in $chips; do
  gpioinfo $chip | grep '7J1 Header Pin' >/dev/null 2>&1
  if [ $? -eq 0 ]; then
    for pin in 3 5 7 8 10 11 12 13 15 16 18 19 21 22 23 24 26 27 28 29 31 32 33 35 36 37 38 40; do
      searchPotatoPin $chip $pin
    done
  fi
done
