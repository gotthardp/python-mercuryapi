#!/usr/bin/env python3

import mercury
reader = mercury.Reader("tmr:///dev/ttyUSB0", baudrate=115200)

print(reader.get_model())
print(reader.get_supported_regions())

reader.set_region("EU3")
reader.set_read_plan([1], "GEN2")
print(reader.read())
