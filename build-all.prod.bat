@echo off

ECHO "Building everything..."


REM Engine
make -f "Engine.prod.makefile.mak" all
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)


REM Sandbox
make -f "Sandbox.prod.makefile.mak" all
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)