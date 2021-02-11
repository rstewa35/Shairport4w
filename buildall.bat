@title "Shairport4w - Rebuild all"

@echo "Shairport4w - Rebuild all"
@echo:
@echo "VS 2019 required"
@echo:

@if "%PROCESSOR_ARCHITECTURE%"=="x86" set DIR=%ProgramFiles%
@if "%PROCESSOR_ARCHITECTURE%"=="AMD64" set DIR=%ProgramFiles(x86)%

@call "%DIR%\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars32.bat"


echo:
echo "Debug x86"
echo:

MSBuild /nologo .\src\Shairport4w.vcxproj /t:Rebuild /p:Configuration=Debug;Platform=x86

echo:
echo "Release x86"
echo:

MSBuild /nologo .\src\Shairport4w.vcxproj /t:Rebuild /p:Configuration=Release;Platform=x86

echo:
echo "Debug x64"
echo:

MSBuild /nologo .\src\Shairport4w.vcxproj /t:Rebuild /p:Configuration=Debug;Platform=x64

echo:
echo "Release x64"
echo:

MSBuild /nologo .\src\Shairport4w.vcxproj /t:Rebuild /p:Configuration=Release;Platform=x64

echo:
echo "Build finished."
echo:

pause

