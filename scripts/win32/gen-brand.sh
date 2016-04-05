#!/bin/sh
#
# Notes:
#   - escape \${var} as => \\${var}

echo "Generating brand-specific OpenVPN scripts"

cat << EOF > openvpn-up.bat

::
:: OpenVPN 'up' script for Windows
:: Sets Encloud gateway so routes can be pulled down manually [CONNECT-333]
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
set LIBENCLOUD_KEY="HKEY_LOCAL_MACHINE\SOFTWARE\\${PACKAGE_ORG}\libencloud"
set PROGRAMFILES_KEY="HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion"
set TMP_FILE=openvpn-up.bat.tmp

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
REG QUERY %PROGRAMFILES_KEY% /v ProgramFilesDir > %TMP_FILE%
FOR /F "tokens=3-4" %%i in (%TMP_FILE%) DO (
    :: Hack to handle possible space (we assume it has max two words)
    if "%%j" == "" (
        set PROGRAMFILES_DIR=%%i
    ) else (
        set PROGRAMFILES_DIR=%%i %%j
    )
)
del %TMP_FILE%
if "%PROGRAMFILES_DIR%" == "" (
    call :log CRITICAL: could not retrieve ProgramFilesDir from registry!
    exit /b 1
)
call :log ProgramFilesDir: %PROGRAMFILES_DIR%

set APPBIN_DIR="%PROGRAMFILES_DIR%\\${PACKAGE_ORG}\\${PACKAGE_PROD_STRIP}\bin"

::
:: Set gateway in Encloud Service
::
%APPBIN_DIR%\qurl.exe http://localhost:%ENCLOUD_PORT%/api/v1/cloud -d "action=setGateway" -d "ip=%ROUTE_VPN_GATEWAY%"

if NOT %errorlevel% == 0 (
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
