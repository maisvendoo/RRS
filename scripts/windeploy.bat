rem Определяем необходимые переменные окружения

set OSG_PLUGINS_DIR=osgPlugins-3.6.5
set DATA_PATH=..\data

rem Создаем структуру каталогов игры

mkdir %RRS_DEV_ROOT%
mkdir %RRS_DEV_ROOT%\bin
mkdir %RRS_DEV_ROOT%\lib
mkdir %RRS_DEV_ROOT%\modules
mkdir %RRS_DEV_ROOT%\plugins

mkdir %RRS_DEV_ROOT%\cfg
mkdir %RRS_DEV_ROOT%\data
mkdir %RRS_DEV_ROOT%\routes
mkdir %RRS_DEV_ROOT%\fonts
mkdir %RRS_DEV_ROOT%\themes

mkdir %RRS_DEV_ROOT%\sdk
mkdir %RRS_DEV_ROOT%\sdk\include

mkdir %RRS_DEV_ROOT%\logs
mkdir %RRS_DEV_ROOT%\screenshots
mkdir %RRS_DEV_ROOT%\themes

rem Копируем бинарные файлы

xcopy /Y /F  ..\..\bin\*.exe %RRS_DEV_ROOT%\bin\
xcopy /Y /F ..\..\lib\*.dll %RRS_DEV_ROOT%\bin\
move %RRS_DEV_ROOT%\bin\rkf5.dll %RRS_DEV_ROOT%\lib\rkf5.dll
move %RRS_DEV_ROOT%\bin\rk4.dll %RRS_DEV_ROOT%\lib\rk4.dll
move %RRS_DEV_ROOT%\bin\euler2.dll %RRS_DEV_ROOT%\lib\euler2.dll
move %RRS_DEV_ROOT%\bin\euler.dll %RRS_DEV_ROOT%\lib\euler.dll

xcopy /Y ..\..\modules\*.dll %RRS_DEV_ROOT%\modules
xcopy /Y ..\..\modules\vl60k\*.dll %RRS_DEV_ROOT%\modules\vl60k\
xcopy /Y ..\..\modules\vl60pk\*.dll %RRS_DEV_ROOT%\modules\vl60pk\
xcopy /Y ..\..\modules\passcar\*.dll %RRS_DEV_ROOT%\modules\passcar\
xcopy /Y ..\..\modules\freightcar\*.dll %RRS_DEV_ROOT%\modules\freightcar\

xcopy /Y ..\..\plugins\*.dll %RRS_DEV_ROOT%\plugins

rem Копируем конфиги

xcopy /Y ..\cfg\*.xml %RRS_DEV_ROOT%\cfg\
xcopy /Y ..\cfg\couplings\*.xml %RRS_DEV_ROOT%\cfg\couplings\
xcopy /Y ..\cfg\devices\*.xml %RRS_DEV_ROOT%\cfg\devices\
xcopy /Y ..\cfg\devices\freejoy\*.xml %RRS_DEV_ROOT%\cfg\devices\freejoy\

xcopy /Y ..\cfg\main-resist\default.xml %RRS_DEV_ROOT%\cfg\main-resist\
xcopy /Y ..\cfg\main-resist\passcar.xml %RRS_DEV_ROOT%\cfg\main-resist\
xcopy /Y ..\cfg\main-resist\loco-resist.xml %RRS_DEV_ROOT%\cfg\main-resist\

xcopy /Y ..\cfg\wheel-rail-friction\*.xml %RRS_DEV_ROOT%\cfg\wheel-rail-friction\

xcopy /Y ..\cfg\vehicles\vl60pk\*.* %RRS_DEV_ROOT%\cfg\vehicles\vl60pk\
xcopy /Y ..\cfg\vehicles\vl60k\*.* %RRS_DEV_ROOT%\cfg\vehicles\vl60k\
xcopy /Y ..\cfg\vehicles\IMR_pass_rzd\*.* %RRS_DEV_ROOT%\cfg\vehicles\IMR_pass_rzd\
xcopy /Y ..\cfg\vehicles\Fr_hopper_RZD\*.* %RRS_DEV_ROOT%\cfg\vehicles\Fr_hopper_RZD\

xcopy /Y ..\cfg\trains\vl60pk-1543-T65_17.xml %RRS_DEV_ROOT%\cfg\trains\
xcopy /Y ..\cfg\trains\vl60pk-1543.xml %RRS_DEV_ROOT%\cfg\trains\
xcopy /Y ..\cfg\trains\VL60k-1737.xml %RRS_DEV_ROOT%\cfg\trains\
xcopy /Y ..\cfg\trains\vl60k-1737-frEmpties.xml %RRS_DEV_ROOT%\cfg\trains\
xcopy /Y ..\cfg\trains\vl60k-1737-frLoads.xml %RRS_DEV_ROOT%\cfg\trains\

rem Копируем движок OSG

xcopy /Y %OSG_BIN_PATH%\libOpenThreads.dll %RRS_DEV_ROOT%\bin\
xcopy /Y %OSG_BIN_PATH%\libosg.dll %RRS_DEV_ROOT%\bin\
xcopy /Y %OSG_BIN_PATH%\libosgAnimation.dll %RRS_DEV_ROOT%\bin\
xcopy /Y %OSG_BIN_PATH%\libosgDB.dll %RRS_DEV_ROOT%\bin\
xcopy /Y %OSG_BIN_PATH%\libosgGA.dll %RRS_DEV_ROOT%\bin\
xcopy /Y %OSG_BIN_PATH%\libosgText.dll %RRS_DEV_ROOT%\bin\
xcopy /Y %OSG_BIN_PATH%\libosgUtil.dll %RRS_DEV_ROOT%\bin\
xcopy /Y %OSG_BIN_PATH%\libosgViewer.dll %RRS_DEV_ROOT%\bin\

xcopy /Y %OSG_BIN_PATH%\%OSG_PLUGINS_DIR%\*.dll %RRS_DEV_ROOT%\bin\%OSG_PLUGINS_DIR%\
del /S %RRS_DEV_ROOT%\bin\%OSG_PLUGINS_DIR%\*d.dll
rem xcopy /Y %OSG_BIN_PATH%\%OSG_PLUGINS_DIR%\*dmd.dll %RRS_DEV_ROOT%\bin\%OSG_PLUGINS_DIR%\
xcopy /Y ..\..\plugins\*dmd.dll %RRS_DEV_ROOT%\bin\%OSG_PLUGINS_DIR%\

rem Копируем прочие необходимые DLL

xcopy /Y %OPENAL_BIN%\*.dll %RRS_DEV_ROOT%\bin\
xcopy /Y %FREETYPE_LIBRARY% %RRS_DEV_ROOT%\bin\
xcopy /Y %ZLIB_LIBRARY% %RRS_DEV_ROOT%\bin\

rem Копируем данные игры (модели, звуки, конфиги анимаций, шрифты)

rem ВЛ60пк
xcopy /Y /S %DATA_PATH%\models\VL60pk-1543\*.* %RRS_DEV_ROOT%\data\models\VL60pk-1543\
xcopy /Y /S %DATA_PATH%\sounds\vl60\*.* %RRS_DEV_ROOT%\data\sounds\vl60\
xcopy /Y /S %DATA_PATH%\animations\vl60pk\*.* %RRS_DEV_ROOT%\data\animations\vl60pk\

rem ВЛ60к
xcopy /Y /S %DATA_PATH%\models\VL60k-1737\*.* %RRS_DEV_ROOT%\data\models\VL60k-1737\
xcopy /Y /S %DATA_PATH%\animations\vl60k\*.* %RRS_DEV_ROOT%\data\animations\vl60k\

rem Пассажирские вагоны
xcopy /Y /S %DATA_PATH%\models\IMR_pass_rzd\*.* %RRS_DEV_ROOT%\data\models\IMR_pass_rzd\
xcopy /Y /S %DATA_PATH%\animations\passcar-ox\*.* %RRS_DEV_ROOT%\data\animations\passcar-ox\
xcopy /Y /S %DATA_PATH%\animations\passcar-oy\*.* %RRS_DEV_ROOT%\data\animations\passcar-oy\
xcopy /Y /S %DATA_PATH%\sounds\pass\*.* %RRS_DEV_ROOT%\data\sounds\pass\

rem Вагоны-хоперы
xcopy /Y /S %DATA_PATH%\models\FrWag_hopper_1\*.* %RRS_DEV_ROOT%\data\models\FrWag_hopper_1\
xcopy /Y /S %DATA_PATH%\animations\freight\*.* %RRS_DEV_ROOT%\data\animations\freight\
xcopy /Y /S %DATA_PATH%\sounds\freight\*.* %RRS_DEV_ROOT%\data\sounds\freight\

rem Дефолтные модели светофоров и конфиги их анимации
xcopy /Y /S %DATA_PATH%\models\experimental-polygon\*.* %RRS_DEV_ROOT%\data\models\experimental-polygon
xcopy /Y /S %DATA_PATH%\animations\experimental-polygon\*.* %RRS_DEV_ROOT%\data\animations\experimental-polygon 

rem Шрифты вьювера и темы оформления лаунчера

xcopy /Y ..\fonts\*.* %RRS_DEV_ROOT%\fonts
xcopy /Y ..\themes\*.* %RRS_DEV_ROOT%\themes

rem Копируем маршруты

xcopy /Y /S ..\routes\experimental-polygon\*.* %RRS_DEV_ROOT%\routes\experimental-polygon\
rmdir /S /Q %RRS_DEV_ROOT%\routes\experimental-polygon\map_editor
del /S %RRS_DEV_ROOT%\routes\experimental-polygon\~*.*

rem Копируем SDK

xcopy /Y /Q ..\CfgReader\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Y /Q ..\filesystem\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Y /Q ..\common-headers\sound-signal.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Y /Q ..\simulator\solver\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Y /Q ..\simulator\physics\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Y /Q ..\simulator\vehicle\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Y /Q ..\simulator\device\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Y /Q ..\viewer\display\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Y /Q ..\libJournal\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Y /S ..\..\sdk\examples\*.* %RRS_DEV_ROOT%\sdk\examples\

rem Копируем иконку

xcopy /Y ..\launcher\resources\images\RRS_logo.ico %RRS_DEV_ROOT%\bin\

rem Копируем лицензию

xcopy /Y ..\LICENSE %RRS_DEV_ROOT%\
xcopy /Y ..\LICENSE-Russian %RRS_DEV_ROOT%\

rem Копируем документацию

xcopy /Y /S ..\docs\*.* %RRS_DEV_ROOT%\docs\

rem Генерируем рантайм Qt

cd %RRS_DEV_ROOT%\bin
windeployqt %RRS_DEV_ROOT%\bin\launcher.exe
windeployqt %RRS_DEV_ROOT%\bin\simulator.exe
windeployqt %RRS_DEV_ROOT%\bin\viewer.exe
windeployqt %RRS_DEV_ROOT%\bin\pathconv.exe
windeployqt %RRS_DEV_ROOT%\bin\profconv.exe
windeployqt %RRS_DEV_ROOT%\bin\routeconv.exe
windeployqt %RRS_DEV_ROOT%\bin\CfgReader.dll
windeployqt %RRS_DEV_ROOT%\bin\tcp-connection.dll