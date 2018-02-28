@echo off

if %1.==. goto usage

set arg1=%1

:: strip off relative path
if "%arg1:~0,3%" == "..\" set arg1=%arg1:~3%

set startdir=%cd%
cd "%~dp0"

echo --- amd_lib ---
cd ..\amd_lib\shared\d3d11\premake
call :createvsfilesincluding2010
call :createvsfileswithminimaldependencies

echo --- amd_sdk ---
cd ..\..\..\..\framework\d3d11\amd_sdk\premake
call :createvsfilesincluding2010
call :createvsfileswithminimaldependencies

echo --- dxut core ---
cd ..\..\dxut\Core
call :createvsfilesincluding2010

echo --- dxut optional ---
cd ..\Optional
call :createvsfilesincluding2010

echo --- dx12u ---
cd ..\..\..\d3d12\dx12u\premake
call :createvsfiles12_b

echo --- gu ---
cd ..\..\gu\premake
call :createvsfiles12_b
cd ..\..\..\..\
:: we don't keep solution files for amd_lib, amd_sdk, dxut, etc.
call :cleanslnfiles

echo --- %arg1% ---
cd %arg1%\premake
call :createvsfiles
call :createvsfiles12_a
cd ..\..\

:: sample, capture_viewer, etc.
for /f %%a in ('dir /a:d /b %arg1%_* 2^>nul') do call :createvsfilesforsamples %%a
for /f %%a in ('dir /a:d /b %arg1%11_* 2^>nul') do call :createvsfilesforsamples %%a
for /f %%a in ('dir /a:d /b %arg1%12_* 2^>nul') do call :createvsfilesforsamples12 %%a

cd "%startdir%"

goto :EOF

::--------------------------
:: SUBROUTINES
::--------------------------

:: sample, capture_viewer, etc.
:createvsfilesforsamples
if exist %1\premake (
    echo --- %1 ---
    cd %1\premake
    call :createvsfiles
    cd ..\..\
)
goto :EOF

:: sample, capture_viewer, etc.
:createvsfilesforsamples12
if exist %1\premake (
    echo --- %1 ---
    cd %1\premake
    ..\..\premake\premake5.exe vs2015
    ..\..\premake\premake5.exe vs2017
    cd ..\..\
)
goto :EOF

:: run premake for vs2012, vs2013, vs2015 and vs2017
:createvsfiles
..\..\premake\premake5.exe vs2012
..\..\premake\premake5.exe vs2013
..\..\premake\premake5.exe vs2015
..\..\premake\premake5.exe vs2017
goto :EOF

:: run premake for vs2015 and vs2017 for d3d12
:createvsfiles12_a
..\..\premake\premake5.exe --file=premake5_d3d12.lua vs2015
..\..\premake\premake5.exe --file=premake5_d3d12.lua vs2017
goto :EOF

:: run premake for vs2015 and vs2017 for d3d12
:createvsfiles12_b
..\..\..\..\premake\premake5.exe vs2015
..\..\..\..\premake\premake5.exe vs2017
goto :EOF

:: run premake for vs2010, vs2012, vs2013, vs2015 and vs2017
:createvsfilesincluding2010
..\..\..\..\premake\premake5.exe vs2010
..\..\..\..\premake\premake5.exe vs2012
..\..\..\..\premake\premake5.exe vs2013
..\..\..\..\premake\premake5.exe vs2015
..\..\..\..\premake\premake5.exe vs2017
goto :EOF

:: run premake for vs2010, vs2012, vs2013, vs2015 and vs2017
:createvsfileswithminimaldependencies
..\..\..\..\premake\premake5.exe --file=premake5_minimal.lua vs2010
..\..\..\..\premake\premake5.exe --file=premake5_minimal.lua vs2012
..\..\..\..\premake\premake5.exe --file=premake5_minimal.lua vs2013
..\..\..\..\premake\premake5.exe --file=premake5_minimal.lua vs2015
..\..\..\..\premake\premake5.exe --file=premake5_minimal.lua vs2017
goto :EOF

:: delete unnecessary sln files
:cleanslnfiles
del /f /q amd_lib\shared\d3d11\build\AMD_LIB_2010.sln
del /f /q amd_lib\shared\d3d11\build\AMD_LIB_2012.sln
del /f /q amd_lib\shared\d3d11\build\AMD_LIB_2013.sln
del /f /q amd_lib\shared\d3d11\build\AMD_LIB_2015.sln
del /f /q amd_lib\shared\d3d11\build\AMD_LIB_2017.sln

del /f /q amd_lib\shared\d3d11\build\AMD_LIB_Minimal_2010.sln
del /f /q amd_lib\shared\d3d11\build\AMD_LIB_Minimal_2012.sln
del /f /q amd_lib\shared\d3d11\build\AMD_LIB_Minimal_2013.sln
del /f /q amd_lib\shared\d3d11\build\AMD_LIB_Minimal_2015.sln
del /f /q amd_lib\shared\d3d11\build\AMD_LIB_Minimal_2017.sln

del /f /q framework\d3d11\amd_sdk\build\AMD_SDK_2010.sln
del /f /q framework\d3d11\amd_sdk\build\AMD_SDK_2012.sln
del /f /q framework\d3d11\amd_sdk\build\AMD_SDK_2013.sln
del /f /q framework\d3d11\amd_sdk\build\AMD_SDK_2015.sln
del /f /q framework\d3d11\amd_sdk\build\AMD_SDK_2017.sln

del /f /q framework\d3d11\amd_sdk\build\AMD_SDK_Minimal_2010.sln
del /f /q framework\d3d11\amd_sdk\build\AMD_SDK_Minimal_2012.sln
del /f /q framework\d3d11\amd_sdk\build\AMD_SDK_Minimal_2013.sln
del /f /q framework\d3d11\amd_sdk\build\AMD_SDK_Minimal_2015.sln
del /f /q framework\d3d11\amd_sdk\build\AMD_SDK_Minimal_2017.sln

del /f /q framework\d3d11\dxut\Core\DXUT_2010.sln
del /f /q framework\d3d11\dxut\Core\DXUT_2012.sln
del /f /q framework\d3d11\dxut\Core\DXUT_2013.sln
del /f /q framework\d3d11\dxut\Core\DXUT_2015.sln
del /f /q framework\d3d11\dxut\Core\DXUT_2017.sln

del /f /q framework\d3d11\dxut\Optional\DXUTOpt_2010.sln
del /f /q framework\d3d11\dxut\Optional\DXUTOpt_2012.sln
del /f /q framework\d3d11\dxut\Optional\DXUTOpt_2013.sln
del /f /q framework\d3d11\dxut\Optional\DXUTOpt_2015.sln
del /f /q framework\d3d11\dxut\Optional\DXUTOpt_2017.sln

del /f /q framework\d3d12\dx12u\build\dx12u_2015.sln
del /f /q framework\d3d12\dx12u\build\dx12u_2017.sln
del /f /q framework\d3d12\gu\build\gu_2015.sln
del /f /q framework\d3d12\gu\build\gu_2017.sln
goto :EOF

::--------------------------
:: usage should be last
::--------------------------

:usage
echo   usage: %0 library_dir_name
echo      or: %0 ..\library_dir_name
echo example: %0 AMD_AOFX
echo      or: %0 ..\AMD_AOFX
