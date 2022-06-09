#!/bin/sh

set -e
BASEDIR="$(dirname "$(realpath "$0")")"

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

trap 'cd ${OLD_CWD}' EXIT

# Build steps

"$PYTHON" "${BASEDIR}"/../../../Tools/Ota/generate_ota_img.py --chip_config_header "${BASEDIR}"/../../../Applications/Matter/light/include/CHIPProjectConfig.h --chip_root "${BASEDIR}"/../../../Components/ThirdParty/Matter/repo --compression lzma --in_file "${BASEDIR}"/../../../Work/light_qpg6105/light_qpg6105.hex --out_file "${BASEDIR}"/../../../Work/light_qpg6105/light_qpg6105.ota --pem_file_path "${BASEDIR}"/../../../Tools/Ota/example_private_key.pem.example --pem_password test1234 --sign
