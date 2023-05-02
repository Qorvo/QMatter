# Tools

This folder contains all the needed tools to make Matter development and testing easy with Qorvo&reg; IoT Dev Kit for QPG6105. Tools that are
provided are:
* [AppConfigurator](AppConfigurator): A tool to customize the Matter reference applications.
* [AppCreator](AppCreator): A tool that generates the complete framework to start the development of any custom Matter
application.
* [CertificateServer](CertificateServer): An example service for device enrollment with Qorvo Program Utility, it
  demonstrates factory block and certificate generation in a factory programming flow context.
* [CredentialsGenerator](CredentialsGenerator): A tool that can generate test certificates to get familiar and integrate
the device attestation feature in any Matter application.
* [CrystalProcurement](CrystalProcurement): Tool that can be used to check the crystal specification that can meet our requirement.
* [FactoryData](FactoryData): A tool to generate a factory block that can be linked in at build time. This factory block
holds the certificates needed for device attestation and also information as discriminator, passcode, etc.
* [Jadelogger](Jadelogger): A tool to capture serial data with Qorvo proprietary protocol.
* [MatterControllers](MatterControllers): Tool that can be used for provisioning and interacting with a Matter node
in a Matter network.
* [MatterOtaProviders](MatterOtaProviders): Tool that can be used to add a Matter OTA provider node in the Matter network.
* [MemoryOverview](MemoryOverview): Tool that can be used to calculate the Flash and RAM usage after builds
* [Ota](Ota): OTA Firmware Upgrade Image/File Creation Tooling.
* [PTC](PTC): Tool needed to do RF testing of QPG6105 products that run the PTC firmware.
* [SecureBoot](SecureBoot): All needed tools for signing of the User Mode Bootloader.
* [Zap](Zap): Tool used to define the clusters for a custom Matter application and generation of the corresponding source
code to interface with the selected clusters.
