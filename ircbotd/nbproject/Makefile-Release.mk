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
CND_CONF=Release
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/parse.o \
	${OBJECTDIR}/socket.o \
	${OBJECTDIR}/conf.o \
	${OBJECTDIR}/llist.o \
	${OBJECTDIR}/lib.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/admin.o \
	${OBJECTDIR}/io.o \
	${OBJECTDIR}/child.o \
	${OBJECTDIR}/module.o \
	${OBJECTDIR}/ircfunc.o

# C Compiler Flags
CFLAGS=

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
	${MAKE}  -f nbproject/Makefile-Release.mk dist/Release/GNU-Linux-x86/${EXECUTABLE}

dist/Release/GNU-Linux-x86/${EXECUTABLE}: ${OBJECTFILES}
	${MKDIR} -p dist/Release/GNU-Linux-x86
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/${EXECUTABLE} ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/parse.o: nbproject/Makefile-${CND_CONF}.mk parse.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -O2 -w -MMD -MP -MF $@.d -o ${OBJECTDIR}/parse.o parse.c

${OBJECTDIR}/socket.o: nbproject/Makefile-${CND_CONF}.mk socket.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -O2 -w -MMD -MP -MF $@.d -o ${OBJECTDIR}/socket.o socket.c

${OBJECTDIR}/conf.o: nbproject/Makefile-${CND_CONF}.mk conf.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -O2 -w -MMD -MP -MF $@.d -o ${OBJECTDIR}/conf.o conf.c

${OBJECTDIR}/llist.o: nbproject/Makefile-${CND_CONF}.mk llist.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -O2 -w -MMD -MP -MF $@.d -o ${OBJECTDIR}/llist.o llist.c

${OBJECTDIR}/lib.o: nbproject/Makefile-${CND_CONF}.mk lib.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -O2 -w -MMD -MP -MF $@.d -o ${OBJECTDIR}/lib.o lib.c

${OBJECTDIR}/main.o: nbproject/Makefile-${CND_CONF}.mk main.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -O2 -w -MMD -MP -MF $@.d -o ${OBJECTDIR}/main.o main.c

${OBJECTDIR}/admin.o: nbproject/Makefile-${CND_CONF}.mk admin.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -O2 -w -MMD -MP -MF $@.d -o ${OBJECTDIR}/admin.o admin.c

${OBJECTDIR}/io.o: nbproject/Makefile-${CND_CONF}.mk io.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -O2 -w -MMD -MP -MF $@.d -o ${OBJECTDIR}/io.o io.c

${OBJECTDIR}/child.o: nbproject/Makefile-${CND_CONF}.mk child.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -O2 -w -MMD -MP -MF $@.d -o ${OBJECTDIR}/child.o child.c

${OBJECTDIR}/module.o: nbproject/Makefile-${CND_CONF}.mk module.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -O2 -w -MMD -MP -MF $@.d -o ${OBJECTDIR}/module.o module.c

${OBJECTDIR}/ircfunc.o: nbproject/Makefile-${CND_CONF}.mk ircfunc.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) -O2 -w -MMD -MP -MF $@.d -o ${OBJECTDIR}/ircfunc.o ircfunc.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Release
	${RM} dist/Release/GNU-Linux-x86/${EXECUTABLE}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
