from typing import Union, Tuple, List, Optional
from pathlib import Path
import datetime

from cryptography import x509
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.asymmetric.types import CERTIFICATE_PRIVATE_KEY_TYPES


class CertificateAttributes:
    # Name and value references: https://github.com/project-chip/connectedhomeip/blob/5924088692dc666f748d7296e3b1508ecdd2c045/src/tools/chip-cert/GeneralUtils.cpp
    ChipAttestationAttrVID = "1.3.6.1.4.1.37244.2.1"
    ChipAttestationAttrPID = "1.3.6.1.4.1.37244.2.2"


class CertificateProperties:
    def __init__(
        self,
        subject: List[x509.NameAttribute],
        issuer: List[x509.NameAttribute],
        constraint: x509.BasicConstraints,
        keyusage: x509.KeyUsage,
        serialnumber: Optional[int] = None,
    ):
        self.subject = subject
        self.issuer = issuer
        self.constraint = constraint
        self.keyusage = keyusage
        self.serialnumber = serialnumber


class MatterCertificateProperties:
    def __init__(self, subject_name: str, issuer_name: str, vid: str, pid: str):
        self._properties = None

    @property
    def properties(self) -> CertificateProperties:
        assert self._properties is not None, "MatterCertificateProperties object is ill-formed."
        return self._properties


class DACProperties(MatterCertificateProperties):
    def __init__(self, subject_name: str, issuer_name: str, vid: int, pid: int):
        self._properties = CertificateProperties(
            subject=[
                x509.NameAttribute(x509.oid.NameOID.COMMON_NAME, subject_name),
                x509.NameAttribute(x509.ObjectIdentifier(CertificateAttributes.ChipAttestationAttrVID), f"{vid:0{4}x}"),
                x509.NameAttribute(x509.ObjectIdentifier(CertificateAttributes.ChipAttestationAttrPID), f"{pid:0{4}x}")
            ],
            issuer=[
                x509.NameAttribute(x509.oid.NameOID.COMMON_NAME, issuer_name),
                x509.NameAttribute(x509.ObjectIdentifier(CertificateAttributes.ChipAttestationAttrVID), f"{vid:0{4}x}"),
                x509.NameAttribute(x509.ObjectIdentifier(CertificateAttributes.ChipAttestationAttrPID), f"{pid:0{4}x}")
            ],
            constraint=x509.BasicConstraints(False, None),
            keyusage=x509.KeyUsage(True, False, False, False, False, False, False, False, False),
        )


class PAIProperties(MatterCertificateProperties):
    def __init__(self, subject_name: str, issuer_name: str, vid: int, pid: int):
        self._properties = CertificateProperties(
            subject=[
                x509.NameAttribute(x509.oid.NameOID.COMMON_NAME, subject_name),
                x509.NameAttribute(x509.ObjectIdentifier(CertificateAttributes.ChipAttestationAttrVID), f"{vid:{4}x}"),
                x509.NameAttribute(x509.ObjectIdentifier(CertificateAttributes.ChipAttestationAttrPID), f"{pid:{4}x}")
            ],
            issuer=[
                x509.NameAttribute(x509.oid.NameOID.COMMON_NAME, issuer_name)
            ],
            constraint=x509.BasicConstraints(True, 0),
            keyusage=x509.KeyUsage(False, False, False, False, False, True, True, False, False),
        )


class PAAProperties(MatterCertificateProperties):
    def __init__(self, subject_name: str, issuer_name: str, vid: int, pid: int):
        self._properties = CertificateProperties(
            subject=[
                x509.NameAttribute(x509.oid.NameOID.COMMON_NAME, subject_name)
            ],
            issuer=[
                x509.NameAttribute(x509.oid.NameOID.COMMON_NAME, issuer_name)
            ],
            constraint=x509.BasicConstraints(True, 1),
            keyusage=x509.KeyUsage(False, False, False, False, False, True, True, False, False),
        )


# ##############################################################################
# Certificate generation and signing functions
# ##############################################################################
def create_matter_certificate(
    public_key: ec.EllipticCurvePublicKey,
    private_key: CERTIFICATE_PRIVATE_KEY_TYPES,
    certprop: CertificateProperties,
    validity_window: Union[Tuple[datetime.datetime, datetime.datetime], Tuple[datetime.datetime, datetime.timedelta]],
) -> x509.Certificate:
    """Generate and sign a Matter authentication certificate (DAC, PAI or PAA).

    :param public_key: Public key to be authenticated
    :param private_key: Signing authority private key
    :param certificate_type: Matter certificate type to generate
    :param validity_window: Time window which the certificate is valid
    :return: The built signed certificate
    """
    # Build an example certificate based on the example certificates based on:
    #   TOOLS_Device_Attestation
    #   https://gist.github.com/bloodearnest/9017111a313777b9cce5

    start_date = validity_window[0]
    if isinstance(validity_window[1], datetime.datetime):
        # Directly set the end date
        end_date = validity_window[1]
    else:
        # Calculate end date from delta, if required
        end_date = validity_window[0] + validity_window[1]

    builder = (
        x509.CertificateBuilder()
        # Add certificate properties
        .subject_name(x509.Name(certprop.subject))
        .issuer_name(x509.Name(certprop.issuer))
        # Add start and end date
        .not_valid_before(start_date)
        .not_valid_after(end_date)
        # Add public key to authenticate
        .public_key(public_key)
        # Add extensions
        .add_extension(certprop.constraint, critical=True)
        .add_extension(certprop.keyusage, critical=True)
        .add_extension(
            x509.SubjectKeyIdentifier.from_public_key(public_key),
            critical=False
        )
        .add_extension(
            x509.AuthorityKeyIdentifier.from_issuer_public_key(private_key.public_key()),
            critical=False
        )
    )

    # Add device information: serial number + public key
    if certprop.serialnumber:
        builder = builder.serial_number(certprop.serialnumber)

    return builder.sign(private_key=private_key, algorithm=hashes.SHA256(), backend=default_backend())


def generate_key_pair() -> Tuple[Tuple[ec.EllipticCurvePrivateKey, bytes], Tuple[ec.EllipticCurvePublicKey, bytes]]:
    """Generates a public/private key-pair, with pre-defined configuration parameters."""
    privkey = ec.generate_private_key(ec.SECP256R1(), default_backend())
    privkey_bytes = privkey.private_bytes(
        serialization.Encoding.PEM,
        serialization.PrivateFormat.TraditionalOpenSSL,
        serialization.NoEncryption(),
    )
    pubkey = privkey.public_key()
    pubkey_bytes = pubkey.public_bytes(
        serialization.Encoding.PEM,
        serialization.PublicFormat.SubjectPublicKeyInfo
    )
    return ((privkey, privkey_bytes), (pubkey, pubkey_bytes))


def save_certificate(
    cert: x509.Certificate,
    filename: Path,
    encoding: serialization.Encoding = serialization.Encoding.PEM
):
    """This function's objective is to exemplify the procedure to save matter certificates."""
    with open(filename, 'wb') as f:
        f.write(cert.public_bytes(encoding))
