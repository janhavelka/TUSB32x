@echo off
setlocal DisableDelayedExpansion
set "TUSB32X_PIO_EXE=%USERPROFILE%\.platformio\penv\Scripts\pio.exe"
if not exist "%TUSB32X_PIO_EXE%" (
  >&2 echo VS Code-managed PlatformIO was not found at: "%TUSB32X_PIO_EXE%". No second Core is installed by this launcher.
  exit /b 1
)
"%TUSB32X_PIO_EXE%" %*
exit /b %ERRORLEVEL%
