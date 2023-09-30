@ECHO OFF
REM Clean Everything

ECHO "Cleaning everything..."


REM Engine
make -f "Engine.makefile.mak" clean 
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)


REM Sandbox
make -f "Sandbox.makefile.mak" clean
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)