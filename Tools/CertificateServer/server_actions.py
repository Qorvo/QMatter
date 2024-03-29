"""Example HTTP server to provide certificate signing services through an HTTP API.
Server actions executed by each HTTP call.
"""
import os
import abc
from typing import Dict, List, Tuple
import logging
import datetime
from pathlib import Path

from cryptography import x509
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.asymmetric.types import CERTIFICATE_PRIVATE_KEY_TYPES


from . import api_objects
from . matter_certificates import create_matter_certificate, DACProperties
try:
    from . assets.QMatter.generate_factory_data import (
        generate_factory_bin,
        parse_command_line_arguments,
        TOOLS as qmatter_tools,
    )
except ModuleNotFoundError:
    from FactoryData.generate_factory_data import (
        generate_factory_bin,
        parse_command_line_arguments,
        TOOLS as qmatter_tools,
    )

# ##############################################################################
# Declare global values to be used when generating factory data
# ##############################################################################
supported_app_ids = ["PROJECT_X"]


# ##############################################################################
# Declare server actions for API calls
# ##############################################################################
class IsAlive(api_objects.IsAlive):
    def process(self):
        """Execute server-side actions based on the request data, update the request data and return. For `IsAlive`,
        return a custom, human-readable, string.
        """
        self.response("A wonderful life is a life full of wonders.")


class CheckServerInfo(api_objects.CheckServerInfo):
    def process(self):
        """Execute server-side actions based on the request data, update the request data and return.
        """
        server_version = api_objects.API_VERSION
        # Check major and minor version for API compatibility
        self.response(server_version, server_version[:2] == self.client_version[:2])


class _GenerateFactoryData(api_objects.GenerateFactoryData):
    @classmethod
    @abc.abstractmethod
    def generate_factory_data(
        cls,
        target_public_key: bytes,
        target_802_15_4_address: str,
        target_ble_address: str,
        mandatory_fields: Dict[str, str],
    ) -> Tuple[bytes, List[api_objects.GenerateFactoryData.KeyType]]:
        ...

    def process(self):
        """Execute server-side actions based on the request data, update the request data and return.
        """
        app_id = self.matching_fields.get("AppID", None)
        if app_id not in supported_app_ids:
            if app_id is None:
                logging.error("Request does not provide an APPID value.")
            else:
                logging.error(f"Rejecting unsupported AppID: {app_id}")
                self.response(bytes(), [], False)
                return

        if not self.in_session_private_key_generation:
            logging.warn("Private key not generated by target in the current session!")

        factory_data, keys = self.generate_factory_data(
            self.target_public_key,
            self.target_802_15_4_address,
            self.target_ble_address,
            self.mandatory_fields,
        )

        if bool(factory_data):
            self.response(factory_data, keys, True)
        else:
            self.response(bytes(), [], False)


class GenerateFactoryDataWithPreGeneratedData(_GenerateFactoryData):
    dac_cert: Path
    dac_key: Path
    dac_key_list: List[api_objects.GenerateFactoryData.KeyType]
    pai_cert: Path
    mft_cd: Path
    cfg_file: Path

    @classmethod
    def InitStaticData(
        cls,
        dac_cert: Path,
        dac_key_der: Path,
        dac_key_list: List[api_objects.GenerateFactoryData.KeyType],
        pai_cert: Path,
        mft_cd: Path,
        cfg_file: Path
    ) -> None:
        cls.dac_cert = dac_cert
        cls.dac_key = dac_key_der
        cls.dac_key_list = dac_key_list
        cls.pai_cert = pai_cert
        cls.mft_cd = mft_cd
        cls.cfg_file = cfg_file
        spake2p_path = str(Path.cwd().joinpath("assets", "QMatter", "spake2p"))
        if not os.path.exists(spake2p_path):
            spake2p_path = "/usr/bin/spake2p"
        assert os.path.exists(spake2p_path)
        qmatter_tools['spake2p'] = spake2p_path

    @classmethod
    def generate_factory_data(
        cls,
        target_public_key: bytes,
        target_802_15_4_address: str,
        target_ble_address: str,
        mandatory_fields: Dict[str, str],
    ) -> Tuple[bytes, List[api_objects.GenerateFactoryData.KeyType]]:
        cli_args = [
            f"--dac-cert={cls.dac_cert}",
            f"--dac-key={cls.dac_key}",
            f"--pai-cert={cls.pai_cert}",
            f"--certification-declaration={cls.mft_cd}",
            # change this to get a random value
            "--unique-id=eac7be98380ad71fd9c3bcbe4531a4f2"
        ]
        args = parse_command_line_arguments(cli_args + cls.cfg_file.read_text().splitlines())
        data = generate_factory_bin(args)
        return (data, cls.dac_key_list)


class GenerateFactoryDataWithTargetGeneratedKey(_GenerateFactoryData):
    pai_private_key: CERTIFICATE_PRIVATE_KEY_TYPES
    pai_certificate_bytes: bytes
    pai_certificate_path: Path
    device_serial_number: int
    output_path: Path
    cfg_file: Path
    mft_cd: Path

    @classmethod
    def InitStaticData(
        cls,
        pai_private_key_file: Path,
        pai_certificate_file: Path,
        serial_number: int,
        output_path: Path,
        cfg_file: Path,
        mft_cd: Path,
    ) -> None:
        """Initialize the common request data."""

        # Read private key
        keytypes = ('.pem', '.key')
        if pai_private_key_file.suffix not in keytypes:
            raise ValueError(f"PAI Private Key must be in {keytypes} filetype.")
        cls.pai_private_key = serialization.load_pem_private_key(
            pai_private_key_file.read_bytes(),
            None,
            default_backend()
        )

        # Read Certificate: from PEM
        if pai_certificate_file.suffix == '.pem':
            cert = x509.load_pem_x509_certificate(pai_certificate_file.read_bytes(), default_backend())
        # Read certificate: from DER
        elif pai_certificate_file.suffix == '.der':
            cert = x509.load_der_x509_certificate(pai_certificate_file.read_bytes(), default_backend())
        else:
            raise ValueError("PAI certificate file in unknown format.")

        # Create object and then re-serialize to validate the certificate data
        cls.pai_certificate_bytes = cert.public_bytes(serialization.Encoding.DER)
        cls.pai_certificate_path = pai_certificate_file

        # Set serial number
        cls.device_serial_number = serial_number

        cls.output_path = output_path
        cls.output_path.mkdir(exist_ok=True, parents=True)
        cls.cfg_file = cfg_file
        cls.mft_cd = mft_cd
        qmatter_tools['spake2p'] = str(Path.cwd().joinpath("assets", "QMatter", "spake2p"))

    @classmethod
    def generate_factory_data(
        cls,
        target_public_key: bytes,
        target_802_15_4_address: str,
        target_ble_address: str,
        mandatory_fields: Dict[str, str],
    ) -> Tuple[bytes, List[api_objects.GenerateFactoryData.KeyType]]:
        """
        References:
        https://github.com/project-chip/connectedhomeip/blob/master/credentials/test/gen-test-attestation-certs.sh
        """
        data = [f"\\x{val:0{2}x}" for val in target_public_key]
        logging.info(
            f"Enrolling device with 802.15.4 address: {target_802_15_4_address}"
            f" BLE: {''.join(target_ble_address)}"
            f" and Public key: b'{''.join(data)}'"
        )

        # Create public key from given encode point, prepend 0x04 to the string to indicate full (x,y) point
        # This is different from the private key and certificate load from files, as the encoding is different
        dac_public_key = ec.EllipticCurvePublicKey.from_encoded_point(
            ec.SECP256R1(),
            bytes([0x04]) + target_public_key
        )

        certprop = DACProperties(
            f'Matter Test DAC {cls.device_serial_number}',
            u'Matter Test PAI',
            vid=0xFFF1,
            pid=0x8000
        )
        certprop.properties.serialnumber = cls.device_serial_number

        # Create certificates
        dac_certificate = create_matter_certificate(
            dac_public_key,
            cls.pai_private_key,
            certprop.properties,
            (datetime.datetime.now(), datetime.timedelta(days=365)),
        )

        # Add the device certificate in the base address
        dac_cert_path = cls.output_path / f"dac_cert_{cls.device_serial_number}.der"
        dac_cert_path.write_bytes(dac_certificate.public_bytes(serialization.Encoding.DER))

        dac_pubkey_path = cls.output_path / f"dac_pubkey_{cls.device_serial_number}.der"
        dac_pubkey_path.write_bytes(
            dac_public_key.public_bytes(serialization.Encoding.DER, serialization.PublicFormat.SubjectPublicKeyInfo)
        )

        logging.info(f"Generated certificate with serial # {cls.device_serial_number}.")
        # Auto-increment serial number
        cls.device_serial_number += 1
        cli_args = [
            f"--dac-cert={dac_cert_path}",
            f"--dac-pubkey={dac_pubkey_path}",
            f"--pai-cert={cls.pai_certificate_path}",
            f"--certification-declaration={cls.mft_cd}"
        ]
        args = parse_command_line_arguments(cli_args + cls.cfg_file.read_text().splitlines())
        data = generate_factory_bin(args)

        # Respond to client with complete signed certificate bundle
        # The private key was supplied by the device, so we don't return any extra keys
        return data, []
