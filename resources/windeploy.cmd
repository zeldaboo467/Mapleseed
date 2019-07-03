SET VCINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC
SET BASE=..
SET RELEASE=%BASE%\bin\release\
windeployqt --dir %RELEASE% --plugindir %RELEASE%plugins %RELEASE%MapleSeed.exe