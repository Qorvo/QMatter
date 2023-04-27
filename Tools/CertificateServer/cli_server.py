"""Example HTTP server to provide certificate signing services through an HTTP API.
"""
import sys
from pathlib import Path
from typing import List, Type

# Adjust package variable so that relative imports work as expected
# https://stackoverflow.com/questions/16981921/relative-imports-in-python-3
# https://stackoverflow.com/questions/14132789/relative-imports-for-the-billionth-time
if not __package__:
    file = Path(__file__).resolve()
    parent, top = file.parent, file.parents[1]
    sys.path = [str(top)] + sys.path
    # remove our own path
    try:
        sys.path.remove(str(parent))
    except ValueError:  # already removed
        pass
    __package__ = file.parent.name

import logging

try:
    import coloredlogs
    coloredlogs.install(fmt="%(asctime)s %(name)s %(levelname)s %(message)s", level=logging.DEBUG)
except ImportError:
    logging.basicConfig(format="%(asctime)s %(name)s %(levelname)s %(message)s", level=logging.INFO)
    pass

import click

from . server import Server
# Import the modified objects from server_actions, instead of the original definitions without actions
from . import server_actions
from . import api_objects
from . assets.samples.matter_auth import files as matter_gend_files
from . assets.QMatter import files as qmatter_files
import struct
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes


def find_hash_user_key(user_key: bytes, user_key_index: int):
    # user key starts from compressed address 0x380180 and has length 16 bytes.
    key_address = 0x00380180 + 16 * user_key_index
    address_key_in_bytes = struct.unpack('4B', struct.pack('<I', key_address))
    cipher_in = bytearray(address_key_in_bytes) + bytearray(12 * [0x00])

    cipher = Cipher(algorithms.AES(user_key), modes.ECB())
    encryptor = cipher.encryptor()
    hash_ = encryptor.update(bytes(cipher_in)) + encryptor.finalize()
    return bytearray(hash_)


# ##############################################################################
# Declare CLI interface (arguments)
# ##############################################################################
@click.command()
@click.option(
    '-h',
    '--hostname',
    type=click.STRING,
    default='localhost',
    help="The hostname that the server will respond to."
)
@click.option(
    '-p',
    '--port',
    type=click.INT,
    default=8000,
    help="The port which the server will listen in."
)
@click.option(
    '-c',
    '--certificate',
    type=click.Path(exists=True, readable=True, resolve_path=True, dir_okay=False, file_okay=True, path_type=Path),
    help="A SSL certiifcate to be used in the server when when listening to connections."
)
@click.option(
    '-k',
    '--keyfile',
    type=click.Path(exists=True, readable=True, resolve_path=True, dir_okay=False, file_okay=True, path_type=Path),
    help="A SSL key file to be used by the server, in conjunction with the certificate file."
)
@click.option(
    '--client-certificates',
    type=click.Path(exists=True, readable=True, resolve_path=True, dir_okay=True, file_okay=True, path_type=Path),
    help="A path to a directory containing accepted client certificates (or to a single accepted certificate file)."
)
@click.option(
    '--no-check-hostname',
    is_flag=True,
    default=False,
    help="Disables checking of incoming request hostname against the supplied server certificate's domain."
)
@click.option(
    '--run',
    type=click.Choice(['target', 'saved']),
    default='saved',
    help="The target's private key source: in-target generation or pre-generated elsewhere (saved).",
)
def main(
    hostname: str,
    port: int,
    certificate: Path,
    keyfile: Path,
    client_certificates: Path,
    no_check_hostname: bool,
    run: str,
) -> None:
    """Start up a HTTP listener and expose the certificate generation API.
    """
    calls: List[Type[api_objects.APIObject]] = [server_actions.IsAlive, server_actions.CheckServerInfo]
    if run == "target":
        logging.info("Preparing server for in-target private key generation.")
        server_actions.GenerateFactoryDataWithTargetGeneratedKey.InitStaticData(
            matter_gend_files.pai_private_key,
            matter_gend_files.pai_cert_pem,
            33,
            Path.cwd() / "generated",
            qmatter_files.example_factory_data_config,
            qmatter_files.Credentials.qorvo_cd_bin,
        )
        calls.append(server_actions.GenerateFactoryDataWithTargetGeneratedKey)
    if run == "saved":
        # Use pre-generated data. This example uses a combination of hard-coded keys and hashes with pre-generated
        # certificate files (from Matter's certtool).

        logging.info("Preparing server for saved private key values.")
        # random value for now
        key_8_bytes = b"\xFA\xFD\x74\x05\x44\x4C\x12\x0F\xA3\x8C\x3A\x26\x8C\x3B\xEE\x4B"
        key_9_bytes = b"\xCB\x07\x55\x35\xC5\x8C\xFC\xE7\x3c\x50\xD1\x30\xB9\xC1\xCD\x4D"
        hash_8_bytes = find_hash_user_key(key_8_bytes, 8)
        hash_9_bytes = find_hash_user_key(key_9_bytes, 9)
        # Keytype can be "plain" or "intransit"
        key_list = [
            api_objects.GenerateFactoryData.KeyType(8, key_8_bytes, 'plain', 0, hash_8_bytes),
            api_objects.GenerateFactoryData.KeyType(9, key_9_bytes, 'plain', 0, hash_9_bytes),
        ]
        logging.warning("Qorvo does not provide user keys or user key hashes.")

        server_actions.GenerateFactoryDataWithPreGeneratedData.InitStaticData(
            qmatter_files.Credentials.qorvo_dac_cert_0_der,
            qmatter_files.Credentials.qorvo_dac_key_0_der,
            key_list,
            qmatter_files.Credentials.qorvo_pai_cert_der,
            qmatter_files.Credentials.qorvo_cd_bin,
            qmatter_files.example_factory_data_config,
        )
        calls.append(server_actions.GenerateFactoryDataWithPreGeneratedData)

    logging.info("Server will listen to these endpoints: " + ", ".join((call.__name__ for call in calls)))
    # Create our server object
    server = Server(
        hostname,
        port,
        calls,
        certificate,
        keyfile,
        client_certificates,
        not no_check_hostname,
    )
    # Block and wait for incoming connections
    server.serve()


if __name__ == "__main__":
    # ignore error caused by click
    # pylint: disable=no-value-for-parameter
    main()
