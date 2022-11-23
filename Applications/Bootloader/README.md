# QMatter bootloader

This directory contains the application level sourcecode of the QMatter bootloader. This document

- provides references to in-depth documentation on Secure Boot in the Qorvo&reg; QPG6xxxx products
- explains the firmware upgrade process in the QMatter application
- points to the specific tools included in this SDK.

# Application Note SW30236

As a general introduction and in-depth dive into secure boot, Qorvo provides Application Note SW30236:

[Volume 1: Security And Secure User Mode Bootloader](../../Documents/Application%20Notes/Bootloader/SW30236_AN_Vol_1_Security_And_Secure_User_Mode_Bootloader.pdf)
of this Application Note covers security of modern IoT devices and shows how Qorvo hardware and software
security solutions protect devices from attackers by keeping the chain of trust between various entities
intact. The document briefly explains security attacks, methods to prevent software tampering and
general security considerations. The main security aspect covered in this document is about Secure
Boot. It starts by first introducing the concept of secure boot followed by an overview of Qorvo Secure
Boot solution.


[Volume 2: Secure User Mode Bootloader Implementation](../../Documents/Application%20Notes/Bootloader/SW30236_AN_Vol_2_Secure_User_Mode_Bootloader_Implementation.pdf)
of this Application Note describes the implementation of secure boot as part of the User Mode Bootloader, and the interface between application and bootloader.


[Volume 3: Secure User Mode Bootloader Update Image Generation](../../Documents/Application%20Notes/Bootloader/SW30236_AN_Vol_3_Secure_User_Mode_Bootloader_Update_Image_Gen.pdf)
of this Application Note provides practical information on generating signed application images for Over-the-Air (OTA) updates.

# Application Note SW30239 on Locking and Product Life Cycle

[Application Note SW30239 Locking and Product Life Cycle](../../Documents/Application%20Notes/Bootloader/SW30239_AN_Locking_And_Product_Life_Cycle.pdf)
explains the hardware and software protections that are in place to lock down read from,
write to and erasure of flash memory as well as limit access to interfaces that provide ways to alter
the executed code and access buses and registers on the cpu.


# User mode Bootloader

[Application Note SW95734 User Mode Bootloader](../../Documents/Application%20Notes/Bootloader/SW95734_AN_User_Mode_Bootloader.pdf)
explains what the User Mode Bootloader does and defines the technical terms we use to describe the
characteristics of the user mode bootloader included in the QMatter SDK:

- It uses License Based Boot
- It uses compression
- No flash remapping is used
- The image is authenticated by verifying the cryptographic signature embedded in the upgrade image.

For debugging purposes, the SDK also includes a non-secure build configuration,
[the debugging guide](../../Documents/Guides/debugging_with_segger_ozone.md) describes how to use
that configuration in your application for debugging purposes.

To dive into the sourcecode of the User mode bootloader you can read the Makefile to see
what sourcecode is built.

# The upgrade process
## 1. OTA Image generation

The [Ota tooling documentation](../../Tools/Ota/README.md) describes how to generate an OTA image.

## 2. Image download and upgrade request

- The Application uses `qvCHIP_OtaWriteChunk()` to write the OTA image to the flash.  This function will write the uncompressed JTOTA section and compressed OTA section (1).
- The application registers intent to update by calling `qvCHIP_OtaSetPendingImage()`. This function will copy the Loaded user license update image to the Upgrade Image User License (2)
- The Loaded User License Update Image freshness counter is updated and the `LOADED_USER_LICENSE_LOAD_COMPLETED_MAGIC_WORD` is set (3)
- The application calls `qvCHIP_OtaReset()` which triggers `SOFT_POR_BOOTLOADER()`, transfering execution to the bootloader by triggering a software initiated power-on-reset.

<div align="center">
  <img src="Images/upgrade-part1-application.png" alt="Qorvo Matter">
</div>

## 3. Update activation

- When the bootloader starts and there is a pending upgrade (1)
  - it will verify the upgrade image user license
  - check the cryptographical signature
  - check the LZMA image header is valid
- When the bootloader decides to proceed with the upgrade, it wil copy over the uncompressed jumptables section (2).
- It will inflate (decompress) the LZMA payload and write it to the Application section on the fly (3).
- It will set the Loaded User License’s load complete magic word.
- As on each startup, the bootloader will jump to application.

<div align="center">
  <img src="Images/upgrade-part2-bootloader.png" alt="Qorvo Matter">
</div>
