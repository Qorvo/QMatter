"""Example HTTP server to provide certificate signing services through an HTTP API.
"""
from typing import Type, Optional, Tuple, Dict, List
from http.server import HTTPServer, BaseHTTPRequestHandler
import ssl
import logging
import os
from collections import defaultdict
from pathlib import Path

from . api_objects import APIObject


class NotFound404(APIObject):
    """Simplistic class for handling 404 requests."""
    content_type = "text/plain"

    def to_message(self, server_side: bool = False) -> bytes:
        return "Nothing to see here people, move along!".encode(encoding='utf8')

    def from_message(self, data: bytes, server_side: bool = False):
        return

    @property
    def return_code(self) -> Tuple[int, str]:
        return (404, "Not Found")


# ##############################################################################
# Declare request handler and server class (handle request and create response)
# ##############################################################################
class SigningRequestHandler(BaseHTTPRequestHandler):
    """HTTP Handler that generates and signs certificates for Qorvo device attestation."""

    method_path_message_map: Dict[str, Dict[str, Type[APIObject]]] = defaultdict(dict)

    @staticmethod
    def add_supported_calls(calls: List[Type[APIObject]]):
        for msg in calls:
            SigningRequestHandler.method_path_message_map[msg.method][msg.path] = msg

    logger = logging.getLogger("Request")

    def do_GET(self):
        """GET request handlers, forwarded to `do_request`."""
        self.do_request("GET")

    def do_POST(self):
        """POST request handlers, forwarded to `do_request`."""
        self.do_request("POST")

    def do_request(self, method: str):
        """Handle incoming requests parametrically by checking the `method_path_message_map`
        dictionary for the available `method` and `path` to handler mapping.

        :param method: Incoming request HTTP method.
        """
        obj_t = self.method_path_message_map.get(method, {}).get(self.path, NotFound404)
        obj = obj_t()
        if method == "GET":
            received_data = bytes()
        else:
            received_data = self.rfile.read(int(self.headers['content-length']))
        self.logger.debug(f"Client {self.client_address} has a {method} request in {self.path} with {received_data}")
        obj.from_message(received_data, server_side=True)
        obj.process()
        response = obj.to_message(server_side=True)
        self.logger.debug(f"Replying to client with {response}")
        self.send_response(obj.return_code[0], obj.return_code[1])
        self.send_header("Content-type", obj.content_type)
        self.end_headers()
        if response:
            self.wfile.write(response)


class Server:
    def __init__(
        self,
        hostname: str,
        port: int,
        calls: List[Type[APIObject]],
        certificate: Optional[Path] = None,
        keyfile: Optional[Path] = None,
        client_certificates: Optional[Path] = None,
        check_request_hostname: bool = True,
    ) -> None:
        """Server that listens locally at the specified port, using the `handler` class to handle incoming requests.

        :param hostname: Hostname of this server.
        :param port: Port which to listen at.
        :param calls: Supported API objects to respond to.
        :param certificate: Server SSL/TLS certificate to use for traffic encryption, defaults to None.
        :param keyfile: Server certificate key (required when using `certificate`), defaults to None.
        :param client_certificates: Path to a directory or file of authenticated clients (optional), defaults to None.
        :param check_request_hostname: Check incoming requests's hostname against the hostname declared in the used.
            security certificate, defaults to True.
        """
        self.hostname = hostname
        self.port = port
        self._logger = logging.getLogger("Server")
        # Setup supported API calls
        SigningRequestHandler.add_supported_calls(calls)
        self._server = HTTPServer((self.hostname, self.port), SigningRequestHandler)
        if certificate:
            # If communication security is required, add the required SSL context
            sslctx = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
            sslctx.check_hostname = check_request_hostname
            sslctx.load_cert_chain(certfile=certificate, keyfile=keyfile)
            if client_certificates:
                # In case the server enforces client authentication via certificates
                sslctx.verify_mode = ssl.CERT_REQUIRED
                if client_certificates.is_file():
                    sslctx.load_verify_locations(cafile=client_certificates)
                else:
                    sslctx.load_verify_locations(capath=client_certificates)
            self._server.socket = sslctx.wrap_socket(self._server.socket, server_side=True)

    def serve(self) -> None:
        """Block current thread and listen for connections at `port` specified in object construction."""
        self._logger.info("Server is now serving...")
        try:
            self._server.serve_forever()
        except KeyboardInterrupt:
            pass
        except Exception as ex:
            self._logger.exception("Something happened to the server.", ex)
        finally:
            # Always release the underlying socket; this allows other instances to use the same address/port
            self._server.server_close()
        self._logger.info("Server stopped.")

    def shutdown(self) -> None:
        """Shutdown server. Socket resource is released automatically.

        As with the HTTPServer, this should be called from a different thread, as it is blocking; otherwise this will
        result in a dead-lock.
        """
        self._server.shutdown()
