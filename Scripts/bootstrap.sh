#!/bin/bash

set -ex

SCRIPT_PATH="$(dirname "$(realpath "${BASH_SOURCE[0]}")")"
VENV_PATH=$(realpath "${SCRIPT_PATH}/../.python_venv")
QMATTER_ROOT_PATH=$(realpath "${SCRIPT_PATH}/..")
BOOTSTRAP_TMP_PATH=$(realpath /tmp)

trap 'on_error $? $LINENO' ERR

on_error() {
  echo "!!! $SCRIPT_PATH failed with error $1 on line $2"
  exit "$1"
}

DEFAULT_TOOLCHAIN_DIR=/opt/TOOL_ARMGCCEMB/arm-gnu-toolchain-12.2.mpacbti-rel1-x86_64-arm-none-eabi

export PATH=$PATH:$DEFAULT_TOOLCHAIN_DIR/bin:${SCRIPT_PATH}/../Tools/FactoryData
export MAKEFLAGS=-s

/proc/self/exe --version 2>/dev/null | grep -q 'GNU bash' ||  (\
    echo "!!!!! This is a BASH script !!!!!"; \
    echo "The shell you are running is $(/proc/self/exe --version)"; \
    echo "Please start bash first by running the 'bash' command"; \
    echo "Press Ctrl-C to abort"; \
    read -r \
)

log() {
    echo "$(realpath "${BASH_SOURCE[0]}") ========= ${1} ============"
}
bootstrap_sh_failure() {
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
    # the 'installed-check' package checks package.json is fulfilled
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
    sudo apt install -y bzip2 tar xz-utils
    wget -P /tmp --progress=dot:giga https://developer.arm.com/-/media/Files/downloads/gnu/12.2.mpacbti-rel1/binrel/arm-gnu-toolchain-12.2.mpacbti-rel1-x86_64-arm-none-eabi.tar.xz
    sudo mkdir -p /opt/TOOL_ARMGCCEMB
    sudo tar -xf /tmp/arm-gnu-toolchain-12.2.mpacbti-rel1-x86_64-arm-none-eabi.tar.xz -C /opt/TOOL_ARMGCCEMB
    rm /tmp/arm-gnu-toolchain-12.2.mpacbti-rel1-x86_64-arm-none-eabi.tar.xz
}

install_gn ()
{
	sudo apt install unzip
	local curDir="${PWD}"
	mkdir -p "$BOOTSTRAP_TMP_PATH/gn"
	cd "$BOOTSTRAP_TMP_PATH/gn" || (bootstrap_sh_failure "chdir to $BOOTSTRAP_TMP_PATH/gn failed"; exit 1)
	wget --content-disposition "https://chrome-infra-packages.appspot.com/dl/gn/gn/linux-amd64/+/latest"
	unzip gn-linux-amd64.zip
	sudo cp gn /usr/local/bin/gn
	sudo chmod +x /usr/local/bin/gn
	rm -rf "$BOOTSTRAP_TMP_PATH/gn"
	cd "$curDir"
}

install_rsync ()
{
    sudo apt-get install -y --no-install-recommends rsync
}

setup_venv ()
{
    Var=$(lsb_release -r)
    NumOnly=$(cut -f2 <<< "$Var")

    #check ubuntu version
    if [[ "$NumOnly" == 22.04 ]]; then
        sudo apt-get install --yes software-properties-common
        sudo apt-get update
        sudo add-apt-repository --yes ppa:deadsnakes/ppa
        sudo apt-get update
    fi

    sudo apt install -y python3.9
    # required for gn exec_script
    sudo apt install -y python-is-python3
    #check openssl minversion
    export openssl_minversion=1.1.1
    # if minversion is not the first in the result, install the deb file
    if ! echo -e "$(openssl version|awk '{print $2}')\n${openssl_minversion}" | sort -V | head -1 | grep -q ${openssl_minversion}
    then
        rm libssl1.1_1.1.1f-1ubuntu2.16_amd64.deb || true
        wget http://nz2.archive.ubuntu.com/ubuntu/pool/main/o/openssl/libssl1.1_1.1.1f-1ubuntu2.16_amd64.deb
        sudo dpkg -i libssl1.1_1.1.1f-1ubuntu2.16_amd64.deb
        rm libssl1.1_1.1.1f-1ubuntu2.16_amd64.deb
    fi

    python3.9 -m venv --help >/dev/null 2>&1 || sudo apt-get install -y python3.9-venv
    python3.9 -m ensurepip --help >/dev/null 2>&1 || sudo apt-get install -y python3.9-venv

    if [[ ! -d ${VENV_PATH} ]]; then
        mkdir -p "${VENV_PATH}"
    fi
    python3.9 -m venv "${VENV_PATH}"
    export VENV_PATH
    # shellcheck source=/dev/null
    source "${VENV_PATH}"/bin/activate
    log "$(python -V)"
    # Install additional modules
    pip3 install wheel dataclasses intelhex click ecdsa cryptography lark jinja2 stringcase pigweed PrettyTable Cheetah3 pylzma
}

setup_submodules ()
{
    # QMatter '-libs' variant lacks the matter submodule to avoid
    # a recursive dependency in project-chip/connectedhomeip
    # For Qorvo-internal CI testing, we add this at validation time
    if test -e "${SCRIPT_PATH}/git_add_submodules.sh"  && test ! -e "${QMATTER_ROOT_PATH}/Components/ThirdParty/Matter/repo/.gitmodules"
    then
        log "Adding submodules to allow package validation"
        # shellcheck source=/dev/null
        source "${SCRIPT_PATH}/git_add_submodules.sh"
    fi

    log "Updating submodules"
    git submodule update --init --depth=1 Components/ThirdParty/Matter/repo

    cd Components/ThirdParty/Matter/repo || (bootstrap_sh_failure "chdir to matter repo failed"; exit 1)
    # TODO: use Components/ThirdParty/Matter/repo/scripts/checkout_submodules.py --platform qpg
    for module_path in  \
        third_party/nlfaultinjection \
        third_party/jsoncpp \
	    third_party/libwebsockets \
        third_party/editline \
        third_party/mbedtls \
        third_party/nlassert \
        third_party/nlio \
        third_party/nlunit-test \
        third_party/freertos \
        third_party/openthread \
        third_party/pigweed \
        third_party/perfetto \
        third_party/qpg_sdk
    do
        git submodule update --init --depth=1 -- "${module_path}"
    done
    log "submodules successfully initialized"
}

install_spake2p ()
{
    log "Installing spake2p build requirements"
    cd "${QMATTER_ROOT_PATH}/Components/ThirdParty/Matter/repo" || (echo chdir to matter repo failed; exit 1)

    # shellcheck source=/dev/null
    source "${SCRIPT_PATH}/build_install_spake2p.sh"
}

install_zap()
{
    ZAP_VERSION_FILE="${QMATTER_ROOT_PATH}/Components/ThirdParty/Matter/repo/scripts/setup/zap.json"
    ZAP_VERSION=$(grep -E "v[0-9]+\.[0-9]+\.[0-9]+-nightly" -o "$ZAP_VERSION_FILE" |head -n 1)
    echo "found version: ${ZAP_VERSION}"
    ZAP_INSTALL_PATH="/opt/zap-${ZAP_VERSION}"

    if test -e "${ZAP_INSTALL_PATH}"
    then
        return
    fi

    sudo mkdir -p "${ZAP_INSTALL_PATH}"
    cd "${ZAP_INSTALL_PATH}"
    sudo wget --progress=dot:giga "https://github.com/project-chip/zap/releases/download/${ZAP_VERSION}/zap-linux-x64.zip"
    sudo apt install unzip
    sudo unzip -o zap-linux-x64.zip
    sudo rm zap-linux-x64.zip
    # keep zap UI (don't delete it)
    sudo rm /usr/bin/zap-cli || true
    sudo ln -s "${ZAP_INSTALL_PATH}/zap-cli" /usr/bin/
    # additional symlink to do the version check
    sudo rm "/usr/bin/zap-cli-${ZAP_VERSION}" || true
    sudo ln -s "${ZAP_INSTALL_PATH}/zap-cli" "/usr/bin/zap-cli-${ZAP_VERSION}"
}
command -v sudo || (
    echo "Please enter your root password to install sudo."
    su -c 'apt-get update; apt-get install -y sudo'
)
command sudo apt-get update

for tool_name in  \
    git \
    clang \
    make \
    ninja \
    curl \
    wget
do
    command -v "$tool_name" || sudo apt-get install -y "${tool_name}" || sudo apt-get install -y "${tool_name}-build"
done

if ! check_installed_dependency node; then
    install_node_npm
fi

if check_installed_dependency arm-none-eabi-gcc
then
    if ! arm-none-eabi-gcc --version | grep -F "12.2.1 20230214" >/dev/null
    then
        echo "Invalid armgcc version detected"
        exit 1
    fi
else
    install_arm_gcc_emb
fi

if ! check_installed_dependency gn; then
    install_gn
fi

if ! check_installed_dependency npm; then
    install_zap_dependencies
fi

if ! check_installed_dependency rsync; then
    install_rsync
fi

setup_venv

setup_submodules

# requires setup_submodules
install_zap

if test ! -e /usr/bin/spake2p
then
    install_spake2p
fi
