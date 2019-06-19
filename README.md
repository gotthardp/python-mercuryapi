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
On Windows, use the pre-compiled binary installer.

On Linux, build and install using `pip install python-mercuryapi`.

Note: The build process will (temporarily) require upto 500MB of free space in `/tmp`.
If your `/tmp` is smaller, use e.g. `pip install python-mercuryapi -b $HOME/tmp` to redirect.

Alternatively, you can follow the **Build Instructions** below and install the software manually.

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
   * `"llrp://192.198.1.100"` is a typical format to connect to an Ethernet device (works on Linux only)
 * *baudrate* defines the desired communication speed of the serial port.
   Supported values include 110, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 57600 and 115200 (default).
   This parameter is not allowed for network-connected readers.

For example:
```python
reader = mercury.Reader("tmr:///dev/ttyUSB0", baudrate=9600)
```
or
```python
reader = mercury.Reader("tmr://192.168.1.101")
```

#### reader.set_read_plan(*antennas*, *protocol*, *bank=[]*, *read_power=default*)
Specifies the antennas and protocol to use for a search:
 * *antennas* list define which antennas (or virtual antenna numbers) to use in the search
 * *protocol* defines the protocol to search on. Supported values are:
   * `"GEN2"`, UPC GEN2
   * `"ISO180006B"`, ISO 180006B
   * `"UCODE"`, ISO 180006B UCODE
   * `"IPX64"`, IPX (64kbps link rate)
   * `"IPX256"`, IPX (256kbps link rate)
   * `"ATA"`
 * *bank* defines the memory banks to read. Supported values are:
   * `"reserved"`
   * `"epc"`
   * `"tid"`
   * `"user"`
 * *read_power* defines the transmit power, in centidBm, for read operations. If not given,
   a reader specific default value is used.

For example:
```python
reader.set_read_plan([1], "GEN2")
```
or
```python
reader.set_read_plan([1], "GEN2", bank=["user"], read_power=1900)
```

#### reader.read(*timeout=500*)
Performs a synchronous read, and then returns a list of *TagReadData* objects resulting from the search.
If no tags were found then the list will be empty.
 * *timeout* sets the reading time

For example:
```python
print(reader.read())
[b'E2002047381502180820C296', b'0000000000000000C0002403']
```

#### reader.write(*epc_code*, *epc_target=None*)
Performs a synchronous write. Returns *True* upon success, or *False* if no tag
was found. Upon failure an exception is raised.

For example:
```python
old_epc = 'E2002047381502180820C296'
new_epc = 'E20020470000000000000012'

reader = Reader('llrp://192.168.0.2')
reader.set_read_plan([1], "GEN2")

if reader.write(epc_code=new_epc, epc_target=old_epc):
    print('Rewrited "{}" with "{}"'.format(old_epc, new_epc))
else:
    print('No tag found')
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

#### reader.read_tag_mem(*bank*, *address*, *count*, *epc_target=None*)
Reads bytes from the memory bank of a tag. Returns a *bytearray* or None if no
tag was found. Upon failure an exception is raised.

For example:
```python
print(reader.read_tag_mem(1, 0x08, 8))
bytearray(b'\x00\x00\x00\x16\x12\x00\x00\x61')
```

#### reader.write_tag_mem(*bank*, *address*, *data*, *epc_target=None*)
Writes bytes to the memory bank of a tag. Returns *True* upon success, or
*False* if no tag was found. Upon failure an exception is raised.

For example:
```python
reader.write_tag_mem(1, 0x08, bytearray(b'\x00\x00\x00\x16\x12\x00\x00\x61'))
```

#### reader.gpi_get(*pin*)
Returns value of a GPIO *pin*, or *None* is the pin is not configured as input (see `get_gpio_inputs`).

For example:
```python
print(get_gpio_inputs())
[1]
print(reader.gpi_get(1))
True
```

#### reader.gpo_set(*pin*, *value*)
Sets value of a GPIO *pin* configured as output (see `get_gpio_outputs`).

For example:
```python
print(get_gpio_outputs())
[1]
reader.gpo_set(1, False)
```

#### reader.get_model()
Returns a model identifier for the connected reader hardware.

For example:
```python
print(reader.get_model())
M6e Nano
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
    * `"IS"`, Israel
    * `"IN"`, India
    * `"JP"`, Japan
    * `"KR"`, Korea MIC
    * `"KR2"`, Korea KCC
    * `"PRC"`, China
    * `"PRC2"`
    * `"AU"`, Australia/AIDA LIPD Variation 2011
    * `"NZ"`, New Zealand

For example:
```python
reader.set_region("EU3")
```

#### reader.get_supported_regions()
Lists supported regions for the connected device.

For example:
```python
print(reader.get_supported_regions())
['NA2', 'IN', 'JP', 'PRC', 'EU3', 'KR2', 'AU', 'NZ']
```

#### reader.get_hop_table()
Gets the frequencies for the reader to use, in kHz.

#### reader.set_hop_table(*list*)
Sets the frequencies for the reader to use, in kHz.

#### reader.get_hop_time()
Gets the frequency hop time, in milliseconds.

#### reader.set_hop_time(*num*)
Sets the frequency hop time, in milliseconds.

#### reader.get_antennas()
Lists available antennas.

For example:
```python
print(reader.get_antennas())
[1, 2]
```

#### reader.get_connected_ports()
Returns numbers of the antenna ports where the reader has detected antennas.

For example:
```python
print(reader.get_connected_ports())
[1]
```

#### reader.get_power_range()
Lists supported radio power range, in centidBm.

For example:
```python
print(reader.get_power_range())
(0, 3000)
```

#### reader.get_read_powers()
Lists configured read powers for each antenna. [(antenna, power)].
The list does not include antennas with default power setting, so the list may be empty.

For example:
```python
print(reader.get_read_powers())
[(1, 1800), (2, 3000)]
```

#### reader.get_write_powers()
Lists configured write powers for each antenna. [(antenna, power)].

#### reader.set_read_powers(*powers*)
Set the read power for each listed antenna and return the real setted values.
Setted values may differ from those passed due to reader rounding.
 * *powers* list of 2-tuples that include:
    * which antenna (or virtual antenna numbers) is going to be setted
    * required power, in centidBm, for the antenna, overriding the value from
      set_read_plan or reader specific default.
      The value must be within the allowed power range.

For example:
```python
setted_powers = reader.set_read_powers([(1, 1533), (2, 1912)])
print(setted_powers)
[(1, 1525), (2, 1900)]
```

#### reader.set_write_powers(*powers*)
Set the write power for each listed antenna and return the real setted values.

#### reader.get_gpio_inputs()
Get numbers of the GPIO pins available as input pins on the device.

For example:
```python
print(reader.get_gpio_inputs())
[1, 2]
```

#### reader.set_gpio_inputs(*list*)
Set numbers of the GPIO pins available as input pins on the device.

For example:
```python
reader.set_gpio_inputs([1, 2])
```

#### reader.get_gpio_outputs()
Get numbers of the GPIO pins available as output pins on the device.

#### reader.set_gpio_outputs(*list*)
Set numbers of the GPIO pins available as output pins on the device.

On some devices this parameter is not writeable. Thus, instead of calling
`set_gpio_outputs` with the a set you may need to call `set_gpio_inputs`
with the pin omitted.

#### reader.get_gen2_blf()
Returns the current Gen2 BLF setting.

For example:
```python
print(reader.get_gen2_blf())
250
```

#### reader.set_gen2_blf(*blf*)
Sets the Gen2 BLF. Supported values include:
 * 250 (250KHz)
 * 320 (320KHz)
 * 640 (640KHz)

Not all values may be supported by a particular reader. If successful the
input value will be returned. For example:
```python
print(reader.set_gen2_blf(640))
640
```

#### reader.get_gen2_tari()
Returns the current Gen2 Tari setting.

For example:
```python
print(reader.get_gen2_tari())
0
```

#### reader.set_gen2_tari(*tari*)
Sets the Gen2 Tari. Supported values include:
 * 0 (25 us)
 * 1 (12.5 us)
 * 2 (6.25 us)

If successful the input value will be returned. For example:
```python
print(reader.set_gen2_tari(1))
1
```

#### reader.get_gen2_tagencoding()
Returns the current Gen2 TagEncoding setting.

For example:
```python
print(reader.get_gen2_tagencoding())
0
```

#### reader.set_gen2_tagencoding(*tagencoding*)
Sets the Gen2 TagEncoding. Supported values include:
 * 0 (FM0)
 * 1 (M = 2)
 * 2 (M = 4)
 * 3 (M = 8)

If successful the input value will be returned. For example:
```python
print(reader.set_gen2_tagencoding(2))
2
```

#### reader.get_gen2_session()
Returns the current Gen2 Session setting.

For example:
```python
print(reader.get_gen2_session())
0
```

#### reader.set_gen2_session(*session*)
Sets the Gen2 Session. Supported values include:
 * 0 (S0)
 * 1 (S1)
 * 2 (S2)
 * 3 (S3)

If successful the input value will be returned. For example:
```python
print(reader.set_gen2_session(2))
2
```

#### reader.get_gen2_target()
Returns the current Gen2 Target setting.

For example:
```python
print(reader.get_gen2_target())
0
```

#### reader.set_gen2_target(*target*)
Sets the Gen2 Target. Supported values include:
 * 0 (A)
 * 1 (B)
 * 2 (AB)
 * 3 (BA)

If successful the input value will be returned. For example:
```python
print(reader.set_gen2_target(2))
2
```

#### reader.get_gen2_q()
Returns the current Gen2 Q setting as a tuple containing the current Q type,
and initial Q value.

For example:
```python
print(reader.get_gen2_q())
(0, 16)
```

#### reader.set_gen2_q(*qtype*, *initialq*)
Sets the Gen2 Q.
 * *qtype* defines Dynamic vs Static Q value where:
   * 0 (Dynamic)
   * 1 (Static)
 * *initialq* defines 2^*initialq* time slots to be used initially for tag communication.

If Dynamic Q is used then the input *initialq* value is ignored as the reader
will choose this on its own. It is then likely for *initialq* on a get to be different than the value used on a set.

If successful the input value will be returned. For example:
```python
print(reader.set_gen2_q(0, 4))
(0, 4)
print(reader.get_gen2_q())
(0, 64)
```
or
```python
print(reader.set_gen2_q(1, 4))
(1, 4)
print(reader.get_gen2_q())
(1, 4)
```

#### reader.get_temperature()
Returns the chip temperature in degrees of Celsius.

### TagReadData Object
Represents a read of an RFID tag:
 * *epc* corresponds to the Electronic Product Code
 * *phase* of the tag response
 * *antenna* indicates where the tag was read
 * *read_count* indicates how many times was the tag read during interrogation
 * *rssi* is the strength of the signal recieved from the tag
 * *frequency* the tag was read with
 * *epc_mem_data* contains the EPC bank data bytes
 * *tid_mem_data* contains the TID bank data bytes
 * *user_mem_data* contains the User bank data bytes
 * *reserved_mem_data* contains the Reserved bank data bytes

```python
print(tag.epc)
b'E2000087071401930700D206'
print(tag.antenna)
2
print(tag.read_count)
2
print(tag.rssi)
-65
print(tag.user_mem_data)
bytearray(b'\x00\x00\x00...')
```

The string representation (`repr`) of the tag data is its EPC.

```python
print(tag)
b'E2000087071401930700D206'
```

Please note that the bank data bytes need to be requested via the *bank* parameter
of the reader.*set_read_plan* function. Data not requested will not be read.


## Build Instructions

### Windows
Use the Windows installer for the
[latest release](https://github.com/gotthardp/python-mercuryapi/releases) and Python 3.6.

If you get the "ImportError: DLL load failed", make sure you have the
[Microsoft Visual C++ 2010 Redistributable Package](https://www.microsoft.com/en-us/download/details.aspx?id=5555)
installed.

To build an installer for other Python releases you need to:
 * Download the latest [Mercury API](https://www.jadaktech.com/documentation/rfid/mercuryapi), e.g.
   [mercuryapi-1.31.2.zip](https://www.jadaktech.com/wp-content/uploads/2018/11/mercuryapi-1.31.2.zip).
 * Go to mercuryapi-1.31.2.40\c\src\api\ltkc_win32 and run `gencode.bat`
 * Open mercuryapi-1.31.2.40\c\src\api\ltkc_win32\inc\stdint_win32.h and comment (or delete)
   the block of `typedef` for `int_fast8_t` through `uint_fast64_t` (8 lines)
 * Download the [latest pthreads-win32](ftp://sourceware.org/pub/pthreads-win32/dll-latest)
   binaries (both .dll and .lib) for your architecture and put them into
   mercuryapi-1.31.2.40\c\src\pthreads-win32\x86 or \x64
 * Obtain Microsoft Visual Studio 2017, including the Python extensions
 * Open the Solution and review the
   [setup-win.py](https://github.com/gotthardp/python-mercuryapi/blob/master/setup-win.py)
   * Verify the `mercuryapi` directory
   * Set `library_dirs` and `data_files` to the pthreads-win32 you downloaded
   * Set Script Arguments to `bdist_wininst -p win32` (default) or `bdist_wininst -p amd64`
 * Start setup-win.py (without debugging)

### Linux
First, make sure you have the required packages
```bash
yum install unzip patch libxslt gcc readline-devel python-devel python-setuptools
```
or
```bash
apt-get install unzip patch xsltproc gcc libreadline-dev python-dev python-setuptools
```

Both Python 2.x and Python 3.x are supported. To use the Python 3.x you may need to
install the `python3-dev[evel]` instead of the `python-dev[evel]` packages.

Build the module simply by running
```bash
cd python-mercuryapi
make
```
This will download and build the [Mercury API SDK](http://www.thingmagic.com/index.php/manuals-firmware)
and then it will build the Python module itself.

The `make` command will automatically determine which Python version is installed. If both
2.x and 3.x are installed, the 3.x takes precedence. To build and install 2.x you need to
explicitly specify the Python interpreter to use:
```bash
sudo make PYTHON=python
```

Then, install the module by running
```bash
sudo make install
```
which is a shortcut to running
```bash
sudo python setup.py install
```

If you are getting a "Module not found" error, please double check that you built and
installed the module using the same Python version (2 or 3) you now use to run your script.
(Or simply build and install it twice: once with python2 and once with python3.)

To access ports like `/dev/ttyUSB0` as a non-root user you may need to add this
user to the `dialout` group:
```bash
sudo usermod -a -G dialout $USER
```

## Copyright and Licensing

The python-mercuryapi is distributed under the terms of the MIT License.
See the [LICENSE](LICENSE).

Copyright (c) 2016-2019 Petr Gotthard
