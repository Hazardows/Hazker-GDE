@ECHO OFF
REM Build script for testbed

ECHO "Building Tests..."

cd ..
make -f "Makefile.tests.windows.mak" all 
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

ECHO "Tests built successfully."