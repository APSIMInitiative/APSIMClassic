# Makefile for tcl component
# Uses stock 8.5.9 distribution 
# These are stored in the runtimes dir and extracted here the 1st time
# this makefile is called.

include $(APSIM)/Model/Build/Platform.make

PROJECT = TclLink
PROJECTTYPE = dll

# ---------------------------------------------------------------------------
# TCL SECTION
# ---------------------------------------------------------------------------
VER=85
MAJOR=8
DOTVER=8.5
PATCH=9
TCLVER=$(DOTVER).$(PATCH)

ifeq ($(PLATFORM),WIN32VS)
 INCLUDES = /I include 
# LIBPATH = /LIBPATH:lib
 LIBS = General ApsimShared ComponentInterface2 lib\tclstub$(VER)
 OBJS = $(APSIM)/Model/ComponentInterface2/CMPComponentEntryPoints.obj
endif

ifeq ($(PLATFORM),Linux)
 INCLUDES = -I /usr/include/tcl$(DOTVER)
 LIBS = General ApsimShared ComponentInterface2 tclstub$(DOTVER)
 OBJS = $(APSIM)/Model/ComponentInterface2/CMPComponentEntryPoints.o
endif

SRC = TclLink.cpp TclComponent.cpp 

PREBUILD = dummy CIDataTypes.tcl

include $(APSIM)/Model/Build/$(PLATFORM)CPP.make

# ---------------------------------------------------------------------------
# This unpacks a SFX tcl install into our install dir
# ---------------------------------------------------------------------------
dummy: 
ifeq ($(PLATFORM),WIN32VS)
	if not exist $(APSIM)\Model\TclLink\include\nul $(APSIM)\Model\RunTime\Tcl8.5.9.exe x -o"$(APSIM)\Model\TclLink" -y
endif

clean:
ifeq ($(PLATFORM),WIN32VS)
	@if exist $(APSIM)\Model\TclLink\bin\nul rmdir /s /q $(APSIM)\Model\TclLink\bin
	@if exist $(APSIM)\Model\TclLink\lib\nul rmdir /s /q $(APSIM)\Model\TclLink\lib
	@if exist $(APSIM)\Model\TclLink\include\nul rmdir /s /q $(APSIM)\Model\TclLink\include
endif

CIDataTypes.tcl: ../DataTypes/Datatypes.xml TclComponent.macro
ifeq ($(PLATFORM),WIN32VS)
	$(APSIM)/Model/ProcessDataTypesInterface.exe ../DataTypes/Datatypes.xml TclComponent.macro
else
	mono $(APSIM)/Model/ProcessDataTypesInterface.exe ../DataTypes/Datatypes.xml TclComponent.macro
endif
