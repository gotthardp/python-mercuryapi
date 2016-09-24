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

### Windows
Use the Windows installer for the [latest release](https://github.com/gotthardp/lorawan-server/releases).

### Linux
First, make sure you have the required packages
```bash
yum install patch libxslt gcc readline-devel
```
or
```bash
apt-get install patch xsltproc gcc libreadline-dev
```

Build the module simply by running
```bash
cd python-mercuryapi
make
```
This will download and build the [Mercury API SDK](http://www.thingmagic.com/index.php/manuals-firmware)
and then it will build the Python module itself.

Then, install the module by running
```bash
sudo python3 setup.py install
```

To access ports like `/dev/ttyUSB0` as a non-root user you may need to add this
user to the `dialout` group:
```bash
sudo usermod -a -G dialout $USER
```

