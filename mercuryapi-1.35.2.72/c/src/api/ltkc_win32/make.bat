@echo off

echo LTKC Build Started
echo Running gencode.bat to generate supporting files.

call "gencode.bat"

echo "Starting Visual Studio 2005..."
set configuration="Debug"
IF EXIST "C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\vsvars32.bat" (
	goto visualStudioProgramFiles
)
IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio 8\Common7\Tools\vsvars32.bat" (
	goto visualStudioProgramFilesx86
)

:visualStudioProgramFiles
call "C:\Program Files\Microsoft Visual Studio 8\Common7\Tools\vsvars32.bat" 
goto buildProjects

:visualStudioProgramFilesx86
call "C:\Program Files (x86)\Microsoft Visual Studio 8\Common7\Tools\vsvars32.bat"
goto buildProjects

:buildProjects
echo "Building LTKC Project Started..."
devenv "proj\VS_LTKC.sln" /Clean
devenv "proj\VS_LTKC.sln" /Build %configuration%
goto exitBatch
rem goto copyFilesToMercuryAPISDK

rem :copyFilesToMercuryAPISDK
rem echo "Copying ..."
rem goto exitBatch

:exitBatch
echo "Execution Completed"
@echo on