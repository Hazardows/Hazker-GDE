REM Build script for testbed
@ECHO OFF
SetLocal EnableDelayedExpansion

REM Get a list of all .c files
SET cFilenames=
FOR /R %%f in (*.c) do (
    SET cFilenames=!cFilenames! "%%f"
)

REM Debugging output (optional)
REM echo Files: %cFilenames%

SET assembly=testbed
SET compilerFlags=-g
REM -Wall -Werror
SET includeFlags=-Isrc -I"..\engine\src"
SET linkerFlags=-L"..\bin" -lhazkerEngine
SET defines=-D_DEBUG -DHIMPORT

ECHO "Building %assembly%..."
clang %cFilenames% %compilerFlags% -o "..\bin\%assembly%.exe" %defines% %includeFlags% %linkerFlags%

IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)
ECHO "%assembly% built successfully."