# Libraries

This folder contains makefiles for [Qorvo libraries](Qorvo) and [ThirdParty libraries](ThirdParty) for use with the
QMatter SDK.

Qorvo libraries are:
* Bootloader library: This is the secure bootloader that gets built as a library to link it in the full Matter application.
* FactoryData library: This is a factory block that gets generated and gets linked in the full Matter application.
* Matter-Qorvo glue library: Matter-Qorvo glue code library.
* OpenThread-Qorvo glue library: OpenThread-Qorvo glue code library.
* Qorvo Stack library: Library that contains all lower level Qorvo Software components (HAL, BSP, NVM, etc.)
* mbedtls alternative library: Library that implements the mbedtls API on top of the Qorvo crypto hardware block.

ThirdParty libraries are:
* FreeRTOS library: A library of FreeRTOS being used by the QPG6105 peripheral applications.

Please read the [QMatter build flow with make](../Documents/Guides/make_build_flow.md) guide for more details.
