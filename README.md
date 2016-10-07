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
Import the module `mercury` and create an `mercury.Reader` object.
```python
import mercury
```

### Reader Object
Represents a connection to the reader.

#### mercury.Reader(*uri*, *baudrate=115200*)
Object constructor. Connects to the reader:
 * *uri* identifies the device communication channel:
   * `"tmr:///com2"` is a typical format to connect to a serial based module on Windows COM2
   * `"tmr:///dev/ttyUSB0"` is a typical format to connect to a USB device named ttyUSB0 on a Unix system
 * *baudrate* defines the desired communication speed.
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
['NA2', 'IN', 'JP', 'PRC', 'EU3', 'KR2', 'AU', 'NZ']
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

#### reader.set_read_plan(*antennas*, *protocol*, *read_power=default*)
Specifies the antennas and protocol to use for a search:
 * *antennas* list define which antennas (or virtual antenna numbers) to use in the search
 * *protocol* defines the protocol to search on. Supported values are:
   * `"GEN2"`, UPC GEN2
   * `"ISO180006B"`, ISO 180006B
   * `"UCODE"`, ISO 180006B UCODE
   * `"IPX64"`, IPX (64kbps link rate)
   * `"IPX256"`, IPX (256kbps link rate)
   * `"ATA"`
 * *read_power* defines the transmit power, in centidBm, for read operations. If not given,
   a reader specific default value is used.

For example:
```python
reader.set_read_plan([1], "GEN2")
```

#### reader.read()
Performs a synchronous read, and then returns a list of *TagReadData* objects resulting from the search.
If no tags were found then the list will be empty.

For example:
```python
print(reader.read())
[b'E2002047381502180820C296', b'0000000000000000C0002403']
```

#### reader.start_reading(*callback*, *on_time=250*, *off_time=0*)
Starts asynchronous reading. It returns immediately and begins a sequence of
reads or a continuous read. The results are passed to the *callback*.
The reads are repeated until the `reader.stop_reading()` method is called
 * *callback(TagReadData)* will be invoked for every tag detected
 * *on_time* sets the duration, in milliseconds, for the reader to be actively querying
 * *off_time* duration, in milliseconds, for the reader to be quiet while querying

For example:
```python
reader.start_reading(lambda tag: print(tag.epc))
b'E2002047381502180820C296'
b'0000000000000000C0002403'
```

#### reader.stop_reading()
Stops the asynchronous reading started by `reader.start_reading()`.

For example:
```python
reader.stop_reading()
```

#### reader.get_model()
Returns a model identifier for the connected reader hardware.

For example:
```python
print(reader.get_model())
M6e Nano
```

### TagReadData Object
Represents a read of an RFID tag:
 * *epc* corresponds to the Electronic Product Code
 * *antenna* indicates where the tag was read
 * *read_count* indicates how many times was the tag read during interrogation
 * *rssi* is the strength of the signal recieved from the tag

The string representation (`repr`) of the tag data is its EPC.


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
sudo make install
```

To access ports like `/dev/ttyUSB0` as a non-root user you may need to add this
user to the `dialout` group:
```bash
sudo usermod -a -G dialout $USER
```

