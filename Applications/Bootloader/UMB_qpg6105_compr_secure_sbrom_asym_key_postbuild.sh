#!/bin/sh

set -e

SCRIPT_DIR="$(dirname "$(realpath "$0")")"

# determine python interpreter path
if [ -f "`which python3`" ]; then
    PYTHON="`which python3`"
else
    if [ -f "`which python`" ]; then
        PYTHON="`which python`"
    else
        echo "No python interpreter found."
        exit 1
    fi
fi

# check python interpreter version
PYTHON_VERSION=`"${PYTHON}" --version | cut -d\  -f 2`
PYTHON_MAJOR=`echo ${PYTHON_VERSION} | cut -d. -f 1`
PYTHON_MINOR=`echo ${PYTHON_VERSION} | cut -d. -f 2`

if [ ! \( ${PYTHON_MAJOR} -eq 3 -o ${PYTHON_MINOR} -lt 6 \) ]; then
    echo "Python 3.x (at least 3.6) is required, you have ${PYTHON_VERSION}."
fi

# seed random source

RANDOM=`date +%s`$$

# set variables

OLD_CWD=`pwd`
TEMP_DIR=/tmp
UNIQUE_ID=${RANDOM}
PROJECT_PATH="$1"
TARGET_PATH="$2"
TARGET_BASEPATH="`echo ${TARGET_PATH} | sed -E 's/\.[^.]+$//g'`"
TARGET_BASENAME="`basename ${TARGET_BASEPATH}`"
TARGET_DIR="`dirname ${TARGET_BASEPATH}`"

trap 'cd ${OLD_CWD}' EXIT

# Build steps

"$PYTHON" "${SCRIPT_DIR}"/../../Tools/SecureBoot/secureboot_sign_hex.py --application_pem "${SCRIPT_DIR}"/../../Tools/Ota/example_private_key.pem.example --application_pem_password test1234 --asymkey "${SCRIPT_DIR}"/../..//Tools/SecureBoot/private_key.pem --input_hex "${SCRIPT_DIR}"/../../Work/UMB_qpg6105_compr_secure_sbrom_asym_key/UMB_qpg6105_compr_secure_sbrom_asym_key.hex --pem_password placeholdersecurebootpassword --section1 0x1800:0xFFFFFFFF --start_addr_area 0x04000000 --write_application_public_key 0x1800

"$PYTHON" "${SCRIPT_DIR}"/../../Tools/Ota/generateRMAToken.py --mac_addr 00:15:5f:00:00:00:0f:55 --pem "${SCRIPT_DIR}"/../../Tools/Ota/example_private_key.pem.example --pem_password test1234 --rma_token_area_address 0x4005800 --token 0x80DF55BA
