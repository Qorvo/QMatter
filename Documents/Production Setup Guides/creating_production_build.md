
# Creating production build for the factory

To mass produce your product, you will need to flash each device at the production line and provision it with Device
Attestation data.

Qorvo provides the Qorvo Programming Utility (to be used with the USBPOD programmer) to enable flashing and provisioning
the device with a private device key in a secure way.

Not only different programming tools will be used, vendor keypairs will have to be generated to enable the device to
perform SecureBoot chain from ROM and to ensure only your company can sign firmware updates.

This guide describes all the steps necessary and modifications to make.

Note this this document already starts from a release build with updated assert handling and disabled verbose logging.
More details on this subject can be found in the guide [Creating a release build](./creating_release_build.md).

To summarize:

- You will lock the ROM programmer bulk erase and debug functionality.
- You will create a keypair to allow the ROM bootloader to verify the UserMode bootloader.
- You will create a keypair to sign the initial firmware application and later versions provided via OTA updates.
- You will create your vendor-specific implementation of the enrollment service which generates
the device private key and factory block and provides this to the Programming Utility.


Use the production bootloader
=============================

1.Add the line `#define GP_DIVERSITY_SECURE_BOOTLOADER_PRODUCTION` to
`Applications/Bootloader/gen/UMB_qpg6105_compr_secure_sbrom_asym_key/qorvo_internals.h`.

2.Try building with make, this should trigger
```
#error BULK ERASE AND DEBUG LOCK ENABLED - Remove this error if you are sure that you want to lock the device
```
in `Applications/Bootloader/src/native_user_license/P334_QPG6105.c`.

3. Remove the #error statement from the file.

> **WARNING:** Please be aware that after doing this modification, devices flashed with the production build can not be
> erased except by using the RMA flow.
>


Unique Stage 1 SecureBoot keypair
=================================

Generating and dumping secp256r1 private/public key pairs for use with asymmetric secure boot.

This will be programmed into the device infopage and used to sign the UMBL

Generate private key and AES encrypt it into a PEM file format, you can store these
files in a safe location of your choice. Create the folder and 'cd' to it:

```
mkdir keys
cd keys
```

Create a private key file for secure boot:
```
openssl ecparam -name prime256v1 -genkey | openssl ec -out secureboot_private_key.pem -aes256
read EC key
writing EC key
Enter pass phrase for PEM:
Verifying - Enter pass phrase for PEM:
```

> WARNING: It is very important to apply security best practices around these key pairs.
> Never store the passphrase for the key pairs online or together with the PEM file.
> If the keys are leaked the Certificate Authority will revoke your certificates, which
> can lead to devices in the field stop functioning.

Your private key is now protected by a pass phrase, we will store the matching public key
in separate files so we can access it without requiring the passphrase.

First we will store a binary dump of it to later verify the programmer XML contains the
correct public key:
```
openssl ec -in secureboot_private_key.pem -pubout -outform DER -out secureboot_public_key.der
read EC key
Enter pass phrase for secureboot_private_key.pem:
writing EC key
```
And create a hexadecimal dump:
```
xxd -s +27 -c 16 -g 16  secureboot_public_key.der  > secureboot_public_key.txt
```

We also need the public key in PEM format to be read by `generate_programmer.py`:
```
openssl ec -in secureboot_private_key.pem -pubout -outform PEM -out secureboot_public_key.pem
read EC key
Enter pass phrase for secureboot_private_key.pem:
writing EC key
```

Update the postbuild scripts path references to point to the newly generated files:

1. In `Applications/Bootloader/UMB_qpg6105_compr_secure_sbrom_asym_key_postbuild.sh` edit the arguments for
   `secureboot_sign_hex.py`:
- change the path for `--asymkey "${SCRIPT_DIR}"/../..//Tools/SecureBoot/private_key.pem`
- remove `--pem_password placeholdersecurebootpassword`

2. In `Applications/Matter/base/base_qpg6105_production_postbuild.sh` edit the arguments for `generate_programmer.py`:
- change the path for `--user-public-key-path "${SCRIPT_DIR}"/../../..//Tools/SecureBoot/public_key.pem` to point to
  `secureboot_public_key.pem`

Build the production makefile. You will be prompted to enter the PEM passphrases.


Verify that the public key from `secureboot_public_key.txt` is
present in the generated XML file.  The XML file should contain the key split over user keys, each 16-byte key should be reversed
compared to the hexdump. For example, the first user key value would be:

```
<User_Key_index>
    0
</User_Key_index>
<User_Key_value>
    0x05538dacddb4752c51522d594f2c0621
</User_Key_value>
```
And the first part of the hexdump the reverse:
```
0000001b: 21062c4f592d52512c75b4ddac8d5305  !.,OY-RQ,u....S.
```

Unique Stage 2 application keypair
==================================

Generate a keypair that is used to sign the application images and OTA payload images.
The keypair's public key will be embedded into the User mode bootloader's
userlicense, and is flashed to the device during the production flashing.

```
openssl ecparam -name prime256v1 -genkey | openssl ec -out application_private_key.pem -aes256
read EC key
writing EC key
Enter pass phrase for PEM:
Verifying - Enter pass phrase for PEM:
```

1. In `Applications/Bootloader/UMB_qpg6105_compr_secure_sbrom_asym_key_postbuild.sh` edit the arguments for `secureboot_sign_hex.py`:
- `--application_pem "${SCRIPT_DIR}"/../../Tools/Ota/example_private_key.pem.example`
- remove `--application_pem_password test1234`

2. In `Applications/Bootloader/UMB_qpg6105_compr_secure_sbrom_asym_key_postbuild.sh` edit the arguments for `generateRMAToken.py`:
- Since this line is an example only applicable to the RMA flow, disable the line by putting a '#' at the start of the line
- update the path `--pem "${SCRIPT_DIR}"/../../Tools/Ota/example_private_key.pem.example`
- remove `--pem_password test1234`

3. In your application's `production_postbuild.sh` script (eg `Applications/Matter/base/base_qpg6105_production_postbuild.sh`) edit the arguments for `generate_ota_img.py`:
- `--pem_file_path "${SCRIPT_DIR}"/../../../Tools/Ota/example_private_key.pem.example`
- remove `--pem_password test1234`
