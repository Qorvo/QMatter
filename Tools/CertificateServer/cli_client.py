"""Example HTTP client to test the production factory data generation server.
"""
import sys
from pathlib import Path

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

from typing import Union, TypeVar
import http.client
import ssl
import logging

import click

try:
    import coloredlogs
    coloredlogs.install(fmt="%(asctime)s %(name)s %(levelname)s %(message)s", level=logging.INFO)
except ImportError:
    pass

from . api_objects import IsAlive, CheckServerInfo, GenerateFactoryData
T = TypeVar('T', IsAlive, CheckServerInfo, GenerateFactoryData)


# ##############################################################################
# Declare connection client (handles security and message (de)serialization)
# ##############################################################################
class Client:
    """Generic API client that accepts `APIObject` requests."""
    _client_counter = 0

    @staticmethod
    def _get_clientid():
        Client._client_counter += 1
        return Client._client_counter

    def __init__(
        self,
        hostname: str,
        port: int,
        secure: bool,
        server_cert: str,
        client_cert: str,
        client_key: str,
    ) -> None:
        self.hostname = hostname
        self.port = port
        self._server_certificate = server_cert
        self._client_certificate = client_cert
        self._client_key = client_key
        self._secure_http = secure
        self._client_id = 0
        self.logger = logging.getLogger(f"Client_{self.client_id}")

        if self._secure_http:
            # In case SSL is being used, take care to check if the arguments are OK and create the secure context
            self._context = ssl.create_default_context(ssl.Purpose.SERVER_AUTH, cafile=server_cert)
            # Disabling hostname check is required as, in this example, we are not accessing the server with the
            # hostname that the certificate was generated with.
            self._context.check_hostname = False
            # context.load_default_certs()
            self._context.load_verify_locations(cafile=server_cert)
            self._context.load_cert_chain(certfile=client_cert, keyfile=client_key)
        else:
            self._context = None
        self._connection = None

    @property
    def client_id(self) -> int:
        if self._client_id == 0:
            self._client_id = self._get_clientid()
        return self._client_id

    @property
    def connection(self) -> Union[http.client.HTTPConnection, http.client.HTTPSConnection]:
        """Expose connection object."""
        if self._connection is None:
            self._connection = self._get_connection()
        return self._connection

    def _get_connection(self) -> Union[http.client.HTTPConnection, http.client.HTTPSConnection]:
        """Internal helper function to get the correct connection object, according to object's construction parameters.
        """
        if self._secure_http:
            return http.client.HTTPSConnection(self.hostname, self.port, context=self._context)
        else:
            return http.client.HTTPConnection(self.hostname, self.port)

    def query(self, request: T) -> T:
        """Serialize and execute the supplied API request to the server specified at object construction."""
        logging.debug(f"Executing {type(request)}")
        conn = self.connection
        headers = {"Content-type": request.content_type}
        conn.request(request.method, request.path, request.to_message(), headers)
        response = conn.getresponse()
        content = response.read()
        logging.debug(f"Received back: {content}")
        if response.getcode() != request.return_code[0]:
            logging.error("Wrong code received! Something must have gone wrong.")
            logging.warn(response.reason)
            logging.debug(content)
            raise RuntimeError(
                f"Wrong return code received. Expected {request.return_code[0]} but got {response.getcode()}"
            )
        request.from_message(content)
        return request


# ##############################################################################
# Declare CLI interface (arguments) and example flow
# ##############################################################################
@click.command()
@click.option(
    '-h',
    '--hostname',
    type=click.STRING,
    default='localhost',
    help="Server hostname or IP to connect to."
)
@click.option(
    '-p',
    '--port',
    type=click.INT,
    default=8000,
    help="Server port to connect to."
)
@click.option(
    '--secure',
    is_flag=True,
    help="Use SSL to connect with server (use HTTPS requests instead of HTTP)."
)
@click.option(
    '--server-cert',
    type=click.Path(exists=True, readable=True, resolve_path=True, dir_okay=False, file_okay=True),
    help="Server certificate file (for validation when using self-signed certificates)."
)
@click.option(
    '--client-cert',
    type=click.Path(exists=True, readable=True, resolve_path=True, dir_okay=False, file_okay=True),
    help="Certificate for client identity verification by the server."
)
@click.option(
    "--client-key",
    type=click.Path(exists=True, readable=True, resolve_path=True, dir_okay=False, file_okay=True),
    help="Key file for client certificate."
)
@click.option(
    "--example-flow",
    is_flag=True,
    help="Execute sample client certificate generation and signing flow"
)
def main(
    hostname: str,
    port: int,
    secure: bool,
    server_cert: str,
    client_cert: str,
    client_key: str,
    example_flow: bool
) -> None:
    client = Client(hostname, port, secure, server_cert, client_cert, client_key)
    if example_flow:
        execute_example_flow(client)
    else:
        logging.info(client.query(IsAlive.request()).message)


# ##############################################################################
# Example connection and request flow
# ##############################################################################
def execute_example_flow(client: Client) -> None:
    # Create some dummy device data
    target_data = [
        (
            b'\x71\xbc\x00\x88\x28\xf0\x3f\xdc\x80\x08\x17\x63\xd0\x14\xf3\x2f\x25\x48\x49\x65\x29\x50\x31\x7f\x4c\x44'
            + b'\x9b\x8f\x8f\x29\x2b\x03\x6a\x57\x5e\xeb\x31\x11\xdd\x45\x79\x1d\x42\xae\x47\x77\xe2\xe4\x84\x63\x1d'
            + b'\xff\xe1\x9d\x3d\xd6\x91\x79\x4e\xbb\xe8\x68\x09\x8b',
            "0x00155F0000000F88", "0x00155F000F88", True, "PROJECT_X",
        ),
        (
            b'\x6d\x18\x12\x81\x08\x54\x64\xd5\x28\xad\xb0\x19\xb9\xc6\xac\x17\x5a\x76\x57\xfd\x85\xd3\xda\x01\xf0\x63'
            + b'\x18\x95\x37\xd5\xd1\xde\x6b\x1a\x85\x85\x4a\x8f\xcf\x62\x60\x56\xa1\xc6\xcc\xe5\xeb\xe9\x21\x3e\x63'
            + b'\x20\xd0\xaa\x94\xba\x22\x7e\x07\x7e\xc1\x8f\x5a\x33',
            "0x00155F0000000F56", "0x00155F000F56", True, "PROJECT_X",
        ),
        (
            b'\x13\x76\xa5\xe3\xe6\xec\x1a\x17\xb4\xea\x97\x9d\x9d\x27\xec\xd4\x89\x9e\xb2\xab\xbc\xa7\x69\x2b\x2e\x21'
            + b'\xcb\xc1\xae\xda\x04\x32\xcc\x84\xf1\xc3\xc8\x4f\xee\xc5\x4a\x6b\xf6\x7d\x53\x4a\x21\x5c\xb9\x85\xaf'
            + b'\x16\x5c\x0b\x28\x99\x8d\x65\xfc\x35\xd7\x1e\x4e\xe2',
            "0x00155F0000000F55", "0x00155F000F55", True, "PROJECT_Y",
        ),
    ]
    # Open the connection to the server
    client.connection.connect()
    logging.info(client.query(IsAlive.request()).message)
    # Check if the server has support for our API version
    server_info = client.query(CheckServerInfo.request())
    if not server_info.server_is_compatible:
        raise RuntimeError(
            f"Connected server ({server_info.server_version}) "
            f"is not compatible wit this client ({server_info.client_version})."
        )
    logging.info(f"Connected to server {client.hostname}:{client.port} with API {server_info.server_version}")
    # Loop through the devices and request signature, report results
    for public_key, mac_address, ble_address, in_session_pk, appid in target_data:
        req = client.query(GenerateFactoryData.request(
            public_key,
            mac_address,
            ble_address,
            in_session_pk,
            matching_fields={"AppID": appid},
        ))
        if req.success:
            logging.info(
                f"Received factory data ({len(req.factory_data) / 1024} kB) "
                f"for device with 802.15.4 address {req.target_802_15_4_address}"
            )
        else:
            logging.warning(
                f"Failed to generate factory data for device with 802.15.4 address {req.target_802_15_4_address})"
            )
    # Close client connection
    client.connection.close()


if __name__ == "__main__":
    main()
