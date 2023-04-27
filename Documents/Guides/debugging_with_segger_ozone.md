# Debugging with Segger Ozone

In this guide we will explain how you can debug your Matter&trade; application using Segger Ozone. Step by step instructions will be
given how to setup the debugging environment. The light application will be used as example for the setup.

**Note** that it is currently not possible to program and debug QPG6105 with secure bootloader. When doing this with
Segger Ozone, the bootloader will reject the programmed application and will enter *panic mode*. This is because a
non-signed application is programmed on the chip instead of the signed one (.elf file does not contain valid signature).
To enable debugging, a non-secure bootloader needs to be used. Instructions to enable the non-secure bootloader variant
for debugging will be explained in this guide as well.

## Step 1: Enable debugging capabilities for a specific application

In this step, the AppConfigurator Tool will be used to perform this action.
As an example reference app, the "lock" application will be used.
The lock application is by default a sleep end device(SED) and is using secure bootloader.
As such the lock application requires the most modifications to be capable of using a debugger.

The following example command will perform this action and recompile the application makefiles afterwards.
```
(.python_venv) QMatter/Tools/AppConfigurator$ python app_configurator.py --update-strategy="update" --ref-app-name="lock" --debugging="enable"
```

This command will :
- Disable sleep support.
- Modify secure bootloader to non-secure bootloader.
- Disable the signing of the firmware in the postbuild steps
- Modify the Segger Ozone project file to point to this application


## Step 2: Installing Segger Ozone
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

## Step 3: Starting a debug session in Segger Ozone
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
