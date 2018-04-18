@echo off

IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    SET VSFLAVOR=Professional
) else IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat" (
    SET VSFLAVOR=Community
) else (
    echo "Can not find Visual Studio 2017"
    GOTO exit
)

"C:\Program Files (x86)\Microsoft Visual Studio\2017\%VSFLAVOR%\MSBuild\15.0\Bin\Roslyn\csi.exe" make/compile.csx %* -force -warnings_are_errors
:exit