#!/usr/bin/python3

import select
import os
import time

tipi_reset='/dev/tipi_reset'

with open(tipi_reset, 'r') as reset_value:
  poller = select.poll()
  poller.register(reset_value, select.POLLIN | select.POLLERR )

  while True:
    poller.poll()
    print('reset')
