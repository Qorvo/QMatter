# Qorvo MAC related files
base_files = ["gpRadio", "gpHal_MAC", "gpHal_Scan", "gpHal_DP", "gpMacDispatcher", "gpMacCore", "gpRxArbiter"]

default = {"Components/Qorvo/HAL_RF": {"gphal": ["gpHal_MAC", "gpHal_Scan", "gpHal_DP"], },
           "Components/Qorvo/802_15_4": {"gpMacDispatcher": [],
                                         "gpMacCore": [],
                                         "gpRxArbiter": [], },
           },
