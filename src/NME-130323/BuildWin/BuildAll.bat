REM Build all Visual C++ projects in Release configuration

MSBuild NMETool.vcxproj /t:build /p:Configuration=Release
MSBuild NMEEPubTool.vcxproj /t:build /p:Configuration=Release
