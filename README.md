# Python wrapper for the ThingMagic Mercury API

The [ThingMagic Mercury API](http://www.thingmagic.com/mercuryapi) is used to discover,
provision and control ThingMagic RFID readers.

Reading RFID tags is as simple as this:

```python
import mercury
reader = mercury.Reader("tmr:///dev/ttyUSB0")

reader.set_region("EU3")
reader.set_read_plan([1], "GEN2")
print(reader.read())
```

## Installation

Build the module by running
```bash
python3 setup.py build
```
and then copy the `.so` file from the `build/lib*` directory (e.g. `build/lib.linux-i686-3.4\mercury.cpython-34m.so` into the the `site-packages` directory of your Python, e.g. `/usr/lib/python3.4/site-packages`.
