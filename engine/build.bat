@ECHO OFF
REM Build script for engine

ECHO "Building Engine..."

cd ..
make -f "Makefile.engine.windows.mak" all 
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

ECHO "Engine built successfully."