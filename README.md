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

## Usage
```python
import mercury
```

### Reader Object
mercury.Reader(*uri*, *baudrate=115200*)
 * *uri* identifies the device communication channel
 * *baudrate* defines the desired speed

For example:
```python
reader = mercury.Reader("tmr:///dev/ttyUSB0", baudrate=9600)
```

#### get_supported_regions
reader.get_supported_regions()

For example:
```python
print(reader.get_supported_regions())
```
#### set_region
reader.set_region(*region*)
 * *region* represents the different regulatory regions that the device may operate in. Supported values are:
    * `"NA"`, North America/FCC
    * `"NA2"`
    * `"NA3"`
    * `"EU"`, European Union/ETSI EN 302 208
    * `"EU2"`, European Union/ETSI EN 300 220
    * `"EU3"`, European Union/ETSI Revised EN 302 208
    * `"KR"`, Korea MIC
    * `"KR2"`, Korea KCC
    * `"IN"`, India
    * `"JP"`, Japan
    * `"PRC"`, China
    * `"PRC2"`
    * `"AU"`, Australia/AIDA LIPD Variation 2011
    * `"NZ"`, New Zealand

For example:
```python
reader.set_region("EU3")
```
#### set_read_plan
reader.set_read_plan(*antennas*, *protocol*)
 * *antennas* list define which antennas (or virtual antenna numbers) to use in the search
 * *protocol* defines the protocol to search on. . Supported values are:
   * `"GEN2"`, UPC GEN2
   * `"ISO180006B"`, ISO 180006B
   * `"UCODE"`, ISO 180006B UCODE
   * `"IPX64"`, IPX (64kbps link rate)
   * `"IPX256"`, IPX (256kbps link rate)
   * `"ATA"`

For example:
```python
reader.set_read_plan([1], "GEN2")
```
#### read
reader.read()

For example:
```python
print(reader.read())
```
#### get_model
reader.get_model()

For example:
```python
print(reader.get_model())
```

## Installation

### Windows
Use the Windows installer for the [latest release](https://github.com/gotthardp/python-mercuryapi/releases) and Python 3.5.

Please let me know if you need other Windows installers.

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

