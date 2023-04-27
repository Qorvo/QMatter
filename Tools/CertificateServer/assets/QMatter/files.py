from pathlib import Path
_cwf = Path(__file__).parent / "../../../FactoryData"

example_factory_data_config = _cwf / "Credentials" / "qorvo_example.factory_data_config"


class Credentials:
    qorvo_dac_cert_0_der = _cwf / "Credentials" / "qorvo_base_dac_cert_0.der"
    qorvo_dac_key_0_der = _cwf / "Credentials" / "qorvo_base_dac_key_0.der"
    qorvo_pai_cert_der = _cwf / "Credentials" / "qorvo_base_pai_cert.der"
    qorvo_cd_bin = _cwf / "Credentials" / "qorvo_base_cd.bin"
