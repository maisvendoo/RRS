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

rem Копируем бинарные файлы

xcopy  ..\..\bin\*.exe %RRS_DEV_ROOT%\bin\
xcopy ..\..\lib\*.dll %RRS_DEV_ROOT%\bin\
move %RRS_DEV_ROOT%\bin\rkf5.dll %RRS_DEV_ROOT%\lib\rkf5.dll
move %RRS_DEV_ROOT%\bin\rk4.dll %RRS_DEV_ROOT%\lib\rk4.dll
move %RRS_DEV_ROOT%\bin\euler2.dll %RRS_DEV_ROOT%\lib\euler2.dll
move %RRS_DEV_ROOT%\bin\euler.dll %RRS_DEV_ROOT%\lib\euler.dll

xcopy ..\..\modules\*.dll %RRS_DEV_ROOT%\modules
xcopy ..\..\modules\vl60\*.dll %RRS_DEV_ROOT%\modules\vl60\
xcopy ..\..\modules\passcar\*.dll %RRS_DEV_ROOT%\modules\passcar\
xcopy ..\..\modules\freightcar\*.dll %RRS_DEV_ROOT%\modules\freightcar\

xcopy ..\..\plugins\*.dll %RRS_DEV_ROOT%\plugins

rem Копируем конфиги

xcopy ..\cfg\*.xml %RRS_DEV_ROOT%\cfg\
del %RRS_DEV_ROOT%\cfg\control-panel.xml
xcopy ..\cfg\couplings\*.xml %RRS_DEV_ROOT%\cfg\couplings\
xcopy ..\cfg\devices\*.xml %RRS_DEV_ROOT%\cfg\devices\

xcopy ..\cfg\main-resist\default.xml %RRS_DEV_ROOT%\cfg\main-resist\
xcopy ..\cfg\main-resist\passcar.xml %RRS_DEV_ROOT%\cfg\main-resist\
xcopy ..\cfg\main-resist\loco-resist.xml %RRS_DEV_ROOT%\cfg\main-resist\

xcopy ..\cfg\wheel-rail-friction\*.xml %RRS_DEV_ROOT%\cfg\wheel-rail-friction\

xcopy ..\cfg\vehicles\vl60pk-1543\*.* %RRS_DEV_ROOT%\cfg\vehicles\vl60pk-1543\
xcopy ..\cfg\vehicles\IMR_pass_rzd-11100\*.* %RRS_DEV_ROOT%\cfg\vehicles\IMR_pass_rzd-11100\
xcopy ..\cfg\vehicles\IMR_pass_rzd-13819\*.* %RRS_DEV_ROOT%\cfg\vehicles\IMR_pass_rzd-13819\
xcopy ..\cfg\vehicles\IMR_pass_rzd-16733\*.* %RRS_DEV_ROOT%\cfg\vehicles\IMR_pass_rzd-16733\
xcopy ..\cfg\vehicles\IMR_pass_rzd-17669\*.* %RRS_DEV_ROOT%\cfg\vehicles\IMR_pass_rzd-17669\
xcopy ..\cfg\vehicles\IMR_pass_rzd-25924\*.* %RRS_DEV_ROOT%\cfg\vehicles\IMR_pass_rzd-25924\

xcopy ..\cfg\trains\vl60pk-1543.xml %RRS_DEV_ROOT%\cfg\trains\
xcopy ..\cfg\trains\vl60pk-1543-pass-train.xml %RRS_DEV_ROOT%\cfg\trains\
xcopy ..\cfg\trains\VL60k-1737.xml %RRS_DEV_ROOT%\cfg\trains\
xcopy ..\cfg\trains\vl60k-1737-frEmpties.xml %RRS_DEV_ROOT%\cfg\trains\
xcopy ..\cfg\trains\vl60k-1737-frLoads.xml %RRS_DEV_ROOT%\cfg\trains\

