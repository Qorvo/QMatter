# Crystal Procurement Tool

Qorvo Crystal Procurement tool is a tool that can be used to check if the electrical parameters, given in the crystal specification, can meet Qorvo procurement requirements. As the 2.4 GHz carrier frequency of Qorvo communication-controllers (“chips”) is also derived from the external
crystal (i.e., 32 MHz or 16 MHz for older generations), the device will not be able to meet frequency offset specifications if the crystal is not meeting our requirements.

IEEE 802.15.4 PHY specifications require a maximum carrier frequency tolerance of ± 40 ppm under different operating temperatures and over product lifetime. This limit includes the offsets introduced by the initial frequency tolerance at 25 ºC, frequency stability over temperature range, aging over lifetime and frequency variations due to variations in the load capacitance.


This tool will also examine if the loop gain is sufficient to oscillate reliably without overdriving the crystal, even in corner cases. Therefore, a reliable and accurate carrier frequency can be guaranteed.

This tool can only be used on a Windows based machine and can be found after unzipping the file [GP_P414_TR_16554_Xtal_Procurement_Tool_Python_Version.zip](GP_P414_TR_16554_Xtal_Procurement_Tool_Python_Version.zip) that you can find in this folder.

More detailed information how to use the tool can be found in this [user manual](GP_P414_UM_16630_Crystal_Procurement_Tool.pdf).

To get a detailed description how the measurement procedure should like can be found in the document [Measuring 32 MHz Crystal Oscillator Frequency Compensation over Temperature Range](GP_P008_AN_20183_Measuring_32MHz_Oscillator_Freq_Compensation_over_Temp_Range.pdf)

To get a description and technical background how to configure the Frequency Offset Correction Algorithm, refer to the document [Configuring the Frequency Offset Correction Algorithm](GP_P008_AN_20182_Configuring_Frequency_Offset_Correction_Algorithm.pdf)
