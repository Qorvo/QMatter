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
    application_files = [
        "chip-", "lock-common", "lighting-common",
        "Accessors",
        "AppTask",
        "ColorFormat",
        "DataModelHandler",
        "DeviceInfoProviderImpl",
        "LightingManager",
        "ZclCallbacks",
        "attribute-storage",
        "attribute-table",
        "binding-table",
        "ember-compatibility-functions"
        "ember-print",
        "generic-callback-stubs",
        "main",
        "message",
        "ota",
        "powercycle_counting",
        "privilege-storage",
        "util",
        "ember-print",
        "ember-compatibility-functions",
    ]

    # List of all .cpp in /src/app/clusters
    cluster_files = [
        "ExtensionFieldSetsImpl",
        "scenes",
        "SceneTableImpl",
        "descriptor",
        "time-format-localization-server",
        "power-source-server",
        "administrator-commissioning-server",
        "client-monitoring-server",
        "diagnostic-logs-server",
        "channel-server",
        "application-launcher-server",
        "content-launch-server",
        "fan-control-server",
        "thermostat-user-interface-configuration-server",
        "general-commissioning-server",
        "mode-select-server",
        "switch-server",
        "low-power-server",
        "thermostat-server",
        "wifi-network-diagnostics-server",
        "user-label-server",
        "color-control-server",
        "identify-server",
        "level-control",
        "door-lock-server-callback",
        "door-lock-server",
        "application-basic-server",
        "basic-information",
        "localization-configuration-server",
        "media-playback-server",
        "group-key-mgmt-server",
        "general-diagnostics-server",
        "GenericFaultTestEventTriggerDelegate",
        "on-off-server",
        "pump-configuration-and-control-client",
        "window-covering-server",
        "pump-configuration-and-control-server",
        "ExtendedOTARequestorDriver",
        "DefaultOTARequestorDriver",
        "OTATestEventTriggerDelegate",
        "ota-requestor-server",
        "DefaultOTARequestorStorage",
        "BDXDownloader",
        "DefaultOTARequestor",
        "keypad-input-server",
        "account-login-server",
        "occupancy-sensor-server",
        "operational-credentials-server",
        "groups-server",
        "access-control-server",
        "barrier-control-server",
        "media-input-server",
        "thread-network-diagnostics-server",
        "fault-injection-server",
        "test-cluster-server",
        "ethernet-network-diagnostics-server",
        "software-diagnostics-server",
        "wake-on-lan-server",
        "target-navigator-server",
        "fixed-label-server",
        "network-commissioning",
        "thermostat-client",
        "BindingManager",
        "PendingNotificationMap",
        "bindings",
        "bridged-device-basic-information-server",
        "audio-output-server",
        "ota-provider",
        "power-source-configuration-server",
    ]

    # Adding CHIP prefixes after SDK compilation
    freertos_files = FreeRTOS.base_files
    freertos_files += ["sdk." + file_name for file_name in freertos_files]

    security_files = Security.base_files + Security.silex_crypto_soc_files + Security.mbed_tls_files
    security_files += ["sdk." + file_name for file_name in security_files]

    Categorization.files.update({
        "Security": security_files,
        "RTOS": ["freertos", "gpFreeRTOS"] + freertos_files,
        "CHIP": [],
        "Thread/CHIP glue": glue_files,
        "Application": application_files + cluster_files,
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
        "Application": {"libZapGenerated": [], },
    })

    Categorization.combined_archives += ["libCHIP", "libCHIP_qpg61", "libCHIP_qpg6105", "libMatter"]

    Categorization.ignore_folders += ["lib"]  # P236_qvCHIP library
