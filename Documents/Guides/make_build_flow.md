# The QMatter build flow

## Make based

The applications in the QMatter SDK are built using GNU MAKE, a well known
build system.  The applications are built on top of component libraries like
protocol stacks and glue layers that form a set of .o library files (.a).

To link in the component libraries and protocol stacks, the top level application makefiles will
include a make rule (for each library) to call the specific component library makefile.
These 'PHONY' rules will trigger each time the build is triggered, so the component library makefile can
check if the .a file needs to be updated.


## Makefile dependency tree
For a given application, its makefiles can call makefiles, forming a dependency tree, for example:

* Applications/Matter/light/Makefile.light\_qpg6105
  * Libraries/ThirdParty/Matter/Makefile.Matter\_light\_qpg6105
  * Libraries/Qorvo/QorvoStack/Makefile.QorvoStack\_qpg6105
  * Libraries/Qorvo/MatterQorvoGlue/Makefile.MatterQorvoGlue\_qpg6105\_libbuild
  * Libraries/Qorvo/Bootloader/Makefile.Bootloader\_qpg6105\_compr\_secure
    * Applications/Bootloader/Makefile.UMB\_QPG6105DK\_K01\_compr\_secure\_armgccemb
  * Libraries/Qorvo/mbedtls\_alt/Makefile.mbedtls\_alt\_qpg6105
  * Libraries/Qorvo/OpenThreadQorvoGlue/Makefile.OpenThreadQorvoGlue\_qpg6105\_mtd
  * Libraries/Qorvo/FactoryData/Makefile.FactoryData\_example

## The Matter stack

To build the Matter stack, the [Matter\_light\_qpg6105 makefile](../../Libraries/ThirdParty/Matter/Makefile.Matter_light_qpg6105) will
run `gn` and `ninja` to build `libMatter_light_qpg6105.a`.

The matter codebase has a specific qpg/chip-gn configuration at `Components/ThirdParty/Matter/repo/config/qpg/chip-gn`
that lists what code is included in this library.

## The QPG6105 bootloader

The QPG6105 bootloader needs to be included in the main factory firmware image to enable the OTA firmware upgrade process.
The bootloader is linked into the main firmware image as binary blobs that are placed at the correct flash addresses.

The bootloader is compiled into a hexfile using [the bootloader application makefile in Applications/Bootloader](../../Applications/Bootloader/Makefile.UMB_QPG6105DK_K01_compr_secure_armgccemb).
To easily embed the bootloader in the main factory firmware image, it is converted to a libBootloader.a objectcode library by
the [bootloader library makefile in Libraries/Qorvo/Bootloader](../../Libraries/Qorvo/Bootloader/Makefile.Bootloader_qpg6105_compr_secure)

## Compiler invocation and compiler flags

The calling of the compiler and the compiler flags used is split to the following files under make/:

* [make/gpcommon.mk](../../make/gpcommon.mk): make compilation rules (.o/.elf/.map/.lss)
* [make/compilers/armgccemb/compiler\_rules.mk](../../make/compilers/armgccemb/compiler_rules.mk) make compilation rules (.hex)
* [make/compilers/armgccemb/compiler\_defines.mk](../../make/compilers/armgccemb/compiler_defines.mk): compiler flags

Additionally, the programmer\_rules.mk file implements a 'program' rule that can be used to copy the hexfile to a drag
and drop USB programming device:

* [make/programmers/cmsisdap/programmer\_rules.mk](../../make/programmers/cmsisdap/programmer_rules.mk)
