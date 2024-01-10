#!/bin/bash

SCRIPT_PATH="$(dirname "$(realpath "${BASH_SOURCE[0]}")")"
VENV_PATH=$(realpath "${SCRIPT_PATH}/../.python_venv")
MATTER_REPO_SUBPATH=Components/ThirdParty/Matter/repo

/proc/self/exe --version 2>/dev/null | grep -q 'GNU bash' ||  (\
    echo "!!!!! This is a BASH script !!!!!"; \
    echo "The shell you are running is $(/proc/self/exe --version)"; \
    echo "Please start bash first by running the 'bash' command"; \
    echo "Press Ctrl-C to abort"; \
    read -r \
)

log() {
    echo "========= ${1} ============"
}
activate_sh_failure() {
    echo "========= ${1} ============"
    export ACTIVATE_SH_FAILURE=true
}


DEFAULT_TOOLCHAIN_DIR=/opt/TOOL_ARMGCCEMB/arm-gnu-toolchain-12.2.mpacbti-rel1-x86_64-arm-none-eabi

export PATH=$PATH:$DEFAULT_TOOLCHAIN_DIR/bin:${SCRIPT_PATH}/../Tools/FactoryData
export MAKEFLAGS=-s

if bash "${SCRIPT_PATH}"/bootstrap.sh
then
    export TOOLCHAIN="$DEFAULT_TOOLCHAIN_DIR"

    QMATTER_ROOT_PATH=$(realpath "${SCRIPT_PATH}/..")
    export CHIP_ROOT="${QMATTER_ROOT_PATH}/${MATTER_REPO_SUBPATH}"
    ZAP_VERSION_FILE="${QMATTER_ROOT_PATH}/Components/ThirdParty/Matter/repo/scripts/setup/zap.json"
    ZAP_VERSION=$(grep -E "v[0-9]+\.[0-9]+\.[0-9]+-nightly" -o "$ZAP_VERSION_FILE" |head -n 1)
    # ZAP_INSTALL_PATH is used by gn examples/ builds
    export ZAP_INSTALL_PATH="/opt/zap-${ZAP_VERSION}"

    # shellcheck source=/dev/null
    source "${VENV_PATH}"/bin/activate || activate_sh_failure "$(realpath "${BASH_SOURCE[0]}") FAILED"
else
    activate_sh_failure "$(realpath "${BASH_SOURCE[0]}") FAILED"
fi
