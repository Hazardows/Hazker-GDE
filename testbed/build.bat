@ECHO OFF
REM Build script for testbed
SetLocal EnableDelayedExpansion

REM Get a list of all .c files
SET cFilenames=
FOR /R %%f in (*.c) do (
    SET cFilenames=!cFilenames! "%%f"
)

REM Get a list of all .cpp files
SET cppFilenames=
FOR /R %%f in (*.cpp) do (
    SET cppFilenames=!cppFilenames! "%%f"
)

IF NOT EXIST "..\bin\obj" ( 
    mkdir "..\bin\obj" 
)

REM Debugging output (optional)
REM echo C Files: %cFilenames%
REM echo CPP Files: %cppFilenames%

REM Project settings
SET assembly=testbed
SET compilerFlags=-g
REM -Wall -Werror

SET includeFlags=-Isrc -I"..\engine\src"
SET linkerFlags=-L"..\bin" -lhazkerEngine -lmsvcrtd
SET defines=-D_DEBUG -DHIMPORT

REM Delete all .o files in the output directory 
DEL "..\bin\obj\*.o"

ECHO "Building %assembly%..."

REM Compile C++ files
FOR %%f in (%cppFilenames%) DO (
    clang++ %%f %compilerFlags% %defines% %includeFlags% -c -o "..\bin\obj\%%~nf.o"
)

REM Compile C files
FOR %%f in (%cFilenames%) DO (
    clang %%f %compilerFlags% %defines% %includeFlags% -c -o "..\bin\obj\%%~nf.o"
)

REM Collect all object files
SET objFiles=
FOR %%f in (%cppFilenames% %cFilenames%) DO (
    SET objFiles=!objFiles! "..\bin\obj\%%~nf.o"
)

ECHO "Linking..."
clang++ %objFiles% -o "..\bin\%assembly%.exe" %linkerFlags%

IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)
ECHO "%assembly% built successfully."
