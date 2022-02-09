@echo off

IF "%1" EQU "" goto error

@ECHO copy DLL to path: %1

XCOPY simple_world\Release\simple_world.dll %1\release_x32\ /y
XCOPY simple_world\Release\simple_world.lib %1\release_x32\ /y

XCOPY simple_world\x64\Release\simple_world.dll %1\release_x64\ /y
XCOPY simple_world\x64\Release\simple_world.lib %1\release_x64\ /y

XCOPY simple_world\Debug\simple_world.dll %1\debug_x32\ /y
XCOPY simple_world\Debug\simple_world.lib %1\debug_x32\ /y

XCOPY simple_world\x64\Debug\simple_world.dll %1\debug_x64\ /y
XCOPY simple_world\x64\Debug\simple_world.lib %1\debug_x64\ /y

XCOPY simple_world\dll_main\simple_world.h %1\include\ /y

pause
goto end

:error
@ECHO Undefined path, send it to first argument
pause

:end