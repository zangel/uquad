#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=mkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/uQuad.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/uQuad.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../../../../../../../../../Yosemite/Users/zangel/microchip/mla/v2014_07_22/framework/usb/src/usb_device.c /Volumes/Yosemite/Users/zangel/microchip/mla/v2014_07_22/framework/usb/src/usb_device_generic.c main.c system.c comm.c app.c usb_decriptors.c leds.c motors.c /Users/zangel/microchip/mla/v2014_07_22/framework/usb/sensors.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/930677255/usb_device.p1 ${OBJECTDIR}/_ext/1021381455/usb_device_generic.p1 ${OBJECTDIR}/main.p1 ${OBJECTDIR}/system.p1 ${OBJECTDIR}/comm.p1 ${OBJECTDIR}/app.p1 ${OBJECTDIR}/usb_decriptors.p1 ${OBJECTDIR}/leds.p1 ${OBJECTDIR}/motors.p1 ${OBJECTDIR}/_ext/2085033782/sensors.p1
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/930677255/usb_device.p1.d ${OBJECTDIR}/_ext/1021381455/usb_device_generic.p1.d ${OBJECTDIR}/main.p1.d ${OBJECTDIR}/system.p1.d ${OBJECTDIR}/comm.p1.d ${OBJECTDIR}/app.p1.d ${OBJECTDIR}/usb_decriptors.p1.d ${OBJECTDIR}/leds.p1.d ${OBJECTDIR}/motors.p1.d ${OBJECTDIR}/_ext/2085033782/sensors.p1.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/930677255/usb_device.p1 ${OBJECTDIR}/_ext/1021381455/usb_device_generic.p1 ${OBJECTDIR}/main.p1 ${OBJECTDIR}/system.p1 ${OBJECTDIR}/comm.p1 ${OBJECTDIR}/app.p1 ${OBJECTDIR}/usb_decriptors.p1 ${OBJECTDIR}/leds.p1 ${OBJECTDIR}/motors.p1 ${OBJECTDIR}/_ext/2085033782/sensors.p1

# Source Files
SOURCEFILES=../../../../../../../../../Yosemite/Users/zangel/microchip/mla/v2014_07_22/framework/usb/src/usb_device.c /Volumes/Yosemite/Users/zangel/microchip/mla/v2014_07_22/framework/usb/src/usb_device_generic.c main.c system.c comm.c app.c usb_decriptors.c leds.c motors.c /Users/zangel/microchip/mla/v2014_07_22/framework/usb/sensors.c


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/uQuad.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=18F4550
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/930677255/usb_device.p1: ../../../../../../../../../Yosemite/Users/zangel/microchip/mla/v2014_07_22/framework/usb/src/usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/930677255 
	@${RM} ${OBJECTDIR}/_ext/930677255/usb_device.p1.d 
	@${RM} ${OBJECTDIR}/_ext/930677255/usb_device.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=none  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/_ext/930677255/usb_device.p1  ../../../../../../../../../Yosemite/Users/zangel/microchip/mla/v2014_07_22/framework/usb/src/usb_device.c 
	@-${MV} ${OBJECTDIR}/_ext/930677255/usb_device.d ${OBJECTDIR}/_ext/930677255/usb_device.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/930677255/usb_device.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/1021381455/usb_device_generic.p1: /Volumes/Yosemite/Users/zangel/microchip/mla/v2014_07_22/framework/usb/src/usb_device_generic.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1021381455 
	@${RM} ${OBJECTDIR}/_ext/1021381455/usb_device_generic.p1.d 
	@${RM} ${OBJECTDIR}/_ext/1021381455/usb_device_generic.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=none  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/_ext/1021381455/usb_device_generic.p1  /Volumes/Yosemite/Users/zangel/microchip/mla/v2014_07_22/framework/usb/src/usb_device_generic.c 
	@-${MV} ${OBJECTDIR}/_ext/1021381455/usb_device_generic.d ${OBJECTDIR}/_ext/1021381455/usb_device_generic.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/1021381455/usb_device_generic.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/main.p1: main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/main.p1.d 
	@${RM} ${OBJECTDIR}/main.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=none  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/main.p1  main.c 
	@-${MV} ${OBJECTDIR}/main.d ${OBJECTDIR}/main.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/main.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/system.p1: system.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/system.p1.d 
	@${RM} ${OBJECTDIR}/system.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=none  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/system.p1  system.c 
	@-${MV} ${OBJECTDIR}/system.d ${OBJECTDIR}/system.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/system.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/comm.p1: comm.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/comm.p1.d 
	@${RM} ${OBJECTDIR}/comm.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=none  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/comm.p1  comm.c 
	@-${MV} ${OBJECTDIR}/comm.d ${OBJECTDIR}/comm.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/comm.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/app.p1: app.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/app.p1.d 
	@${RM} ${OBJECTDIR}/app.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=none  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/app.p1  app.c 
	@-${MV} ${OBJECTDIR}/app.d ${OBJECTDIR}/app.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/app.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/usb_decriptors.p1: usb_decriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/usb_decriptors.p1.d 
	@${RM} ${OBJECTDIR}/usb_decriptors.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=none  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/usb_decriptors.p1  usb_decriptors.c 
	@-${MV} ${OBJECTDIR}/usb_decriptors.d ${OBJECTDIR}/usb_decriptors.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/usb_decriptors.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/leds.p1: leds.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/leds.p1.d 
	@${RM} ${OBJECTDIR}/leds.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=none  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/leds.p1  leds.c 
	@-${MV} ${OBJECTDIR}/leds.d ${OBJECTDIR}/leds.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/leds.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/motors.p1: motors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/motors.p1.d 
	@${RM} ${OBJECTDIR}/motors.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=none  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/motors.p1  motors.c 
	@-${MV} ${OBJECTDIR}/motors.d ${OBJECTDIR}/motors.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/motors.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/2085033782/sensors.p1: /Users/zangel/microchip/mla/v2014_07_22/framework/usb/sensors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2085033782 
	@${RM} ${OBJECTDIR}/_ext/2085033782/sensors.p1.d 
	@${RM} ${OBJECTDIR}/_ext/2085033782/sensors.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  -D__DEBUG=1 --debugger=none  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/_ext/2085033782/sensors.p1  /Users/zangel/microchip/mla/v2014_07_22/framework/usb/sensors.c 
	@-${MV} ${OBJECTDIR}/_ext/2085033782/sensors.d ${OBJECTDIR}/_ext/2085033782/sensors.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/2085033782/sensors.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
else
${OBJECTDIR}/_ext/930677255/usb_device.p1: ../../../../../../../../../Yosemite/Users/zangel/microchip/mla/v2014_07_22/framework/usb/src/usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/930677255 
	@${RM} ${OBJECTDIR}/_ext/930677255/usb_device.p1.d 
	@${RM} ${OBJECTDIR}/_ext/930677255/usb_device.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/_ext/930677255/usb_device.p1  ../../../../../../../../../Yosemite/Users/zangel/microchip/mla/v2014_07_22/framework/usb/src/usb_device.c 
	@-${MV} ${OBJECTDIR}/_ext/930677255/usb_device.d ${OBJECTDIR}/_ext/930677255/usb_device.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/930677255/usb_device.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/1021381455/usb_device_generic.p1: /Volumes/Yosemite/Users/zangel/microchip/mla/v2014_07_22/framework/usb/src/usb_device_generic.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1021381455 
	@${RM} ${OBJECTDIR}/_ext/1021381455/usb_device_generic.p1.d 
	@${RM} ${OBJECTDIR}/_ext/1021381455/usb_device_generic.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/_ext/1021381455/usb_device_generic.p1  /Volumes/Yosemite/Users/zangel/microchip/mla/v2014_07_22/framework/usb/src/usb_device_generic.c 
	@-${MV} ${OBJECTDIR}/_ext/1021381455/usb_device_generic.d ${OBJECTDIR}/_ext/1021381455/usb_device_generic.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/1021381455/usb_device_generic.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/main.p1: main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/main.p1.d 
	@${RM} ${OBJECTDIR}/main.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/main.p1  main.c 
	@-${MV} ${OBJECTDIR}/main.d ${OBJECTDIR}/main.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/main.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/system.p1: system.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/system.p1.d 
	@${RM} ${OBJECTDIR}/system.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/system.p1  system.c 
	@-${MV} ${OBJECTDIR}/system.d ${OBJECTDIR}/system.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/system.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/comm.p1: comm.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/comm.p1.d 
	@${RM} ${OBJECTDIR}/comm.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/comm.p1  comm.c 
	@-${MV} ${OBJECTDIR}/comm.d ${OBJECTDIR}/comm.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/comm.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/app.p1: app.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/app.p1.d 
	@${RM} ${OBJECTDIR}/app.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/app.p1  app.c 
	@-${MV} ${OBJECTDIR}/app.d ${OBJECTDIR}/app.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/app.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/usb_decriptors.p1: usb_decriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/usb_decriptors.p1.d 
	@${RM} ${OBJECTDIR}/usb_decriptors.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/usb_decriptors.p1  usb_decriptors.c 
	@-${MV} ${OBJECTDIR}/usb_decriptors.d ${OBJECTDIR}/usb_decriptors.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/usb_decriptors.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/leds.p1: leds.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/leds.p1.d 
	@${RM} ${OBJECTDIR}/leds.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/leds.p1  leds.c 
	@-${MV} ${OBJECTDIR}/leds.d ${OBJECTDIR}/leds.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/leds.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/motors.p1: motors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR} 
	@${RM} ${OBJECTDIR}/motors.p1.d 
	@${RM} ${OBJECTDIR}/motors.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/motors.p1  motors.c 
	@-${MV} ${OBJECTDIR}/motors.d ${OBJECTDIR}/motors.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/motors.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
${OBJECTDIR}/_ext/2085033782/sensors.p1: /Users/zangel/microchip/mla/v2014_07_22/framework/usb/sensors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2085033782 
	@${RM} ${OBJECTDIR}/_ext/2085033782/sensors.p1.d 
	@${RM} ${OBJECTDIR}/_ext/2085033782/sensors.p1 
	${MP_CC} --pass1 $(MP_EXTRA_CC_PRE) --chip=$(MP_PROCESSOR_OPTION) -Q -G  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"    -o${OBJECTDIR}/_ext/2085033782/sensors.p1  /Users/zangel/microchip/mla/v2014_07_22/framework/usb/sensors.c 
	@-${MV} ${OBJECTDIR}/_ext/2085033782/sensors.d ${OBJECTDIR}/_ext/2085033782/sensors.p1.d 
	@${FIXDEPS} ${OBJECTDIR}/_ext/2085033782/sensors.p1.d $(SILENT) -rsi ${MP_CC_DIR}../  
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/uQuad.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) --chip=$(MP_PROCESSOR_OPTION) -G -mdist/${CND_CONF}/${IMAGE_TYPE}/uQuad.X.${IMAGE_TYPE}.map  -D__DEBUG=1 --debugger=none  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"        -odist/${CND_CONF}/${IMAGE_TYPE}/uQuad.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}     
	@${RM} dist/${CND_CONF}/${IMAGE_TYPE}/uQuad.X.${IMAGE_TYPE}.hex 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/uQuad.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) --chip=$(MP_PROCESSOR_OPTION) -G -mdist/${CND_CONF}/${IMAGE_TYPE}/uQuad.X.${IMAGE_TYPE}.map  --double=32 --float=32 --emi=wordwrite --rom=default,-0-FFF,-1006-1007,-1016-1017 --opt=default,+asm,+asmfile,+speed,-space,-debug --addrqual=ignore --mode=pro -P -N255 -I"." -I"../../sw/src" -I"/Users/zangel/microchip/mla/v2014_07_22/framework" --warn=0 --asmlist --summary=default,-psect,-class,+mem,-hex,-file --codeoffset=0x1000 --output=default,-inhx032 --runtime=default,+clear,+init,-keep,-no_startup,-download,+config,+clib,+plib --output=-mcof,+elf "--errformat=%f:%l: error: %s" "--warnformat=%f:%l: warning: %s" "--msgformat=%f:%l: advisory: %s"     -odist/${CND_CONF}/${IMAGE_TYPE}/uQuad.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}     
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell "${PATH_TO_IDE_BIN}"mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
