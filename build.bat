@title "Shairport4w - Single build"

@echo "Shairport4w - Single build"
@echo "Use : build.bat BUILD_PLATFORM=<platform> BUILD_CONFIG=<config>, e.g. build.bat BUILD_PLATFORM=x64 BUILD_CONFIG=Release"
@echo "Default for build.bat alone : x64 release"
@echo "VS 2019 required"
@echo:


@if "%PROCESSOR_ARCHITECTURE%"=="x86" set DIR=%ProgramFiles%
@if "%PROCESSOR_ARCHITECTURE%"=="AMD64" set DIR=%ProgramFiles(x86)%

@call "%DIR%\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars32.bat"


rem Platform x86/x64
if "%BUILD_PLATFORM%"=="" set BUILD_PLATFORM=x64

rem Config Debug/Release
if "%BUILD_CONFIG%"=="" set BUILD_CONFIG=Release

echo:
echo "Building %BUILD_PLATFORM% %BUILD_CONFIG%"
echo:

MSBuild /nologo .\src\Shairport4w.vcxproj /t:Rebuild /p:Configuration=%BUILD_CONFIG%;Platform=%BUILD_PLATFORM%

echo:
echo "Build finished."
echo:

pause

