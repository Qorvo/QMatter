# Secure boot tools

* [secureboot_sign_hex.py](secureboot_sign_hex.py): A tool that signs the User mode Bootloader with a private key. The
  matching public key is programmed into the device info page via [generate_programmer.py](generate_programmer.py) and
  used by the ROM bootloader to authenticate the User mode bootloader.
* [generate_programmer.py](generate_programmer.py): A tool that generates an XML configuration file for the Qorvo
  Programming utility.
* [generateRMAToken.py](generateRMAToken.py): A tool to create [a RMA token file for customer returns](../../Documents/Application%20Notes/Bootloader/SW30239_AN_Locking_And_Product_Life_Cycle.pdf).
* [private_key.pem](private_key.pem): an example private key to provide a complete build.
* [public_key.pem](public_key.pem): an example private key to provide a complete build.
* [setRollbackCounter.py](setRollbackCounter.py): Tool to initialize Ota rollback counter (used by peripheral examples)
* [signFirmware.py](signFirmware.py): Firmare signing tool (used by peripheral examples)
* [crypto_utils.py](crypto_utils.py): common functions used across tools
