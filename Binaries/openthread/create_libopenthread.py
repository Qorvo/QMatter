import sys
import os

import pathlib
import subprocess


def check_type(type_: str) -> str:
    accepted = ['ftd', 'mtd', 'rcp']
    if type_ not in accepted:
        raise ValueError(f"Argument ({type_}) need to be in {accepted}")
    return type_


def create_lib(output_file: str, chip: str, type_: str, tcp: bool) -> None:
    print(f"Create libopenthread-{type_}.a with {'' if tcp else 'no '}tcp support > {os.path.basename(output_file)}")
    path = os.path.realpath(os.path.join(__file__, '..'))
    script_name = "libopenthread.mri"
    script = ([f"create {output_file}",
               f"addlib {path}/libopenthread-{type_}.a",
               f"addlib {path}/libopenthread-{chip}-{type_}.a",
               f"addlib {path}/libmbedcrypto.a",
               f"addlib {path}/libmbedtls.a",
               f"addlib {path}/libmbedx509.a"] +
              ([] if not tcp else [f"addlib {path}/libtcplp-{type_}.a"]) +
              ["save",
              "end"])
    with open(script_name, 'w+', encoding='utf-8') as file:
        file.write('\n'.join(script))

    subprocess.run(['ar', '-M'], stdin=open(script_name))
    pathlib.Path(script_name).unlink()


if __name__ == "__main__":
    argc = len(sys.argv)
    path_idx = 1
    chip_idx = 2
    type_idx = 3
    tcp_idx = 4

    if argc > tcp_idx:
        tcp = not not int(sys.argv[tcp_idx])
    else:
        tcp = False

    if argc > type_idx:
        type_ = check_type(sys.argv[type_idx])
    else:
        type_ = 'ftd'

    if argc > chip_idx:
        chip = sys.argv[chip_idx]
    else:
        ValueError("Not enough arguments")

    if argc > path_idx:
        path = sys.argv[path_idx]
    else:
        ValueError("Not enough arguments")

    create_lib(path, chip, type_, tcp)
    sys.exit(0)
