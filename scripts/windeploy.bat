rem Определяем необходимые переменные окружения

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

xcopy /Y  ..\..\bin\*.exe %RRS_DEV_ROOT%\bin\
xcopy /Y  ..\..\lib\*.dll %RRS_DEV_ROOT%\bin\
move %RRS_DEV_ROOT%\bin\rkf5.dll %RRS_DEV_ROOT%\lib\
move %RRS_DEV_ROOT%\bin\rk4.dll %RRS_DEV_ROOT%\lib\
move %RRS_DEV_ROOT%\bin\euler2.dll %RRS_DEV_ROOT%\lib\
move %RRS_DEV_ROOT%\bin\euler.dll %RRS_DEV_ROOT%\lib\

rem Копируем модули

xcopy /Y ..\..\modules\*.dll %RRS_DEV_ROOT%\modules
xcopy /Y ..\..\modules\vl60k\*.dll %RRS_DEV_ROOT%\modules\vl60k\
xcopy /Y ..\..\modules\vl60pk\*.dll %RRS_DEV_ROOT%\modules\vl60pk\
xcopy /Y ..\..\modules\passcar\*.dll %RRS_DEV_ROOT%\modules\passcar\
xcopy /Y ..\..\modules\freightcar\*.dll %RRS_DEV_ROOT%\modules\freightcar\

rem Копируем плагины

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
rem Копируем прочие необходимые DLL

xcopy /Y %OPENAL_BIN%\*.dll %RRS_DEV_ROOT%\bin\

rem Копируем шейдеры
xcopy /Y /S %DATA_PATH%\shaders\*.* %RRS_DEV_ROOT%\data\shaders\

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
xcopy /Y /S %DATA_PATH%\models\default-objects\*.* %RRS_DEV_ROOT%\data\models\default-objects
xcopy /Y /S %DATA_PATH%\animations\default-objects\*.* %RRS_DEV_ROOT%\data\animations\default-objects 

rem Шрифты вьювера и темы оформления лаунчера

xcopy /Y ..\fonts\*.* %RRS_DEV_ROOT%\fonts
xcopy /Y ..\themes\*.* %RRS_DEV_ROOT%\themes

rem Копируем маршруты

rem xcopy /Y /S ..\routes\experimental-polygon\*.* %RRS_DEV_ROOT%\routes\experimental-polygon\
rem rmdir /S /Q %RRS_DEV_ROOT%\routes\experimental-polygon\map_editor
rem del /S %RRS_DEV_ROOT%\routes\experimental-polygon\~*.*

xcopy /Y /S ..\routes\experimental-polygon-gltf\*.* %RRS_DEV_ROOT%\routes\experimental-polygon\

rem Копируем SDK

xcopy /Y /Q ..\common-headers\key-symbols.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Y /Q ..\common-headers\sound-signal.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Y /Q ..\CfgReader\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Y /Q ..\filesystem\include\*.h %RRS_DEV_ROOT%\sdk\include\
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

xcopy /Y /S ..\docs\*.pdf %RRS_DEV_ROOT%\docs\

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