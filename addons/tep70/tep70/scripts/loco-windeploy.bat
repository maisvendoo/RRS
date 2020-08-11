set DEPLOY_DIR=%1

set LOCO_NAME=%2

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

xcopy /S %SOURCE_DIR%\modules\%LOCO_NAME%\*.dll %DEPLOY_DIR%\modules\%LOCO_NAME%\
xcopy /S %SOURCE_DIR%\cfg\vehicles\%LOCO_NAME%\*.* %DEPLOY_DIR%\cfg\vehicles\%LOCO_NAME%\
copy %SOURCE_DIR%\cfg\trains\%LOCO_NAME%.xml %DEPLOY_DIR%\cfg\trains\%LOCO_NAME%.xml

xcopy /S %SOURCE_DIR%\data\animations\%LOCO_NAME%\*.* %DEPLOY_DIR%\data\animations\%LOCO_NAME%\
xcopy /S %SOURCE_DIR%\data\models\%LOCO_NAME%\*.* %DEPLOY_DIR%\data\models\%LOCO_NAME%\
xcopy /S %SOURCE_DIR%\data\sounds\%LOCO_NAME%\*.* %DEPLOY_DIR%\data\sounds\%LOCO_NAME%\