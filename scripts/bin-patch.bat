rem Копируем бинарные файлы

xcopy /F /Y  ..\..\bin\*.exe %RRS_DEV_ROOT%\bin\
xcopy /F /Y ..\..\lib\*.dll %RRS_DEV_ROOT%\bin\
move %RRS_DEV_ROOT%\bin\rkf5.dll %RRS_DEV_ROOT%\lib\rkf5.dll
move %RRS_DEV_ROOT%\bin\rk4.dll %RRS_DEV_ROOT%\lib\rk4.dll
move %RRS_DEV_ROOT%\bin\euler2.dll %RRS_DEV_ROOT%\lib\euler2.dll
move %RRS_DEV_ROOT%\bin\euler.dll %RRS_DEV_ROOT%\lib\euler.dll

xcopy /F /Y ..\..\modules\*.dll %RRS_DEV_ROOT%\modules
xcopy /F /Y ..\..\modules\vl60k\*.dll %RRS_DEV_ROOT%\modules\vl60k\
xcopy /F /Y ..\..\modules\vl60pk\*.dll %RRS_DEV_ROOT%\modules\vl60pk\
xcopy /F /Y ..\..\modules\passcar\*.dll %RRS_DEV_ROOT%\modules\passcar\
xcopy /F /Y ..\..\modules\freightcar\*.dll %RRS_DEV_ROOT%\modules\freightcar\

xcopy /F /Y ..\..\plugins\*.dll %RRS_DEV_ROOT%\plugins

rem Копируем конфиги

xcopy /F /Y ..\cfg\*.xml %RRS_DEV_ROOT%\cfg\
xcopy /F /Y ..\cfg\couplings\*.xml %RRS_DEV_ROOT%\cfg\couplings\
xcopy /F /Y ..\cfg\devices\*.xml %RRS_DEV_ROOT%\cfg\devices\
xcopy /F /Y ..\cfg\devices\freejoy\*.xml %RRS_DEV_ROOT%\cfg\devices\freejoy\

xcopy /F /Y ..\cfg\main-resist\default.xml %RRS_DEV_ROOT%\cfg\main-resist\
xcopy /F /Y ..\cfg\main-resist\passcar.xml %RRS_DEV_ROOT%\cfg\main-resist\
xcopy /F /Y ..\cfg\main-resist\loco-resist.xml %RRS_DEV_ROOT%\cfg\main-resist\

xcopy /F /Y ..\cfg\wheel-rail-friction\*.xml %RRS_DEV_ROOT%\cfg\wheel-rail-friction\

xcopy /F /Y ..\cfg\vehicles\vl60pk\*.* %RRS_DEV_ROOT%\cfg\vehicles\vl60pk\
xcopy /F /Y ..\cfg\vehicles\vl60k\*.* %RRS_DEV_ROOT%\cfg\vehicles\vl60k\
xcopy /F /Y ..\cfg\vehicles\IMR_pass_rzd\*.* %RRS_DEV_ROOT%\cfg\vehicles\IMR_pass_rzd\
xcopy /F /Y ..\cfg\vehicles\Fr_hopper_RZD\*.* %RRS_DEV_ROOT%\cfg\vehicles\Fr_hopper_RZD\

xcopy /F /Y ..\cfg\trains\vl60pk-1543-T65_17.xml %RRS_DEV_ROOT%\cfg\trains\
xcopy /F /Y ..\cfg\trains\vl60pk-1543.xml %RRS_DEV_ROOT%\cfg\trains\
xcopy /F /Y ..\cfg\trains\VL60k-1737.xml %RRS_DEV_ROOT%\cfg\trains\
xcopy /F /Y ..\cfg\trains\vl60k-1737-frEmpties.xml %RRS_DEV_ROOT%\cfg\trains\
xcopy /F /Y ..\cfg\trains\vl60k-1737-frLoads.xml %RRS_DEV_ROOT%\cfg\trains\

xcopy /F /Y ..\..\plugins\*dmd.dll %RRS_DEV_ROOT%\bin\%OSG_PLUGINS_DIR%\