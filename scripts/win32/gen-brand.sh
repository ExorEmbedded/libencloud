#!/bin/sh
#
# Notes:
#   - escape \${var} as => \\${var}

echo "Generating brand-specific OpenVPN scripts for Windows"

if [ "${PACKAGE_XBRAND}" = "true" ]; then
    ORG=""
else
    ORG="${PACKAGE_ORG}"
fi

cat << EOF > openvpn-route-up.bat

::
:: OpenVPN 'up' script for Windows:
::  1) Sets Encloud gateway to be used when adding and deleting routes
::    (Switchboard's "openvpn_internal_ip" is unreliable on multicore [CONNECT-377])
::

@echo off
setlocal EnableDelayedExpansion

::
:: Check preconditions
::
if "%ROUTE_VPN_GATEWAY%" == "" (
    call :log CRITICAL: ROUTE_VPN_GATEWAY variable must be defined!
    exit /b 1
)

::
:: Set locals
::
set SOFTWARE32_KEY=HKEY_LOCAL_MACHINE\SOFTWARE
set SOFTWARE64_KEY=HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node
set LIBENCLOUD64_KEY=%SOFTWARE64_KEY%\\${ORG}\libencloud
REG QUERY %LIBENCLOUD64_KEY% >nul
if %errorlevel% == 0 (
    set PLATFORM=64
) else (
    set PLATFORM=32
)
call :log Detected platform: %PLATFORM%
if %PLATFORM% == 64 (
    set SOFTWARE_KEY=%SOFTWARE64_KEY%
    set PROGRAMFILES_VAL="ProgramFilesDir (x86)"
) else (
    set SOFTWARE_KEY=%SOFTWARE32_KEY%
    set PROGRAMFILES_VAL="ProgramFilesDir"
)

set PROGRAMFILES_KEY=%SOFTWARE_KEY%\Microsoft\Windows\CurrentVersion
set LIBENCLOUD_KEY=%SOFTWARE_KEY%\\${ORG}\libencloud
set TMP_FILE=openvpn-route-up.bat.tmp

::
:: Parse port from registry
::
:: The following parsing method works as plain batch not when launched from OpenVPN up script:
:: for /f "usebackq tokens=1-3" %%i in ('REG QUERY %LIBENCLOUD_KEY% /v port') do (
::     set ENCLOUD_PORT=%%k
:: )

REG QUERY %LIBENCLOUD_KEY% /v port > %TMP_FILE%
FOR /F "tokens=3" %%i in (%TMP_FILE%) DO set ENCLOUD_PORT=%%i
del %TMP_FILE%
if "%ENCLOUD_PORT%" == "" (
    call :log CRITICAL: could not retrieve Encloud port from registry!
    exit /b 1
)
call :log Read Encloud Port: %ENCLOUD_PORT%

:: We have no environment, so we read also system paths from registry
REG QUERY %PROGRAMFILES_KEY% /v %PROGRAMFILES_VAL% > %TMP_FILE%
if %PLATFORM% == 64 (
    set START_TOKEN=4
) else (
    set START_TOKEN=3
)
FOR /F "tokens=%START_TOKEN%-6" %%a in (%TMP_FILE%) DO (
    :: Hack to handle possible space (we assume it has max three words)
    set PROGRAMFILES_DIR=%%a
    if "%%b" NEQ "" (
        set PROGRAMFILES_DIR=!PROGRAMFILES_DIR! %%b
    ) 
    if "%%c" NEQ "" (
        set PROGRAMFILES_DIR=!PROGRAMFILES_DIR! %%c
    ) 
)
del %TMP_FILE%
if "%PROGRAMFILES_DIR%" == "" (
    call :log CRITICAL: could not retrieve ProgramFilesDir from registry!
    exit /b 1
)
call :log ProgramFilesDir: %PROGRAMFILES_DIR%

set APPBIN_DIR="%PROGRAMFILES_DIR%\\${ORG}\\${PACKAGE_PROD_STRIP}\bin"

::
:: Set gateway in Encloud Service
::
%APPBIN_DIR%\qurl.exe http://localhost:%ENCLOUD_PORT%/api/v1/cloud -d action=setGateway -d ip=%ROUTE_VPN_GATEWAY%

if %errorlevel% NEQ 0 (
    call :log CRITICAL: qurl command exited with code: %errorlevel%
    exit /b 1
)

call :log Success

endlocal
echo on

exit /b 0

:log
:: log to file (debug)
::echo %* >> c:\out.log
echo %*
exit /b 0

EOF
echo "Done"
exit 0
