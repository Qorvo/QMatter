# Tools

This folder contains all the needed tools to make Matter development and testing easy with Qorvo QPG6105. Tools that are
provided are:
* [AppCreator](AppCreator): A tool that generates the complete framework to start the development of any custom Matter
application.
* [CredentialsGenerator](CredentialsGenerator): A tool that can generate test certificates to get familiar and integrate
the device attestation feature in any Matter application.
* [FactoryData](FactoryData): A tool to generate a factory block that can be linked in at build time. This factory block
holds the certificates needed for device attestation and also information as discriminator, passcode, etc.
* [Jadelogger](Jadelogger): A tool to capture serial data with Qorvo proprietary protocol.
* [MatterControllers](MatterControllers): Tool that can be used for provisioning and interacting with a Matter node
in a Matter network.
* [MatterOtaProviders](MatterOtaProviders): Tool that can be used to add a Matter OTA provider node in the Matter
network.
* [Ota](OTA): OTA Firmware Upgrade Image/File Creation Tooling.
* [PTC](PTC): Tool needed to do RF testing of QPG6105 products that run the PTC firmware.
* [Zap](Zap): Tool used to define the clusters for a custom Matter application and generation of the corresponding source
code to interface with the selected clusters.

