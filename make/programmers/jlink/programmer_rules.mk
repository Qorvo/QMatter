# Define directories
DEVICE=QPG6105
DIRSTLINK = /opt/SEGGER/JLink
# Define programs and commands.
JLINK_PROGRAMMER = $(DIRSTLINK)/JLinkExe
JLINK_GDBSERVER = $(DIRSTLINK)/JLinkGDBServer

#Python script to generate JLINK script
SCRIPT_NAME = $(ENV_PATH)/make/programmers/$(PROGRAMMER)/temp.jlink
ERASE_SCRIPT = $(ENV_PATH)/make/programmers/$(PROGRAMMER)/erase.jlink
RESET_SCRIPT = $(ENV_PATH)/make/programmers/$(PROGRAMMER)/reset.jlink

program: $(APPFILE:.$(PROGRAM_EXT)=.hex) postbuild-app
	@$(ECHO) Generate Commander Script
	echo speed 4000 >"${SCRIPT_NAME}"
	echo si swd >>"${SCRIPT_NAME}"
	echo erase >>"${SCRIPT_NAME}"
	echo loadfile $(APPFILE) >>"${SCRIPT_NAME}"
	echo reset >>"${SCRIPT_NAME}"
	echo exit >>"${SCRIPT_NAME}"
	@$(ECHO) Programming $<
	@$(JLINK_PROGRAMMER) -Device $(DEVICE) -nogui 1 -CommanderScript $(SCRIPT_NAME)
	@$(ECHO) Remove commander script
	rm $(SCRIPT_NAME)

erase:
	@$(ECHO) "Erase All Flash"
	@$(JLINK_PROGRAMMER) -Device $(DEVICE) -if SWD -speed 4000 -autoconnect 1 -CommanderScript $(ERASE_SCRIPT)

reset:
	@$(ECHO) "Resetting"
	@$(JLINK_PROGRAMMER) -Device $(DEVICE) -if SWD -speed 4000 -autoconnect 1 -CommanderScript $(RESET_SCRIPT)

gdbserver:
	@$(JLINK_GDBSERVER) -Device $(DEVICE) -if SWD

help::
	$(ECHO) "$(BWhite)--- Programmer --- $(Color_Off)\n\
program           - Program with \n$(APPFILE)\n\
reset             - Reset \n\
erase             - Flash erase Kx\n\
