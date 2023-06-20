rem Определяем необходимые переменные окружения

set OSG_PLUGINS_DIR=osgPlugins-3.6.5

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

xcopy  ..\..\bin\*.exe %RRS_DEV_ROOT%\bin\
xcopy ..\..\lib\*.dll %RRS_DEV_ROOT%\bin\
move %RRS_DEV_ROOT%\bin\rkf5.dll %RRS_DEV_ROOT%\lib\rkf5.dll
move %RRS_DEV_ROOT%\bin\rk4.dll %RRS_DEV_ROOT%\lib\rk4.dll

xcopy ..\..\modules\*.dll %RRS_DEV_ROOT%\modules
xcopy ..\..\modules\vl60k\*.dll %RRS_DEV_ROOT%\modules\vl60k\
xcopy ..\..\modules\vl60pk\*.dll %RRS_DEV_ROOT%\modules\vl60pk\
xcopy ..\..\modules\passcar\*.dll %RRS_DEV_ROOT%\modules\passcar\

xcopy ..\..\plugins\*.dll %RRS_DEV_ROOT%\plugins

rem Копируем конфиги

xcopy ..\cfg\*.xml %RRS_DEV_ROOT%\cfg\
del %RRS_DEV_ROOT%\cfg\control-panel.xml
xcopy ..\cfg\couplings\*.xml %RRS_DEV_ROOT%\cfg\couplings\
xcopy ..\cfg\devices\*.xml %RRS_DEV_ROOT%\cfg\devices\

xcopy ..\cfg\main-resist\default.xml %RRS_DEV_ROOT%\cfg\main-resist\
xcopy ..\cfg\main-resist\passcar.xml %RRS_DEV_ROOT%\cfg\main-resist\
xcopy ..\cfg\main-resist\loco-resist.xml %RRS_DEV_ROOT%\cfg\main-resist\

xcopy ..\cfg\vehicles\vl60pk-1543\*.* %RRS_DEV_ROOT%\cfg\vehicles\vl60pk-1543\
xcopy ..\cfg\vehicles\VL60k-1737\*.* %RRS_DEV_ROOT%\cfg\vehicles\VL60k-1737\
xcopy ..\cfg\vehicles\IMR_pass_rzd-11100\*.* %RRS_DEV_ROOT%\cfg\vehicles\IMR_pass_rzd-11100\
xcopy ..\cfg\vehicles\IMR_pass_rzd-13819\*.* %RRS_DEV_ROOT%\cfg\vehicles\IMR_pass_rzd-13819\
xcopy ..\cfg\vehicles\IMR_pass_rzd-16733\*.* %RRS_DEV_ROOT%\cfg\vehicles\IMR_pass_rzd-16733\
xcopy ..\cfg\vehicles\IMR_pass_rzd-17669\*.* %RRS_DEV_ROOT%\cfg\vehicles\IMR_pass_rzd-17669\
xcopy ..\cfg\vehicles\IMR_pass_rzd-25924\*.* %RRS_DEV_ROOT%\cfg\vehicles\IMR_pass_rzd-25924\
xcopy ..\cfg\vehicles\IMR_pass_rzd-21071\*.* %RRS_DEV_ROOT%\cfg\vehicles\IMR_pass_rzd-21071\
xcopy ..\cfg\vehicles\IMR_pass_rzd-65361\*.* %RRS_DEV_ROOT%\cfg\vehicles\IMR_pass_rzd-65361\
xcopy ..\cfg\vehicles\Fr_hopper_RZD-2851\*.* %RRS_DEV_ROOT%\cfg\vehicles\Fr_hopper_RZD-2851\

xcopy ..\cfg\trains\vl60pk-1543.xml %RRS_DEV_ROOT%\cfg\trains\
xcopy ..\cfg\trains\vl60pk-1543-pass-train.xml %RRS_DEV_ROOT%\cfg\trains\
xcopy ..\cfg\trains\VL60k-1737.xml %RRS_DEV_ROOT%\cfg\trains\
xcopy ..\cfg\trains\VL60k-1737-frTest.xml %RRS_DEV_ROOT%\cfg\trains\

rem Копируем движок OSG

xcopy %OSG_BIN_PATH%\libOpenThreads.dll %RRS_DEV_ROOT%\bin\
xcopy %OSG_BIN_PATH%\libosg.dll %RRS_DEV_ROOT%\bin\
xcopy %OSG_BIN_PATH%\libosgAnimation.dll %RRS_DEV_ROOT%\bin\
xcopy %OSG_BIN_PATH%\libosgDB.dll %RRS_DEV_ROOT%\bin\
xcopy %OSG_BIN_PATH%\libosgGA.dll %RRS_DEV_ROOT%\bin\
xcopy %OSG_BIN_PATH%\libosgText.dll %RRS_DEV_ROOT%\bin\
xcopy %OSG_BIN_PATH%\libosgUtil.dll %RRS_DEV_ROOT%\bin\
xcopy %OSG_BIN_PATH%\libosgViewer.dll %RRS_DEV_ROOT%\bin\
xcopy %OSG_BIN_PATH%\libosgQt5.dll %RRS_DEV_ROOT%\bin\

xcopy %OSG_BIN_PATH%\%OSG_PLUGINS_DIR%\*.dll %RRS_DEV_ROOT%\bin\%OSG_PLUGINS_DIR%\
del /S %RRS_DEV_ROOT%\bin\%OSG_PLUGINS_DIR%\*d.dll
xcopy %OSG_BIN_PATH%\%OSG_PLUGINS_DIR%\*dmd.dll %RRS_DEV_ROOT%\bin\%OSG_PLUGINS_DIR%\

rem Копируем прочие необходимые DLL

xcopy %OPENAL_BIN%\*.dll %RRS_DEV_ROOT%\bin\
xcopy %FREETYPE_LIBRARY% %RRS_DEV_ROOT%\bin\
xcopy %ZLIB_LIBRARY% %RRS_DEV_ROOT%\bin\

rem Копируем данные игры (модели, звуки, конфиги анимаций)

rem ВЛ60пк
xcopy /S ..\..\data\models\VL60pk-1543\*.* %RRS_DEV_ROOT%\data\models\VL60pk-1543\
xcopy /S ..\..\data\sounds\vl60\*.* %RRS_DEV_ROOT%\data\sounds\vl60\
xcopy /S ..\..\data\animations\vl60pk-1543\*.* %RRS_DEV_ROOT%\data\animations\vl60pk-1543\

rem ВЛ60к
xcopy /S ..\..\data\models\VL60k-1737\*.* %RRS_DEV_ROOT%\data\models\VL60k-1737\
xcopy /S ..\..\data\animations\VL60k-1737\*.* %RRS_DEV_ROOT%\data\animations\VL60k-1737\

rem Пассажирские вагоны
xcopy /S ..\..\data\models\IMR_pass_rzd\*.* %RRS_DEV_ROOT%\data\models\IMR_pass_rzd\
xcopy /S ..\..\data\animations\IMR_pass_rzd-11100\*.* %RRS_DEV_ROOT%\data\animations\IMR_pass_rzd-11100\
xcopy /S ..\..\data\animations\IMR_pass_rzd-13819\*.* %RRS_DEV_ROOT%\data\animations\IMR_pass_rzd-13819\
xcopy /S ..\..\data\animations\IMR_pass_rzd-16733\*.* %RRS_DEV_ROOT%\data\animations\IMR_pass_rzd-16733\
xcopy /S ..\..\data\animations\IMR_pass_rzd-17669\*.* %RRS_DEV_ROOT%\data\animations\IMR_pass_rzd-17669\
xcopy /S ..\..\data\animations\IMR_pass_rzd-25924\*.* %RRS_DEV_ROOT%\data\animations\IMR_pass_rzd-25924\
xcopy /S ..\..\data\animations\IMR_pass_rzd-21071\*.* %RRS_DEV_ROOT%\data\animations\IMR_pass_rzd-21071\
xcopy /S ..\..\data\animations\IMR_pass_rzd-15563\*.* %RRS_DEV_ROOT%\data\animations\IMR_pass_rzd-15563\


rem Вагоны-хоперы
xcopy /S ..\..\data\models\FrWag_hopper_1\*.* %RRS_DEV_ROOT%\data\models\FrWag_hopper_1\
xcopy /S ..\..\data\animations\Fr_hopper_RZD-2851\*.* %RRS_DEV_ROOT%\data\animations\Fr_hopper_RZD-2851\

rem Шрифты вьювера и темы оформления лаунчера

xcopy ..\..\fonts\*.* %RRS_DEV_ROOT%\fonts
xcopy ..\themes\*.* %RRS_DEV_ROOT%\themes

rem Копируем маршруты

xcopy /S ..\..\routes\experimental-polygon\*.* %RRS_DEV_ROOT%\routes\experimental-polygon\
rmdir /S /Q %RRS_DEV_ROOT%\routes\experimental-polygon\map_editor
del /S %RRS_DEV_ROOT%\routes\experimental-polygon\~*.*

rem Копируем SDK

xcopy /Q ..\CfgReader\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q ..\filesystem\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q ..\simulator\solver\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q ..\simulator\physics\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q ..\simulator\vehicle\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q ..\simulator\device\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q ..\viewer\display\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /S ..\..\sdk\examples\*.* %RRS_DEV_ROOT%\sdk\examples\

rem Копируем иконку

xcopy ..\launcher\resources\images\RRS_logo.ico %RRS_DEV_ROOT%\bin\

rem Копируем лицензию

xcopy ..\LICENSE %RRS_DEV_ROOT%\
xcopy ..\LICENSE-Russian %RRS_DEV_ROOT%\

rem Копируем документацию

xcopy /S ..\docs\*.* %RRS_DEV_ROOT%\docs\

rem Генерируем рантайм Qt

cd %RRS_DEV_ROOT%\bin
windeployqt %RRS_DEV_ROOT%\bin\launcher.exe
windeployqt %RRS_DEV_ROOT%\bin\launcher2.exe --qmldir ..\launcher2\qml
windeployqt %RRS_DEV_ROOT%\bin\simulator.exe
windeployqt %RRS_DEV_ROOT%\bin\viewer.exe
windeployqt %RRS_DEV_ROOT%\bin\pathconv.exe
windeployqt %RRS_DEV_ROOT%\bin\profconv.exe
windeployqt %RRS_DEV_ROOT%\bin\routeconv.exe
windeployqt %RRS_DEV_ROOT%\bin\CfgReader.dll
windeployqt %RRS_DEV_ROOT%\bin\TcpConnection.dll