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

"$PYTHON" "${SCRIPT_DIR}"/../../../Tools/Ota/generate_ota_img.py --chip_config_header "${SCRIPT_DIR}"/../../../Applications/Matter/light/include/CHIPProjectConfig.h --chip_root "${SCRIPT_DIR}"/../../../Components/ThirdParty/Matter/repo --compression none --factory_data_config "${SCRIPT_DIR}"/../../../Tools/FactoryData/Credentials/test_light.factory_data_config --in_file "${SCRIPT_DIR}"/../../../Work/light_qpg6105_ext_flash_production/light_qpg6105_ext_flash_production.hex --out_file "${SCRIPT_DIR}"/../../../Work/light_qpg6105_ext_flash_production/light_qpg6105_ext_flash_production.ota --pem_file_path "${SCRIPT_DIR}"/../../../Tools/Ota/example_private_key.pem.example --pem_password test1234 --prune_only --sign

"$PYTHON" "${SCRIPT_DIR}"/../../../Tools/MemoryOverview/memoryoverview.py --logfile "${SCRIPT_DIR}"/../../../Work/light_qpg6105_ext_flash_production/light_qpg6105_ext_flash_production.memoryoverview --only-this "${SCRIPT_DIR}"/../../../Work/light_qpg6105_ext_flash_production/light_qpg6105_ext_flash_production.map

"$PYTHON" "${SCRIPT_DIR}"/../../../Tools/SecureBoot/generate_programmer.py  --enroll-key 8 --enroll-key 9 --enrollment  --enrollment-field AppID ProjectX --factorydata-address 0x4004800 --gpproductid QPG6105 --hexlink "${SCRIPT_DIR}"/../../../Work/light_qpg6105_ext_flash_production/light_qpg6105_ext_flash_production.hex --input "${SCRIPT_DIR}"/../../../Tools/SecureBoot/templates/programmer_info_k8e.tmpl --output "${SCRIPT_DIR}"/../../../Work/light_qpg6105_ext_flash_production/light_qpg6105_ext_flash_production.xml --secure-boot-type 17 --server-address http://localhost:8000 --user-public-key-path "${SCRIPT_DIR}"/../../..//Tools/SecureBoot/public_key.pem
