# Device attestation (DA) in the Matter&trade; protocol

The purpose of the Matter device attestation process is to verify during commissioning that the device being commissioned is Matter procotol certified. It is also used to securely obtain certain information about the device (vendor identifier, product identifer, ...). This device attestation process is mandatory for all Matter devices and makes sure no counterfeit devices can join the Matter network.

In this guide we dive deeper into the Matter device attestation feature and explain what it does and how it is used. Also
the needed pointers are given to get started with the device attestation feature during your development cycle.


## Terminology

* **Vendor Identifier (VID)**: An unique identifier of the vendor that is producing the device. This identifier is
assigned by the Connectivity Standards Alliance (CSA).
* **Product Identifier (PID)**: An unique identifier to identify the product. This identifier is assigned by the vendor.
* **Public Key Infrastructure (PKI)**: A set of entities that are linked as a chain of trust in which the authenticity
of one enity is validated by another entity by using the associated private key. At the top of the certificate chain is
always a certificate authority (CA). The PKI in the Matter stack is defined by a set of certificates together with its
corresponding public/private key pair.
* **Certificate Authority (CA)**: Entity that stores, signs and issues certificates.
* **Distributed Compliance Ledger (DCL)**: Database that is maintained by the CSA. It maintains a trusted root CA store
and stores information about the certification status of different vendors and products.
* **Product Attestation Authority (PAA)**: The root CA. PAAs are part of the DCL. PAA can be shared between different
vendors or a vendor can set up his own PAA to serve its own products only.
* **Product Attestation Intermediate (PAI)**: Intermediate certificate for a specific vendor that is issued and signed
by a PAA. Vendor identifier is present in the PAI. For this certificate also a product identifier can be present but
is optional. This means that a vendor can create one PAI for all its products, or create one PAI per product.
* **Device Attestation Certificates (DAC)**: Unique certificate per device that gets issued and signed by the PAI. Both
vendor identifier and product identifier are mandatory for this certificate. Vendor identifier needs to match the vendor
identifier of the PAI.
* **Certification Declaration (CD)**: Contains information that is needed for the device attestation process (VID, PID,
certificate identifier, certification type). This is a set of information that is issue by the CSA, that proves the
device's compliance with the Matter protocol. This is created by the CSA when the certification process finishes.
* **Distinguished Encoding Rules (DER)**: Binary encoding for X.509 certificates and private keys.
* **Privacy Enhanced Mail (PEM)**: Common format for X.509 certificates and cryptographic keys.


## Device Attestation PKI

Every commissionable Matter node needs to have a unique DAC and corresponding private key. The DAC is a DER-encoded
X.509v3-compliant certificate as defined in [RFC 5280](https://www.rfc-editor.org/rfc/rfc5280) and shall be issued by a
PAI that chains directly to an approved PAA.

The DAC contains a signature which is validated with the PAI public key. At the root of the chain of trust, the PAA
public key validates signatures of the PAI. Hierarchy of the Matter device attestation PKI can be found
in below picture.

<div align="center">
  <img src="Images/pki_hierarchy.png" alt="Matter device attestation PKI">
</div>

## Device Attestation data

The DAC is signed by the PAI certificate. The PAI is based on a vendor identifier which is typically held by the device
manufacturer. The PAI certificate is signed by the PAA certificate which acts as the root CA. The list of trusted PAA
certificates are stored in the DCL.

During manufacturing the following elements need to be stored in the persistent storage space of the device for completing
the device attestation process (see also picture below):
* PAI certificate
* DAC certificate
* DAC private key
* CD

All this information will be fetched and used during the commissioning process by the Matter controller. The Matter controller will verify them together with the vendor and product identifier. This process makes sure the commissioned Matter device is a trusted device.

<div align="center">
  <img src="Images/persistent_storage_dac.png" alt="Matter protocol certificate storage" width="500">
</div>


## Device Attestation procedure

During the device attestation procedure, the commissioner (Matter controller) is responsible for attesting the commissionee (Matter device). Below picture illustrates the device attestation steps that are taken during commissioning.

**Step 1**: The commissioner will generate a random 32 byte attestation nonce.

**Step 2**: The commissioner will send the attestation nonce to the commissionee and requests attestation information
using the command *Attestation Request*.

**Step 3**: The commissionee will generate the attestation information and sign the information with the DAC private
key. Data that is part of the attestation information is:
* Certification Declaration.
* Attestation Nonce.
* Timestamp.
* Firmware information.

**Step 4**: The commissionee will send the signed attestation information to the commissionee using the command
*Attestation Response*.

**Step 5**: The commissioner reads the certificate chain and requests the DAC and PAI certificates using the command
*Certificate Chain Request*. The commissionee sends back the PAI and DAC certificates using the command
*Certificate Chain Response*.

**Step 6**: The commissioner validates the attestation information. Validation steps that are taken are:
* Validation of DAC certificate chain. Making sure the PAA is validated for presence in the commissioner's trusted root
store.
* VID on the DAC matches the VID on the PAI.
* VID and PID matches the attribute values found in the Basic Information Cluster.
* Attestation signature is valid.
* Attestation nonce matches the attestation nonce generated by the commissioner.
* Certificate Declaration signature is valid. Should be signed by one of the CSA's CD signing keys.
* Firmware information matches the information present in the DCL.

<div align="center">
  <img src="Images/attestation_steps.png" alt="Overview of the Matter device attestation steps" width="500">
</div>


## Using device attestation during development

Qorvo Matter SDK provides all the needed tools to get familiar with the device attestation feature of Matter. The Qorvo Matter reference applications come with Qorvo development certificates and keys to get started out of the box. This is explained in the section [Default configuration](#default-configuration). In the section
[Generation of certificate data](#generation-of-certificate-data) you will find the information how to generate your own
PAA, PAI, DACs and CDs. Finally, in the section [Generation of the factory block](#generation-of-the-factory-block),
information will be given how to program the generated PAI, DAC, DAC keys and CD on your device to be able to go through
the device attestation procedure during commissioning.


### Default configuration

In the initialization start-up procedure the device attestation credentials provider is selected:

```
    SetDeviceAttestationCredentialsProvider(&mFactoryDataProvider);
```

By default the vendor identifier in the reference applications for light and lock is set to the vendor identifier
0xFFF1. For the base application the Qorvo vendor identifier (0x10D0) is used and the corresponding Qorvo certificates.
The factory data provider implementation makes sure that all the device attestation credentials will be read from a
factory block that contains the needed information for device attestation. This factory block is automatically generated
in the build process and will be automatically linked after building.

For the base applications, following default elements are used during the generation of the factory block:
- DAC: [qorvo_dac_cert_0.der](../../Tools/FactoryData/Credentials/qorvo_dac_cert_0.der)
- DAC private key: [qorvo_dac_key_0.der](../../Tools/FactoryData/Credentials/qorvo_dac_key_0.der)
- PAI: [qorvo_pai_cert.der](../../Tools/FactoryData/Credentials/qorvo_pai_cert.der)
- CD: [qorvo_cd.bin](../../Tools/FactoryData/Credentials/qorvo_cd.bin)


### Generation of certificate data

If you want to generate your own certificates to use with the device attestation feature, Qorvo created a Python tool
that allows you to do that. The tool is called *generate_certificate_data.py* and more information how to use it can be
found [here](../../Tools/CredentialsGenerator).

With this tool it is possible to generate a PAA certificate with corresponding public/private key. The tool uses the
generated PAA certificate to generate and sign a PAI certificate for a specific vendor identifier. Additionally, the tool can
generate multiple unique DACS for a specific product identifier that are getting signed with the PAI. Finally a
certification declaration is also generated. The certificates and corresponding private/public key pairs will be
generated as separate files and can be used as input for the factory block generator to be able to store these on the
device.


### Generation of the factory block

To be able to use your own generated certificates during device attestation, Qorvo created a Python tool that allows you
to generate a factory block that contains all the needed factory data to be able to do device attestation. In the
end, the factory block gets linked in and is part of the persistent storage of the Matter device once it is programmed.

More information on the usage of the factory block and how it is integrated in Make can be found
[here](../../Tools/FactoryData)


## Provisioning device certificates in a production environment

More information on the steps to take to get all the needed data programmed on your production device will be added
later.
