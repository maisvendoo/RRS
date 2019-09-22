set DEPLOY_DIR=%1

mkdir %DEPLOY_DIR%
mkdir %DEPLOY_DIR%\modules

mkdir %DEPLOY_DIR%\cfg
mkdir %DEPLOY_DIR%\cfg\vehicles\
mkdir %DEPLOY_DIR%\cfg\trains\

mkdir %DEPLOY_DIR%\data
mkdir %DEPLOY_DIR%\data\animations\
mkdir %DEPLOY_DIR%\data\models\
mkdir %DEPLOY_DIR%\data\sounds\

set SOURCE_DIR=..\..\..\..\..

xcopy /S %SOURCE_DIR%\modules\chs2t\*.dll %DEPLOY_DIR%\modules\chs2t\
xcopy /S %SOURCE_DIR%\cfg\vehicles\chs2t\*.* %DEPLOY_DIR%\cfg\vehicles\chs2t\
copy %SOURCE_DIR%\cfg\trains\chs2t-single.xml %DEPLOY_DIR%\cfg\trains\chs2t.xml

xcopy /S %SOURCE_DIR%\data\animations\chs2t\*.* %DEPLOY_DIR%\data\animations\chs2t\
xcopy /S %SOURCE_DIR%\data\models\chs2t\*.* %DEPLOY_DIR%\data\models\chs2t\
xcopy /S %SOURCE_DIR%\data\sounds\chs2t\*.* %DEPLOY_DIR%\data\sounds\chs2t\