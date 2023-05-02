# Components

This folder contains the source code of the Qorvo&reg; SW stack (Base Components),
Matter&trade; and OpenThread glue code and third parties.
This source code is used to build the libraries that are used in the link step when building the sample applications.

Here below is a list of Components required for this Qorvo IoT Dev Kit for QPG6105

 - **Qorvo**: The Qorvo SW stacks, Matter and OpenThread glue code are placed here
    - [802_15_4](Qorvo/802_15_4): The MAC layer source code
    - [BSP](Qorvo/BSP): The Board Specific Package codes
    - [BaseUtils](Qorvo/BaseUtils): The basic utilities
    - [BleApplication](Qorvo/BleApplication): The Bluetooth&reg; LE Application layer designed by Qorvo
    - [BleController](Qorvo/BleController): The Bluetooth LE Controller layer
    - [Bootloader](Qorvo/Bootloader): Source code for the Bootloader application
    - [HAL_PLATFORM](Qorvo/HAL_PLATFORM): Hardware Abstraction Layer (HAL) for the Cortex-M4 Core
    - [HAL_RF](Qorvo/HAL_RF): Hardware Abstraction Later for the Radio Core
    - [Matter](Qorvo/Matter): Glue-layer for Matter source code
    - [OS](Qorvo/OS): The Operation System codes for the application to run on
    - [OpenThread](Qorvo/OpenThread): Glue-layer for OpenThread source code
    - [ROM](Qorvo/ROM): Information of the ROM code
    - [Rt](Qorvo/Rt): Necessary code for RT System
    - [Test](Qorvo/Test): Source code for test purposes

 - **ThirdParty**: All third-parties code necessary for this Qorvo IoT Dev Kit for QPG6105 are placed here
    - ARM
      - [cordio](ThirdParty/ARM/cordio-r20-05): The Bluetooth LE Cordio stack from ARM
      - [mbedtls](ThirdParty/ARM/mbedtls): The mbedtls crypto library from ARM
    - [Matter](ThirdParty/Matter): The Matter stack guided by CSA
    - [BLE Host Stack](ThirdParty/Pxxx_BLE_Host_Stack): Contains the BLE host stack
    - Silex
      - [cryptosoc](ThirdParty/Silex/cryptosoc): Contains the cryptosoc library for encryption
    - [CMSIS](ThirdParty/TOOL_CMSIS/CMSIS/Core/Include): Contains the CMSIS headers for Cortex-M4
