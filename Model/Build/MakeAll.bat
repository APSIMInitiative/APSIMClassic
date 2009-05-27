@echo off
rem -------------------------------------------------------------
rem This batch file compiles everything
rem -------------------------------------------------------------
pushd ..\..
set APSIM=%CD%

rem -------------------------------------------------------------
rem Remove the old build.out file.
rem -------------------------------------------------------------
cd %APSIM%\Model\Build
del /Q build.out 2> nul

rem ----------------------------------------------
rem Put version info in apsim.xml
rem ----------------------------------------------
VersionStamper.tcl

rem -------------------------------------------------------------
rem Need to compile the GUI first so that the VBGeneral &
rem CSGeneral exist for the DataTypes project.
rem -------------------------------------------------------------
call MakeProject ApsimUI

rem -------------------------------------------------------------
rem Need to process the datatypes.interface file and auto-
rem generate the datatypes.cpp, .h etc.
rem -------------------------------------------------------------
call MakeProject DataTypes
call %APSIM%\Model\DataTypes\DataTypes.bat

rem -------------------------------------------------------------
rem Need to compile all pre-requisite projects before the main
rem bulk of projects
rem -------------------------------------------------------------
call MakeProject General
call MakeProject Build\def2imp
call MakeProject GeneralVCL
call MakeProject ApsimShared
call MakeProject Protocol
call MakeProject ComponentInterface
call MakeProject ComponentInterface2
call MakeProject DotNetComponentInterface
call MakeProject FortranInfrastructure
call MakeProject FortranComponentInterface
call MakeProject FortranComponentInterface2
call MakeProject CropTemplate

rem -------------------------------------------------------------
rem Now we can compile all the rest of the APSIM projects.
rem -------------------------------------------------------------
call MakeProject Apsim
call MakeProject Accum
call MakeProject ApsimRun
call MakeProject ApsimToSim
call MakeProject Canopy
call MakeProject Clock
call MakeProject ConToSim
call MakeProject CropMod
call MakeProject Eo
call MakeProject Erosion
call MakeProject Fertiliser
call MakeProject Grasp
call MakeProject Graz
call MakeProject Growth
call MakeProject Input
call MakeProject Irrigation
call MakeProject Log
call MakeProject Maize
call MakeProject Manager
call MakeProject Map
call MakeProject MicroMet
call MakeProject Millet
call MakeProject Operations
call MakeProject Oryza
call MakeProject Ozcot
call MakeProject Parasite
call MakeProject Pasture
call MakeProject PatchInput
call MakeProject Plant
call MakeProject Plant2
call MakeProject Pond
call MakeProject ProtocolManager
call MakeProject Report
call MakeProject SiloInput
call MakeProject SOI
call MakeProject SoilN
call MakeProject SoilP
call MakeProject SoilTemp
call MakeProject SoilWat
call MakeProject Solute
call MakeProject Sorghum
call MakeProject Stock
call MakeProject Sugar
call MakeProject Supplement
call MakeProject Surface
call MakeProject SurfaceOM
call MakeProject SurfaceTemp
call MakeProject SWIM2
call MakeProject SysBal
call MakeProject TclLink
call MakeProject Tracker
call MakeProject Tree
call MakeProject VenLink
call MakeProject WaterSupply
call MakeProject YieldProphet

call MakeProject ApsVis
call MakeProject SEGReport
call MakeProject CallManagedDll
call MakeProject ApsimReport
call MakeProject MergeOutputFiles
call MakeProject ShellExtensions
call MakeProject SetXMLValue
call MakeProject ProbeDll
call MakeProject Plant2Documentation

popd
