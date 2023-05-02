# Credentials generator

## Purpose

This tool is intended for testing the full Device Attestation flow during development. It can generate all the needed
certificates to be used for testing. For more information about the Device Attestation process, please refer to
[Device attestation](../../Documents/Guides/device_attestation.md).

The output of this tool needs to be used as input for the factory block generator to make sure the Device Attestation
Certificate (DAC), Product Attestation Intermediate (PAI) and the Certification Declaration (CD) are getting
programmed on the device as part of the factory block. For more information about the factory block generation, please
refer to [Factory block generator](../FactoryData/README.md).

The credentials generator can generate:
* PAA certificate.
* PAA private/public key pair.
* PAI certificate signed with PAA.
* PAI private/public key pair.
* Multiple DAC certificates signed with PAI.
* Multiple private/public DAC keys.
* Certification Declaration.


## Implementation overview

This tool is implemented as a Python wrapper around the Matter&trade; certification tool (chip-cert). More details around this
Matter tool can be found [here](https://github.com/Qorvo/connectedhomeip/tree/v1.0.0.0_qorvo/src/tools/chip-cert).

## Usage


```
      python3 generate_certificate_data.py --help
      usage: generate_certificate_data.py [-h] [--chip-cert-tool-path CHIP_CERT_TOOL_PATH] [--paa-out-key PAA_OUT_KEY]
                                          [--paa-out-cert PAA_OUT_CERT] [--pai-out-key PAI_OUT_KEY]
                                          [--pai-out-cert PAI_OUT_CERT] [--dac-out-key DAC_OUT_KEY]
                                          [--dac-out-cert DAC_OUT_CERT] [--cd CD]
                                          [--sign-cd-priv-key SIGN_CD_PRIV_KEY]
                                          [--sign-cd-cert SIGN_CD_CERT] [--vid VID] [--pid PID] [--did DID]
                                          [--nmbr-dacs NMBR_DACS]

      Chip Certificate data generator tool

      optional arguments:
        -h, --help            show this help message and exit
        --chip-cert-tool-path CHIP_CERT_TOOL_PATH
                        path to the binary of chip-cert tool (example: <path>/<to>/QMatter/Tools/CredentialsGenerator)
        --paa-out-key PAA_OUT_KEY
                        filename for storing the PAA key (without extension) [default: qorvo_paa_key]
        --paa-out-cert PAA_OUT_CERT
                        filename for storing the PAA certificate (without extension) [default: qorvo_paa_cert]
        --pai-out-key PAI_OUT_KEY
                        filename for storing the PAI key (without extension) [default: qorvo_pai_key]
        --pai-out-cert PAI_OUT_CERT
                        filename for storing the PAI certificate (without extension) [default: qorvo_pai_cert]
        --dac-out-key DAC_OUT_KEY
                        filename for storing the DAC key (without extension) [default: qorvo_dac_key]
        --dac-out-cert DAC_OUT_CERT
                        filename for storing the DAC certificate (without extension) [default: qorvo_dac_cert]
        --cd CD
                        filename for storing the certification declaration (without extension) [default: qorvo_cd]
        --sign-cd-priv-key SIGN_CD_PRIV_KEY
                        path to the file containing private key to be used to sign the Certification Declaration
                        (example: <path>/<to>/QMatter/Tools/CredentialsGenerator/CertificationDeclaration/Chip-Test-
                        CD-Signing-Key.pem
        --sign-cd-cert SIGN_CD_CERT
                        path to the file containing the certificate to be used to sign the Certification Declaration
                        (example: <path>/<to>/QMatter/Tools/CredentialsGenerator/CertificationDeclaration/Chip-Test-
                        CD-Signing-Cert.pem
        --vid VID
                        vendor ID in hexadecimal form without 0x (example: --vid=FFF1)
        --pid PID
                        product ID in hexadecimal form without 0x (example: --pid=0123)
        --did DID
                        device ID in hexadecimal form without 0x (example: --pid=0016)
        --nmbr-dacs NMBR_DACS
                        number of DACs to be generated [default: 1]
```

## Example

Below example will generate 10 unique DAC certificates that can be programmed on the device. Next to the DAC
certificates also generation will be done of PAA certificate, PAA public/private key pair, PAI certificate, PAI
public/private key pair and certification declaration. This is all generated for a vendor ID 0x10D0
(Qorvo&reg; vendor ID), product ID 0x8005 and device ID 0x0016.


```
      python3 generate_certificate_data.py --chip-cert-tool-path=./
                                           --sign-cd-priv-key=./CertificationDeclaration/Chip-Test-CD-Signing-Key.pem
                                           --sign-cd-cert=./CertificationDeclaration/Chip-Test-CD-Signing-Cert.pem
                                           --vid=10D0 --pid=8005 --did=0016 --nmbr-dacs=10
```

Output will be individual files for PAA, PAI, DACs and CD:
* qorvo_paa_cert(.pem/.der)
* qorvo_paa_key(.pem/.der)
* qorvo_pai_cert(.pem/.der)
* qorvo_pai_key(.pem/.der)
* qorvo_dac_cert_x(.pem/.der)
* qorvo_dac_key_x(.pem/.der)
* qorvo_cd.bin

qorvo_dac_cert_x.der, qorvo_dac_key_x.der, qorvo_pai_cert.der and qorvo_cd.bin must be embedded into the device
by feeding it as input to the [factory data generator](../FactoryData/) tool.

qorvo_paa_cert.der must be provided to the Matter controller to validate the full device attestation flow during commissioning.
See [device attestation during commissioning](../../Documents/Guides/commissioning_posix_cli_chiptool.md#device-attestation).
