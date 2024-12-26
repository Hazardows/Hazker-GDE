REM Build script for engine
@ECHO OFF
SetLocal EnableDelayedExpansion

REM Set paths to Vulkan SDK

REM Get a list of all .c files
SET cFilenames=
FOR /R %%f in (*.c) do (
    SET cFilenames=!cFilenames! "%%f"
)

IF NOT EXIST "..\bin" ( 
    mkdir "..\bin" 
)

REM Debugging output (optional)
REM echo Files: %cFilenames%

SET assembly=hazkerEngine
SET compilerFlags=-g -shared -Wvarargs -Wall -Werror
REM -Wall -Werror

REM Include flags
SET includeFlags=-Isrc -I%VULKAN_SDK%\Include

REM Linker flags
SET linkerFlags=-luser32 -lvulkan-1 -L%VULKAN_SDK%\Lib

REM Defines
SET defines=-D_DEBUG -DHEXPORT -D_CRT_SECURE_NO_WARNINGS

ECHO "Building %assembly%..."
clang %cFilenames% %compilerFlags% -o "../bin/%assembly%.dll" %defines% %includeFlags% %linkerFlags%

IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)
ECHO "%assembly% built successfully."
