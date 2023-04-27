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

"$PYTHON" "${SCRIPT_DIR}"/../../../Tools/Ota/setRollbackCounter.py --hex ${TARGET_BASEPATH}.hex --license_address 0x4006000 --rollback_counter 0x0

cp "${TARGET_BASEPATH}.hex" "${TARGET_BASEPATH}-unsigned.hex"

"$PYTHON" "${SCRIPT_DIR}"/../../../Tools/Ota/signFirmware.py --hex ${TARGET_BASEPATH}.hex --license_offset 0x6000 --pem "${SCRIPT_DIR}"/../../../Tools/Ota/example_private_key.pem.example --pem_password test1234 --section1 0x006100:0xffffffff --section2 0x800:0x1000 --set_bootloader_loaded --start_addr_area 0x4000000

cp "${TARGET_BASEPATH}.hex" "${TARGET_BASEPATH}.out"
