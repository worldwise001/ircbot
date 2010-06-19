#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_CONF=Debug
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/ircq.o \
	${OBJECTDIR}/ircfunc.o \
	${OBJECTDIR}/irclist.o \
	${OBJECTDIR}/ircsock.o \
	${OBJECTDIR}/ircenv.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/irc.o

# C Compiler Flags
CFLAGS=-rdynamic

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lpthread -ldl

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Debug.mk dist/Debug/GNU-Linux-x86/libcircle

dist/Debug/GNU-Linux-x86/libcircle: ${OBJECTFILES}
	${MKDIR} -p dist/Debug/GNU-Linux-x86
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libcircle ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/ircq.o: nbproject/Makefile-${CND_CONF}.mk ircq.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/ircq.o ircq.c

${OBJECTDIR}/ircfunc.o: nbproject/Makefile-${CND_CONF}.mk ircfunc.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/ircfunc.o ircfunc.c

${OBJECTDIR}/irclist.o: nbproject/Makefile-${CND_CONF}.mk irclist.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/irclist.o irclist.c

${OBJECTDIR}/ircsock.o: nbproject/Makefile-${CND_CONF}.mk ircsock.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/ircsock.o ircsock.c

${OBJECTDIR}/ircenv.o: nbproject/Makefile-${CND_CONF}.mk ircenv.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/ircenv.o ircenv.c

${OBJECTDIR}/main.o: nbproject/Makefile-${CND_CONF}.mk main.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/main.o main.c

${OBJECTDIR}/irc.o: nbproject/Makefile-${CND_CONF}.mk irc.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/irc.o irc.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Debug
	${RM} dist/Debug/GNU-Linux-x86/libcircle

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
