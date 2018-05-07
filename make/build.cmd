@echo off

IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\Preview\Professional\MSBuild\15.0\Bin\Roslyn\csi.exe" (
    SET VSVINTAGE=Preview
    SET VSFLAVOR=Professional
) else IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\Preview\Community\MSBuild\15.0\Bin\Roslyn\csi.exe" (
    SET VSVINTAGE=Preview
    SET VSFLAVOR=Community
) else IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\MSBuild\15.0\Bin\Roslyn\csi.exe" (
    SET VSVINTAGE=2017
    SET VSFLAVOR=Professional
) else IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\Roslyn\csi.exe" (
    SET VSVINTAGE=2017
    SET VSFLAVOR=Community
) else (
    echo "Can not find Visual Studio 2017"
    GOTO exit
)

"C:\Program Files (x86)\Microsoft Visual Studio\%VSVINTAGE%\%VSFLAVOR%\MSBuild\15.0\Bin\Roslyn\csi.exe" make/compile.csx %* -warnings_are_errors
:exit