#!/bin/bash

SCRIPT_PATH="$(dirname "$(realpath "${BASH_SOURCE[0]}")")"

log() {
    echo "========= ${1} ============"
}

check_installed_dependency ()
{
    $1 --version foo >/dev/null 2>&1 || {
        echo >&2 "$1 not installed. installing........"
        return 1
    }
    return 0
}

install_node_npm ()
{
    sudo apt-get update

    ### Node.js v16 ###
    curl -fsSL https://deb.nodesource.com/setup_16.x | sudo bash -

    sudo apt install -y nodejs

    if ! npm list installed-check &>/dev/null; then
        npm install installed-check
    fi

    if ! ./node_modules/.bin/installed-check -c &>/dev/null; then
        npm install
    fi
}

install_zap_dependencies ()
{
    sudo apt-get update
    sudo apt-get install -y clang-format npm

    sudo apt-get install -y --fix-missing libpixman-1-dev libcairo-dev libsdl-pango-dev libjpeg-dev libgif-dev

    curl -fsSL https://deb.nodesource.com/setup_16.x | sudo bash -

    sudo apt install -y nodejs

    if ! npm list installed-check &>/dev/null; then
        npm install installed-check
    fi

    if ! ./node_modules/.bin/installed-check -c &>/dev/null; then
        npm install
    fi
}

install_arm_gcc_emb ()
{
    wget -P /tmp --progress=dot:giga https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2019q4/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2
    sudo mkdir -p /opt/TOOL_ARMGCCEMB
    sudo tar -xf /tmp/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 -C /opt/TOOL_ARMGCCEMB
}

install_gn ()
{
    git clone https://gn.googlesource.com/gn /tmp/gn
    python3 /tmp/gn/build/gen.py --out-path=/tmp/gn/out
    ninja -C /tmp/gn/out
    sudo cp /tmp/gn/out/gn /usr/local/bin/gn
    sudo chmod +x /usr/local/bin/gn
}

setup_venv ()
{
    python3.8 -m venv --help >/dev/null 2>&1 || sudo apt-get install -y python3-venv
    python3.8 -m ensurepip --help >/dev/null 2>&1 || sudo apt-get install -y python3-venv

    VENV_PATH=$(realpath "${SCRIPT_PATH}/../.python_venv")
    if [[ ! -d ${VENV_PATH} ]]; then
        mkdir -p "${VENV_PATH}"
    fi
    python3 -m venv "${VENV_PATH}"
    # shellcheck disable=SC1090,SC1091
    source "${VENV_PATH}"/bin/activate
    log "$(python -V)"
    # Install additional modules
    pip3 install dataclasses intelhex click ecdsa cryptography
}

DEFAULT_TOOLCHAIN_DIR=/opt/TOOL_ARMGCCEMB/gcc-arm-none-eabi-9-2019-q4-major

export PATH=$PATH:$DEFAULT_TOOLCHAIN_DIR/bin
export MAKEFLAGS=-s

command -v sudo || apt-get install -y sudo
command -v git || sudo apt-get install -y git
command -v clang || sudo apt-get install -y clang
command -v make || sudo apt-get install -y make
command -v ninja || sudo apt-get install -y ninja-build
command -v curl || sudo apt-get install -y curl

if ! check_installed_dependency node; then
    install_node_npm
fi

if ! check_installed_dependency arm-none-eabi-gcc; then
    install_arm_gcc_emb
fi

if ! check_installed_dependency gn; then
    install_gn
fi

if ! check_installed_dependency npm; then
    install_zap_dependencies
fi

if test -d "$DEFAULT_TOOLCHAIN_DIR"
then
    export TOOLCHAIN="$DEFAULT_TOOLCHAIN_DIR"
fi

setup_venv

git submodule update --init --depth=1 Components/ThirdParty/Matter/repo

(
    cd Components/ThirdParty/Matter/repo || (echo chdir to matter repo failed; exit 1)
    #git submodule update --init --recursive
    for module_path in  \
        third_party/mbedtls \
        third_party/nlassert \
        third_party/nlio \
        third_party/freertos \
        third_party/lwip \
        third_party/openthread \
        third_party/pigweed \
        third_party/qpg_sdk
    do
        git submodule update --init --depth=1 -- "${module_path}"
    done
)

log "$(realpath "${BASH_SOURCE[0]}") Complete"
