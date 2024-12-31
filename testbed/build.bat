@ECHO OFF
REM Build script for testbed

ECHO "Building Testbed..."

cd ..
make -f "Makefile.testbed.windows.mak" all 
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

ECHO "Testbed built successfully."