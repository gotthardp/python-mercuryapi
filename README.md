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
Represents a connection to the reader.

#### mercury.Reader(*uri*, *baudrate=115200*)
Connects to the reader:
 * *uri* identifies the device communication channel
   * `"tmr:///com2"` is a typical format to connect to a serial based module on Windows COM2
   * `"tmr:///dev/ttyUSB0"` is a typical format to connect to a USB device named ttyUSB0 on a Unix system
 * *baudrate* defines the desired speed.
   Supported values include 110, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 57600 and 115200 (default).

For example:
```python
reader = mercury.Reader("tmr:///dev/ttyUSB0", baudrate=9600)
```

#### reader.get_supported_regions()
Lists supported regions for the connected device.

For example:
```python
print(reader.get_supported_regions())
```
#### reader.set_region(*region*)
Controls the Region of Operation for the connected device:
 * *region* represents the regulatory region that the device will operate in. Supported values are:
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
#### reader.set_read_plan(*antennas*, *protocol*)
Specifies the antennas and protocol to use for a search:
 * *antennas* list define which antennas (or virtual antenna numbers) to use in the search
 * *protocol* defines the protocol to search on. Supported values are:
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
#### reader.read()
Performs a synchronous read, and then returns a list of tag information resulting from the search.
If no tags were found then the list will be empty.

For example:
```python
print(reader.read())
```
#### reader.get_model()
Returns a model identifier for the connected reader hardware.

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

