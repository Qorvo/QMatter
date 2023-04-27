"""QPU Certificate Signing API v1.0; Reference Development Implementation
This file declares the data interface of the API, and can be imported without
any external dependencies for run-time inspection
"""
from typing import List, Tuple, Any, Dict, Optional
from abc import ABC, abstractmethod
import json
from base64 import b64decode, b64encode

API_VERSION = "v1.0"


# Base Classes
class APIObject(ABC):
    """Base API object allowing client and server to use the same data definition."""
    # HTTP request method. E.g. GET
    method = ""
    # HTTP header content-type definition. E.g. text/plain
    content_type = ""
    # HTTP request path
    path = ""

    def __init__(self):
        self.server_version = ""
        self.client_version = ""

    @abstractmethod
    def to_message(self, server_side: bool = False) -> bytes:
        """Serialize `APIObject` into the HTTP request body.

        :param server_side: Boolean indicating if the caller is on the server or client side.
        :return: Binary representation of the request.
        """
        ...

    def _add_base_info(self, data: Dict[str, Any], server_side: bool) -> Dict[str, Any]:
        if server_side:
            data['SERVER_API_VERSION'] = API_VERSION
            self.server_version = API_VERSION
        else:
            data['CLIENT_API_VERSION'] = API_VERSION
            self.client_version = API_VERSION
        return data

    @abstractmethod
    def from_message(self, data: bytes, server_side: bool = False) -> None:
        """De-serialize data from a HTTP request body into the current `APIObject` request.

        :param data: Binary representation of the request.
        :param server_side: Boolean indicating if the caller is on the server or client side.
        """
        ...

    def _get_base_info(self, data: Dict[str, Any], server_side: bool) -> None:
        if server_side:
            self.server_version = API_VERSION
            self.client_version = data['CLIENT_API_VERSION']
        else:
            self.client_version = API_VERSION
            self.server_version = data['SERVER_API_VERSION']

    def process(self):
        """Execute server-side actions based on the request data, udpate the request data and return. Implementation
        should be overriden by child classes.
        """
        return

    @property
    def return_code(self) -> Tuple[int, str]:
        """Expected HTTP return code and reason.

        :return: HTTP return code and reason string.
        """
        return (200, "OK")


class APIJSONObject(APIObject):
    # JSON requests should always use POST
    method = "POST"
    content_type = "application/json"

    @abstractmethod
    def _to_dict(self, server_side: bool) -> Dict[str, Any]:
        ...

    @abstractmethod
    def _from_dict(self, data: Dict[str, Any], server_side: bool) -> None:
        ...

    def to_message(self, server_side: bool = False) -> bytes:
        ret = self._to_dict(server_side)
        data = self._add_base_info(ret, server_side)
        return json.dumps(data).encode('utf8')

    def from_message(self, data: bytes, server_side: bool = False) -> None:
        ret = json.loads(data.decode())
        self._get_base_info(ret, server_side)
        self._from_dict(ret, server_side)


# Protocol Messages
class IsAlive(APIObject):
    """IsAlive returns a constant string with the sole purpose of allowing human feedback of the server's status. It is
    also the only request that does not use JSON data or transmits the API version. This implies that *all* API servers
    must support this call.
    """
    method = "GET"
    content_type = "text/plain"
    path = "/is_alive"

    def __init__(self):
        # Request
        # Response
        self.message = ""

    @classmethod
    def request(cls):
        return cls()

    def response(self, message: str):
        self.message = message

    def to_message(self, server_side: bool = False) -> bytes:
        if server_side:
            return self.message.encode('utf8')
        else:
            return bytes()

    def from_message(self, data: bytes, server_side: bool = False) -> None:
        if server_side:
            pass
        else:
            self.message = data.decode()


class CheckServerInfo(APIJSONObject):
    """The client supplies its version to the server through a `CheckServerInfo` call, expecting a response
    from the server with its version (for auditing purposes) and if it is compatible with the client's API version.
    """
    path = "/check_server_info"

    def __init__(self):
        # Request
        # Response
        self.server_version = ""
        self.server_is_compatible = False

    @classmethod
    def request(cls):
        return cls()

    def response(self, server_version: str, is_compatible: bool):
        self.server_version = server_version
        self.server_is_compatible = is_compatible

    def _to_dict(self, server_side: bool) -> Dict[str, Any]:
        if server_side:
            data = {
                'server':
                {
                    'version': self.server_version,
                    'compatible': self.server_is_compatible,
                },
            }
        else:
            data = {}
        return data

    def _from_dict(self, data: Dict[str, Any], server_side: bool) -> None:
        if server_side:
            pass
        else:
            self.server_version = data['server']['version']
            self.server_is_compatible = data['server']['compatible']


class GenerateFactoryData(APIJSONObject):
    """With the `GenerateFactoryData` call the client supplies the target's data and requests the corresponding
    factory data block for the target.
    """
    path = "/generate_factorydata"

    class KeyType:
        def __init__(self, keyindex: int, key: bytes, keytype: str, intransitindex: int, hash: bytes) -> None:
            self.keyindex = keyindex
            self.key = key
            self.keytype = keytype
            self.intransitindex = intransitindex
            self.hash = hash

        def to_dict(self) -> Dict[str, Any]:
            return {
                'index': self.keyindex,
                'key': b64encode(self.key).decode('utf-8'),
                'type': self.keytype,
                'intransitindex': self.intransitindex,
                'hash': b64encode(self.hash).decode('utf-8'),
            }

        def from_dict(self, data: Dict[str, Any]) -> None:
            self.keyindex = data['index']
            self.key = b64decode(data['key'])
            self.keytpe = data['type']
            self.intransitindex = data['intransitindex']
            self.hash = b64decode(data['hash'])

        def __repr__(self):
            return str({
                'index': self.keyindex,
                'key': self.key.hex(),
                'type': self.keytype,
                'intransitindex': self.intransitindex,
                'hash': self.hash.hex(),
            })

    def __init__(self):
        # Request
        self.target_public_key = bytes()
        self.target_802_15_4_address = ""
        self.target_ble_address = ""
        self.in_session_private_key_generation = False
        self.mandatory_fields: Dict[str, str] = {}
        self.matching_fields: Dict[str, str] = {}
        # Response
        self.factory_data = bytes()
        self.keys: List[GenerateFactoryData.KeyType] = []
        self.success = False

    @classmethod
    def request(
        cls,
        public_key: bytes,
        device_802_15_4_address: str,
        ble_address: str,
        in_session_private_key_generation: bool,
        mandatory_fields: Optional[Dict[str, str]] = None,
        matching_fields: Optional[Dict[str, str]] = None,
    ) -> 'GenerateFactoryData':
        obj = cls()
        obj.target_public_key = public_key
        obj.target_802_15_4_address = device_802_15_4_address
        obj.target_ble_address = ble_address
        obj.in_session_private_key_generation = in_session_private_key_generation
        obj.mandatory_fields = mandatory_fields or {}
        obj.matching_fields = matching_fields or {}
        obj.keys = []
        return obj

    def response(self, factory_data: bytes, keys: List[KeyType], success: bool) -> None:
        self.factory_data = factory_data
        self.keys = keys
        self.success = success

    def _to_dict(self, server_side: bool) -> Dict[str, Any]:
        if server_side:
            data = {
                'factory_data': b64encode(self.factory_data).decode('utf-8'),
                'keys': [key.to_dict() for key in self.keys],
                'success': self.success
            }
        else:
            data = {
                'target_public_key': b64encode(self.target_public_key).decode('utf-8'),
                'target_802_15_4_address': self.target_802_15_4_address,
                'target_ble_address': self.target_ble_address,
                'in_session_private_key_generation': self.in_session_private_key_generation,
                'certificate_fields': {
                    'mandatory': self.mandatory_fields,
                    'matching': self.matching_fields,
                }
            }
        return data

    def _from_dict(self, data: Dict[str, Any], server_side: bool) -> None:
        if server_side:
            self.target_public_key = b64decode(data['target_public_key'])
            self.target_802_15_4_address = data['target_802_15_4_address']
            self.target_ble_address = data['target_ble_address']
            self.in_session_private_key_generation = data['in_session_private_key_generation']
            self.mandatory_fields = data['certificate_fields']['mandatory']
            self.matching_fields = data['certificate_fields']['matching']
        else:
            self.factory_data = b64decode(data['factory_data'])
            data_keys = data['keys']
            keys = [GenerateFactoryData.KeyType(0, b"", "", 0, b"")] * len(data_keys)
            for d_key, key in zip(data_keys, keys):
                key.from_dict(d_key)
            self.keys = keys
            self.success = data['success']
