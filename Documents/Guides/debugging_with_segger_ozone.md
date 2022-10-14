# Debugging with Segger Ozone

In this guide we will explain how you can debug your application using Segger Ozone. Step by step instructions will be
given how to setup the debugging environment. The Matter light application will be used as example for the setup.

**Note** that it is currently not possible to program and debug QPG6105 with secure bootloader. When doing this with
Segger Ozone, the bootloader will reject the programmed application and will enter *panic mode*. This is because a
non-signed application is programmed on the chip instead of the signed one (.elf file does not contain valid signature).
To enable debugging, a non-secure bootloader needs to be used. Instructions to enable the non-secure bootloader variant
for debugging will be explained in this guide as well.

## Step 1: Configure non-secure bootloader in the project

### Step 1.1: Update Makefile to use non-secure bootloader
By default the secure bootloader will be used. This can be seen if you open the Makefile of the Matter light
application ([*Makefile.light_qpg6105*](../../Applications/Matter/light/Makefile.light_qpg6105)). At the bottom of the
file you can find the dependency make targets for the Matter light. One if the dependencies is the secure bootloader
build (*libBootloader_qpg6105_compr_secure.a*). To make sure the non-secure bootloader is used in the build tree, update
these lines as shown below:

```diff
+PRECIOUS: $(BASEDIR)/../../../Work/Bootloader_qpg6105/libBootloader_qpg6105.a
+.PHONY: $(BASEDIR)/../../../Work/Bootloader_qpg6105/libBootloader_qpg6105.a
+$(BASEDIR)/../../../Work/Bootloader_qpg6105/libBootloader_qpg6105.a:
+    $(MAKE) -f $(BASEDIR)/../../../Libraries/Qorvo/Bootloader/Makefile.Bootloader_qpg6105
-.PRECIOUS: $(BASEDIR)/../../../Work/Bootloader_qpg6105_compr_secure/libBootloader_qpg6105_compr_secure.a
-.PHONY: $(BASEDIR)/../../../Work/Bootloader_qpg6105_compr_secure/libBootloader_qpg6105_compr_secure.a
-$(BASEDIR)/../../../Work/Bootloader_qpg6105_compr_secure/libBootloader_qpg6105_compr_secure.a:
-    $(MAKE) -f $(BASEDIR)/../../../Libraries/Qorvo/Bootloader/Makefile.Bootloader_qpg6105_compr_secure
```

In the Makefile there are other references to the secure bootloader. Make sure to also update these paths to refer to
the non-secure bootloader:

```diff
+LIB_APP+=$(BASEDIR)/../../../Work/Bootloader_qpg6105/libBootloader_qpg6105.a
+LIB_APP+=$(BASEDIR)/../../../Work/Bootloader_qpg6105/libBootloader_qpg6105.a
+LIB_APP+=$(BASEDIR)/../../../Work/Bootloader_qpg6105/libBootloader_qpg6105.a
-LIB_APP+=$(BASEDIR)/../../../Work/Bootloader_qpg6105_compr_secure/libBootloader_qpg6105_compr_secure.a
-LIB_APP+=$(BASEDIR)/../../../Work/Bootloader_qpg6105_compr_secure/libBootloader_qpg6105_compr_secure.a
-LIB_APP+=$(BASEDIR)/../../../Work/Bootloader_qpg6105_compr_secure/libBootloader_qpg6105_compr_secure.a
```

```diff
COMPILER_SPECIFIC_LIB_RULE ?= yes
.PRECIOUS:default_target_makefile
.PHONY:default_target_makefile
default_target_makefile:  \
$(BASEDIR)/../../../Work/FactoryData_example_light/libFactoryData_example_light.a \
$(BASEDIR)/../../../Work/MatterQorvoGlue_qpg6105_libbuild/libMatterQorvoGlue_qpg6105_libbuild.a \
$(BASEDIR)/../../../Work/QorvoStack_qpg6105/libQorvoStack_qpg6105.a \
$(BASEDIR)/../../../Work/mbedtls_alt_qpg6105/libmbedtls_alt_qpg6105.a \
$(BASEDIR)/../../../Work/Matter_light_qpg6105/libMatter_light_qpg6105.a \
$(BASEDIR)/../../../Work/OpenThreadQorvoGlue_qpg6105_mtd/libOpenThreadQorvoGlue_qpg6105_mtd.a \
+$(BASEDIR)/../../../Work/Bootloader_qpg6105/libBootloader_qpg6105.a \
-$(BASEDIR)/../../../Work/Bootloader_qpg6105_compr_secure/libBootloader_qpg6105_compr_secure.a
app
```

```diff
PREREQ_HEADER_GENERATION_TARGETS =  \
$(BASEDIR)/../../../Work/FactoryData_example_light/libFactoryData_example_light.a \
$(BASEDIR)/../../../Work/MatterQorvoGlue_qpg6105_libbuild/libMatterQorvoGlue_qpg6105_libbuild.a \
$(BASEDIR)/../../../Work/QorvoStack_qpg6105/libQorvoStack_qpg6105.a \
$(BASEDIR)/../../../Work/mbedtls_alt_qpg6105/libmbedtls_alt_qpg6105.a \
$(BASEDIR)/../../../Work/Matter_light_qpg6105/libMatter_light_qpg6105.a \
$(BASEDIR)/../../../Work/OpenThreadQorvoGlue_qpg6105_mtd/libOpenThreadQorvoGlue_qpg6105_mtd.a \
+$(BASEDIR)/../../../Work/Bootloader_qpg6105/libBootloader_qpg6105.a
-$(BASEDIR)/../../../Work/Bootloader_qpg6105_compr_secure/libBootloader_qpg6105_compr_secure.a
```

Also updates are needed in the Makefiles of some library builds to disable secure boot:

#### [*Makefile.MatterQorvoGlue_qpg6105_libbuild*](../../Libraries/Qorvo/MatterQorvoGlue/Makefile.MatterQorvoGlue_qpg6105_libbuild)

```diff
# Application defines
APPNAME:=MatterQorvoGlue_qpg6105_libbuild
INC_APP:=
INC_APP+=-I$(BASEDIR)/../../../Applications/Matter/shared/config/inc
INC_APP+=-I$(BASEDIR)/../../../Components/Qorvo/HAL_PLATFORM/inc
INC_APP+=-I$(BASEDIR)/../../../Components/Qorvo/HAL_PLATFORM/inc/compiler/ARMGCCEMB
INC_APP+=-I$(BASEDIR)/../../../Libraries/Qorvo/MatterQorvoGlue/gen/MatterQorvoGlue_qpg6105_libbuild
INC+=$(INC_APP)
AINC_APP:=
AINC_APP+=-I$(BASEDIR)/../../../Applications/Matter/shared/config/inc
AINC_APP+=-I$(BASEDIR)/../../../Libraries/Qorvo/MatterQorvoGlue/gen/MatterQorvoGlue_qpg6105_libbuild
AINC+=$(AINC_APP)
LIB_APP:=
+LIB_APP+=$(BASEDIR)/../../../Work/Bootloader_qpg6105_compr_secure/libBootloader_qpg6105.a
-LIB_APP+=$(BASEDIR)/../../../Work/Bootloader_qpg6105_compr_secure/libBootloader_qpg6105_compr_secure.a
LIB+=$(LIB_APP)
```

#### [*Makefile.QorvoStack_qpg6105*](../../Libraries/Qorvo/QorvoStack/Makefile.QorvoStack_qpg6105)

```diff
# Application defines
APPNAME:=QorvoStack_qpg6105
INC_APP:=
INC_APP+=-I$(BASEDIR)/../../../Applications/Matter/shared/config/inc
INC_APP+=-I$(BASEDIR)/../../../Components/Qorvo/HAL_PLATFORM/inc
INC_APP+=-I$(BASEDIR)/../../../Components/Qorvo/HAL_PLATFORM/inc/compiler/ARMGCCEMB
INC_APP+=-I$(BASEDIR)/../../../Libraries/Qorvo/QorvoStack/gen/QorvoStack_qpg6105
INC+=$(INC_APP)
AINC_APP:=
AINC_APP+=-I$(BASEDIR)/../../../Applications/Matter/shared/config/inc
AINC_APP+=-I$(BASEDIR)/../../../Libraries/Qorvo/QorvoStack/gen/QorvoStack_qpg6105
AINC+=$(AINC_APP)
LIB_APP:=
+LIB_APP+=$(BASEDIR)/../../../Work/Bootloader_qpg6105_compr_secure/libBootloader_qpg6105.a
-LIB_APP+=$(BASEDIR)/../../../Work/Bootloader_qpg6105_compr_secure/libBootloader_qpg6105_compr_secure.a
LIB+=$(LIB_APP)
```

### Step 1.2: Update postbuild step to disable signing of the application
By default a postbuild script is called to sign the application hex and to generate an Over-The-Air upgrade file. This
is seen in the Makefile of the Matter light ([*Makefile.light_qpg6105*](../../Applications/Matter/light/Makefile.light_qpg6105)):

```
POSTBUILD_SCRIPT:=$(BASEDIR)/../../../Applications/Matter/light/light_qpg6105_postbuild.sh
```

Inside this postbuild script ([*light_qpg6105_postbuild.sh*](../../Applications/Matter/light/light_qpg6105_postbuild.sh))
we need to disable the signing step of the firmware by doing following changes:

```diff
+"$PYTHON" "${BASEDIR}"/../../../Tools/Ota/generate_ota_img.py --chip_config_header "${BASEDIR}"/../../../Applications/Matter/light/include/CHIPProjectConfig.h --chip_root "${BASEDIR}"/../../../Components/ThirdParty/Matter/repo --compression lzma --in_file "${BASEDIR}"/../../../Work/light_qpg6105/light_qpg6105.hex --out_file "${BASEDIR}"/../../../Work/light_qpg6105/light_qpg6105.ota
-$PYTHON" "${BASEDIR}"/../../../Tools/Ota/generate_ota_img.py --chip_config_header "${BASEDIR}"/../../../Applications/Matter/light/include/CHIPProjectConfig.h --chip_root "${BASEDIR}"/../../../Components/ThirdParty/Matter/repo --compression lzma --in_file "${BASEDIR}"/../../../Work/light_qpg6105/light_qpg6105.hex --out_file "${BASEDIR}"/../../../Work/light_qpg6105/light_qpg6105.ota --pem_file_path "${BASEDIR}"/../../../Tools/Ota/example_private_key.pem.example --pem_password test1234 --sign
```

With these changes you should be able to build a Matter light application that uses a non-secure bootloader. Building
of the application is done by following commands:

```
cd QMatter
source Scripts/activate.sh
cd Applications/Matter/light
make -f Makefile.light_qpg6105
```

Artifacts of this build will be generated in the *QMatter/Work/light_qpg6105* folder. *light_qpg6105.elf* will be used
as input for the Segger Ozone debugger. To set this up, continue to the next step.

## Step 2: Configuration of Segger Ozone project file
In this step you will set up the the Segger Ozone project file so the debugger can find the source code paths. A
template project file can be found here: [qmatter.jdebug](../../qmatter.jdebug)

In the project file you need to make sure the paths are set correctly by replacing ```<path>/<to>``` so it matches the
absolute path towards QMatter:

```
  Project.AddPathSubstitute ("<path>/<to>/QMatter", "$(ProjectDir)");
  Project.AddPathSubstitute ("<path>/<to>/qmatter", "$(ProjectDir)");
```

Dependent on the application you want to start debugging, make sure the File.Open function refers to the correct .elf
file (relative path). In this case we want to debug the Matter light application:

```
File.Open ("Work/light_qpg6105/light_qpg6105.elf");
```

Also make sure the project file (qmatter.jdebug) and QPG6105.svd files are kept in the root directory of QMatter as it
contains relatives paths.

## Step 3: Installing Segger Ozone
You can download Segger Ozone for linux using below link (Version 3.26e was used at the time of writing). Advise to
download .deb variant for easy installation on a Ubuntu distribution.

[https://www.segger.com/downloads/jlink/#Ozone](https://www.segger.com/downloads/jlink/#Ozone)

<div align="center">
  <img src="Images/segger_ozone_download.png" alt="Segger Ozone download">
</div>

Once it is downloaded on your linux PC you can go through the software installation process to get it installed on your
machine.

**_Additional requirement:_**
In order to properly use Ozone, please make sure you have the latest J-Link Software Package installed. Downloads for all platforms and installation instructions can be found on the [J-Link download page](https://www.segger.com/downloads/jlink/).


Once JLink and Ozone are installed successfully, you can proceed to the next step.

## Step 4: Starting a debug session in Segger Ozone
Now everything should be ready to setup a debug session. Open Segger Ozone using the application starter on your Linux
PC. This should bring up Segger Ozone with following dialog window:

<div align="center">
  <img src="Images/segger_ozone_starting_dialog.png" alt="Segger Ozone start">
</div>

Now click on ```Open Existing Project``` and select the updated qmatter.jdebug project file. If everything is set up
correctly, the debug environment should load correctly as seen in below picture:

<div align="center">
  <img src="Images/segger_ozone_debug.png" alt="Segger Ozone debug">
</div>

Make sure your QPG6105 development kit is plugged in via USB to your PC. Now you can trigger a download and reset to start your
debugging session. This can be done by clicking selecting *Download & Reset Program* as seen in below screenshot:

<div align="center">
  <img src="Images/segger_ozone_download_and_reset.png" alt="Segger Ozone download and reset">
</div>

You might see a pop-up that is indicating a license is missing. This is to make sure you only use this tool for
evaluation purposes. If so, you can click *Yes*

<div align="center">
  <img src="Images/segger_ozone_license_missing.png" alt="Segger Ozone license missing">
</div>

Next, It will start programming the chip, reset the chip and break in the main function:

<div align="center">
  <img src="Images/segger_ozone_main.png" alt="Segger Ozone break in main">
</div>

Now you have everything configured successfully to start your debug session.
