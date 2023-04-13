"""Description of files <> category sorting - used in memory usage scripting"""

try:
    import gppy.tools.memory.categorization.feature.ble as Ble
    import gppy.tools.memory.categorization.feature.freertos as FreeRTOS
    import gppy.tools.memory.categorization.feature.mac as Mac
    import gppy.tools.memory.categorization.feature.security as Security
except ImportError:
    try:
        import ble as Ble
        import freertos as FreeRTOS
        import mac as Mac
        import security as Security
    except ImportError:
        print("Using outside of Env ?")


class Categorization(object):
    order = [
        "MAC",
        "Debug", "Library", "RTOS", "Security",
        "Base",  # Left-over category currently - should be merged with more specific ones
        "Bootloader",
        "Application",
    ]

    files = {
        "Thread": ["libopenthread"],
        "Security": Security.base_files + Security.silex_crypto_soc_files + Security.mbed_tls_files,
        "BLE Base": Ble.base_files,
        "BLE": ["BleModule"] + Ble.arm_cordio_files,
        "MAC": Mac.base_files,
        "Mesh": [],
        "Debug": ["gpLog", "gpAssert"],
        "Library": ["crt"],
        "RTOS": ["freertos", "gpFreeRTOS"] + FreeRTOS.base_files,
        "Bootloader": ["gpUpgrade", "gpOta", "gpSecureBoot", "lzma"],
        "Base": ["gp", "hal", "default", "handlers", "iar", "ivt", "rom_access", "dig_hal"],
    }

    # Dict[str category, Dict[str moduleName , Dict[str componentName, List[str filematch] ] ]
    default = {
        "Thread": {
        },
        "Security": Security.default,
        "BLE Base": Ble.base_default,
        "BLE": Ble.default,
        "MAC": Mac.default,
        "Mesh": Ble.mesh_default,
        "Debug": {"Components/Qorvo/OS": {"gpLog": [],
                                          "gpAssert": [], },
                  },
        "Library": {
        },
        "RTOS": FreeRTOS.default,
        "Base": {
        },
        "Bootloader": {
        },
        "Application": {
            'P236_CHIP': {"app": []},
            'P345_Matter_DK_Endnodes': {"matter": []},
            'src/app/clusters': {"clusters": []},
        },
    }

    # Dict[str category, Dict[str archivename_start , List[str objectname_start] ]
    archives = {
        "Thread": {"libopenthread": [],
                   },
        "Security": {"libmbedcrypto": [],
                     "libmbedtls": [],
                     },
        "BLE Base": {
        },
        "BLE": {
        },
        "MAC": {
        },
        "Mesh": {
        },
        "Debug": {
        },
        "Library": {"dl7M_tln.a": [],
                    "dlpp7M_tl_ne.a": [],
                    "m7M_tl.a": [],
                    "rt7M_tl.a": [],
                    "shb_l.a": [],
                    "libc.a": [],
                    "libc_nano.a": [],
                    "libgcc.a": [],
                    "libstdc++_nano.a": [],
                    "libm.a": [],
                    "iar_cortexM4l_math.a": [],
                    },
        "RTOS": {
        },
        "Bootloader": {"libBootloader": [],
                       },
        "Base": {"libQorvo": [],
                 },
        "CHIP": {"libCHIP": [],
                 "libMatter": [],
                 },
        "Application": {
        },
    }

    # objects of libraries in combined_archives are passed through the categorisation of categories_files
    combined_archives = ["libQorvo"]

    # Folders that are ignored when determining the module and component from a path
    ignore_folders = ["src", "patch", "k8c", "k8a", "comps", "apps", "gen", "gp", "code", "bin"]
