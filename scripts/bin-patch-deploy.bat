rem Копируем бинарные файлы

xcopy /Y  ..\..\bin\*.exe %RRS_DEV_ROOT%\bin\
xcopy /Y ..\..\lib\*.dll %RRS_DEV_ROOT%\bin\
move %RRS_DEV_ROOT%\bin\rkf5.dll %RRS_DEV_ROOT%\lib\rkf5.dll
move %RRS_DEV_ROOT%\bin\rk4.dll %RRS_DEV_ROOT%\lib\rk4.dll
move %RRS_DEV_ROOT%\bin\euler2.dll %RRS_DEV_ROOT%\lib\euler2.dll
move %RRS_DEV_ROOT%\bin\euler.dll %RRS_DEV_ROOT%\lib\euler.dll

xcopy /Y  ..\..\modules\*.dll %RRS_DEV_ROOT%\modules
xcopy /Y  ..\..\modules\freightcar\*.dll %RRS_DEV_ROOT%\modules\freightcar\
xcopy /Y  ..\..\modules\passcar\*.dll %RRS_DEV_ROOT%\modules\passcar\
xcopy /Y  ..\..\modules\vl60k\*.dll %RRS_DEV_ROOT%\modules\vl60k\
xcopy /Y  ..\..\modules\vl60pk\*.dll %RRS_DEV_ROOT%\modules\vl60pk\

xcopy /Y  ..\..\plugins\*.dll %RRS_DEV_ROOT%\plugins

rem Копируем SDK

xcopy /Q /Y  ..\CfgReader\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q /Y  ..\filesystem\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q /Y  ..\simulator\solver\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q /Y  ..\simulator\physics\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q /Y  ..\simulator\vehicle\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q /Y  ..\simulator\device\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q /Y  ..\viewer\display\include\*.h %RRS_DEV_ROOT%\sdk\include\