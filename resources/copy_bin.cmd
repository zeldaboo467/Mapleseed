SET MODE=debug
set DIR=..
SET OUTDIR=binaries

MKDIR %OUTDIR%

copy %DIR%\bin\CemuCrypto\%MODE%\CemuCrypto.dll %OUTDIR%\CemuCrypto.dll
copy %DIR%\bin\CemuCrypto\%MODE%\CemuCrypto.pdb %OUTDIR%\CemuCrypto.pdb

copy %DIR%\bin\CemuDatabase\%MODE%\CemuDatabase.dll %OUTDIR%\CemuDatabase.dll
copy %DIR%\bin\CemuDatabase\%MODE%\CemuDatabase.pdb %OUTDIR%\CemuDatabase.pdb

copy %DIR%\bin\CemuLibrary\%MODE%\CemuLibrary.dll %OUTDIR%\CemuLibrary.dll
copy %DIR%\bin\CemuLibrary\%MODE%\CemuLibrary.pdb %OUTDIR%\CemuLibrary.pdb

copy %DIR%\bin\Network\%MODE%\Network.dll %OUTDIR%\Network.dll
copy %DIR%\bin\Network\%MODE%\Network.pdb %OUTDIR%\Network.pdb

copy %DIR%\bin\Core\%MODE%\MapleSeed.exe %OUTDIR%\MapleSeed.exe
copy %DIR%\bin\Core\%MODE%\MapleSeed.pdb %OUTDIR%\MapleSeed.pdb

copy %DIR%\resources\libraries\* %OUTDIR%\*