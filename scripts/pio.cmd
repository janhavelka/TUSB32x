@echo off
setlocal DisableDelayedExpansion
rem Clear the optional environment variable that shadows CMD's actual exit status.
set "ERRORLEVEL="
set "TUSB32X_PIO_EXE=%USERPROFILE%\.platformio\penv\Scripts\pio.exe"
if not exist "%TUSB32X_PIO_EXE%" (
  >&2 echo VS Code-managed PlatformIO was not found at: "%TUSB32X_PIO_EXE%". No second Core is installed by this launcher.
  exit /b 1
)
"%TUSB32X_PIO_EXE%" %*
exit /b %ERRORLEVEL%
