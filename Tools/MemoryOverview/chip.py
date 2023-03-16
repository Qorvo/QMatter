try:
    from gppy.tools.memory.categorization.generic import Categorization
    import gppy.tools.memory.categorization.feature.freertos as FreeRTOS
    import gppy.tools.memory.categorization.feature.security as Security
except ImportError:
    try:
        from generic import Categorization
        import freertos as FreeRTOS
        import security as Security
    except ImportError:
        print("Using outside of Env ?")


class CategorizationCHIP(Categorization):
    order = [
        "Thread", "CHIP", "Thread/CHIP glue",
        "BLE Base", "BLE", "Mesh",
    ] + Categorization.order

    glue_files = ["qvCHIP"]
    application_files = ["chip-", "lock-common", "lighting-common"]

    # Adding CHIP prefixes after SDK compilation
    freertos_files = FreeRTOS.base_files
    freertos_files = ["sdk." + file_name for file_name in freertos_files]

    security_files = Security.base_files + Security.silex_crypto_soc_files + Security.mbed_tls_files
    security_files = ["sdk." + file_name for file_name in security_files]

    Categorization.files.update({
        "Security": security_files,
        "RTOS": ["freertos", "gpFreeRTOS"] + freertos_files,
        "CHIP": [],
        "Thread/CHIP glue": glue_files,
        "Application": application_files,
    })

    Categorization.default.update({
        "CHIP": {
            'P236_CHIP': {},
        },
        "Thread/CHIP glue": {
        },
    })

    Categorization.archives.update({
        "Thread/CHIP glue": {"libOpenThread_qpg61": [],
                             "libOpenThreadQorvoGlue": [],
                             },
    })

    Categorization.combined_archives += ["libCHIP", "libCHIP_qpg61", "libCHIP_qpg6105", "libMatter"]

    Categorization.ignore_folders += ["lib"]  # P236_qvCHIP library
