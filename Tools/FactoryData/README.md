# Factory Block generator

## Purpose

Matter requires device specific data to be stored in program flash for use at runtime.
This data is computed at the factory line and flashed together with the firmware image.

To facilitate this process the factory block was created, a simple datastructure based on the TLV (type/length/value)
concept.  No CRC or other content verification is done at runtime.

The factory block can store:

* DAC certificate (device attestation certificate)
* DAC key
* PAI certificate (product attestation certificate)
* CD certificate (certificate declaration)
* spake2p items
* customer specific data if any free space is available

The factory block flash region is only writable during production line flashing, it can not be changed later on.

## Implementation overview

The implementation consists of 4 parts:

* A data-format as means of data-exchange between factory line and firmware.
* A python tool to generate the binary data section content.
* C code to look up the stored data at runtime.
* A Makefile to facilitate 'integration into development flow' by re-generating the factory data payload before the firmware linking step.

## Data format

The file starts with a header word reading 'QFDA' (for Qorvo factory data in ASCII).

After this, the data is stored encoded as a simple type/length/data format. This means for each element stored, three items are stored:
* A type word (uint32), identifying what kind of data is to follow
* A length word (uint32) indicating the length of the data that will follow
* A variable length Value.

The last TLV element shall be an 'End marker'; an element of type 0 and length 0, stored as [0x00, 0x00].

Every TLV element shall be word-aligned, so padding bytes (0x00) need to be added after the variable length value when
necessary to align the type word correctly to a 32-bit word boundary.

## Usage

    % python3 generate_factory_data.py --help

    usage: generate_factory_data.py [-h] [-p PASSCODE] [-d DISCRIMINATOR] [--dac-cert DAC_CERT] [--dac-key DAC_KEY] [--pai-cert PAI_CERT] [--certification-declaration CERTIFICATION_DECLARATION] [-s MAXIMUM_SIZE]
                                    [--out_file OUT_FILE] [--data DATA] [--vendor-name VENDOR_NAME] [--vendor-id VENDOR_ID] [--product-name PRODUCT_NAME] [--product-id PRODUCT_ID] [--serial-num SERIAL_NUM] [--manuf-date MANUF_DATE]
                                    [--hw-ver HW_VER] [--hw-ver-str HW_VER_STR] [--unique-id UNIQUE_ID] [--enable-key ENABLE_KEY] [--write-depfile-and-exit WRITE_DEPFILE_AND_EXIT]

    Chip Factory NVS binary generator tool

    options:
      -h, --help            show this help message and exit
      -p PASSCODE, --passcode PASSCODE
                            The discriminator for pairing, range: 0x01-0x5F5E0FE
      -d DISCRIMINATOR, --discriminator DISCRIMINATOR
                            The passcode for pairing, range: 0x00-0x0FFF
      --dac-cert DAC_CERT   (1) The path to the DAC certificate in der format
      --dac-key DAC_KEY     (1) The path to the DAC private key in der format
      --pai-cert PAI_CERT   (1) The path to the PAI certificate in der format
      --certification-declaration CERTIFICATION_DECLARATION
                            (1) The path to the certificate declaration der format
      -s MAXIMUM_SIZE, --maximum-size MAXIMUM_SIZE
                            The maximum size of the factory blob, default: 0x6000
      --out_file OUT_FILE   Path to output file (.bin file)
      --data DATA           extra element to add, specify tag_integer:@filename_with_binary_data or tag_integer:hex_string
      --vendor-name VENDOR_NAME
                            (2) Vendor name
      --vendor-id VENDOR_ID
                            (2) Vendor ID
      --product-name PRODUCT_NAME
                            (2) Product name
      --product-id PRODUCT_ID
                            (2) Product ID
      --serial-num SERIAL_NUM
                            (2) Serial number (string)
      --manuf-date MANUF_DATE
                            (2) Manufacturing date
      --hw-ver HW_VER       (2) Hardware version
      --hw-ver-str HW_VER_STR
                            (2) Hardware version string
      --unique-id UNIQUE_ID
                            (2) Rotating unique ID
      --enable-key ENABLE_KEY
                            (2) Enable key (hex_string)
      --write-depfile-and-exit WRITE_DEPFILE_AND_EXIT
                            Write make depfile to disk and exit


### Arguments (configuration) file

When dealing with a particularly long argument list like this tool, it makes sense to keep the list of arguments in a (configuration) file rather than typing it out at the command line.
For this reason, arguments that start with an '@' character will be treated as a 'read this file'-statement, and @ arguments will be replaced by the argument list read from the file. For example:

    % cat factory_data.conf
     --discriminator=3840
     --passcode=20202021
    % python generate_factory_data.py --discriminator=1234 @factory_data.conf --out_file=data.bin

Arguments read from a file must by default be one per line and are treated as if they were in the same place as the original file referencing argument on the command line.
So in the example above, the resulting arguments used are "--discriminator=1234 --discriminator=3840 --passcode=20202021 --out\_file=data.bin"

## Example

This example will create a factory data file with two application specific elements, with id's 1 and 2. Two strings are used, the first will be 3 bytes, the second one 4.

    % echo -n 'one' >/tmp/two (will not add a newline)
    % echo 'two' >/tmp/two    (will add a newline)
    % python3 generate_factory_data.py --out_file /tmp/example.bin --data 1:@/tmp/one --data 2:@/tmp/two
    tag id:       0x1
    data length:  0x4
    data:         6f6e650a
    tag id:       0x2
    data length:  0x4
    data:         74776f0a
    tag id:       0x0
    data length:  0x0
    data:
    %

![Hexdump](Images/factory_block.png)

## Integration into development flow

The Makefile.FactoryData\_example will:

* Execute generate\_factory\_data.py to create the factory data binary file.
* call objcopy to turn the binary file into an .o object file with symbol \_factory\_data\_bin\_start
* call ar to provide the factory data as a .a library that can be linked in.
* use the generate\_factory\_data.py --write-depfile-and-exit feature to rebuild the library when input files change

Note that GN configuration or Makefile used to link the application must have a linker flag to avoid the linker optimizing this symbol away.

    -Wl,-u_binary_factory_data_bin_start
