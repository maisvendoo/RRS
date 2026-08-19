rem Копируем бинарные файлы

xcopy /Y  ..\..\bin\*.exe %RRS_DEV_ROOT%\bin\
xcopy /Y  ..\..\lib\*.dll %RRS_DEV_ROOT%\bin\
move %RRS_DEV_ROOT%\bin\rkf5.dll %RRS_DEV_ROOT%\lib\
move %RRS_DEV_ROOT%\bin\rk4.dll %RRS_DEV_ROOT%\lib\
move %RRS_DEV_ROOT%\bin\euler2.dll %RRS_DEV_ROOT%\lib\
move %RRS_DEV_ROOT%\bin\euler.dll %RRS_DEV_ROOT%\lib\

rem Копируем модули

xcopy /Y ..\..\modules\*.dll %RRS_DEV_ROOT%\modules
xcopy /Y ..\..\modules\vl60\*.dll %RRS_DEV_ROOT%\modules\vl60\
xcopy /Y ..\..\modules\passcar\*.dll %RRS_DEV_ROOT%\modules\passcar\
xcopy /Y ..\..\modules\freightcar\*.dll %RRS_DEV_ROOT%\modules\freightcar\
xcopy /Y ..\lua\*.lua %RRS_DEV_ROOT%\modules\lua\

rem Копируем плагины

xcopy /Y ..\..\plugins\*.dll %RRS_DEV_ROOT%\plugins

xcopy /Y %OPENAL_BIN%\*.dll %RRS_DEV_ROOT%\bin\

rem Генерируем рантайм Qt

cd %RRS_DEV_ROOT%\bin
windeployqt %RRS_DEV_ROOT%\bin\launcher.exe
windeployqt %RRS_DEV_ROOT%\bin\simulator.exe
windeployqt %RRS_DEV_ROOT%\bin\viewer.exe
windeployqt %RRS_DEV_ROOT%\bin\pathconv.exe
windeployqt %RRS_DEV_ROOT%\bin\profconv.exe
windeployqt %RRS_DEV_ROOT%\bin\routeconv.exe
windeployqt %RRS_DEV_ROOT%\bin\CfgReader.dll
