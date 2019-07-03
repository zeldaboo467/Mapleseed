SET MODE=debug
set DIR=..
SET OUTDIR=binaries\mapleseed

MKDIR %OUTDIR%

copy %DIR%\build\CemuCrypto\%MODE%\CemuCrypto.dll %OUTDIR%\CemuCrypto.dll
copy %DIR%\build\CemuCrypto\%MODE%\CemuCrypto.pdb %OUTDIR%\CemuCrypto.pdb

copy %DIR%\build\CemuDB\%MODE%\CemuDatabase.dll %OUTDIR%\CemuDatabase.dll
copy %DIR%\build\CemuDB\%MODE%\CemuDatabase.pdb %OUTDIR%\CemuDatabase.pdb

copy %DIR%\build\CemuLibrary\%MODE%\CemuLibrary.dll %OUTDIR%\CemuLibrary.dll
copy %DIR%\build\CemuLibrary\%MODE%\CemuLibrary.pdb %OUTDIR%\CemuLibrary.pdb

copy %DIR%\build\Network\%MODE%\Network.dll %OUTDIR%\Network.dll
copy %DIR%\build\Network\%MODE%\Network.pdb %OUTDIR%\Network.pdb

copy %DIR%\build\Core\%MODE%\MapleSeed.exe %OUTDIR%\MapleSeed.exe
copy %DIR%\build\Core\%MODE%\MapleSeed.pdb %OUTDIR%\MapleSeed.pdb

copy %DIR%\resources\libraries\* %OUTDIR%\*