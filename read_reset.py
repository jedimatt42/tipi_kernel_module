


with open("/dev/tipi_reset", "r") as reset:
    while 1:
        c = reset.read(1)
        if c == '0':
            print(c)


