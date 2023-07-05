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
xcopy ..\..\modules\chs2t\*.dll %RRS_DEV_ROOT%\modules\chs2t\
xcopy ..\..\modules\freightcar\*.dll %RRS_DEV_ROOT%\modules\freightcar\
xcopy ..\..\modules\passcar\*.dll %RRS_DEV_ROOT%\modules\passcar\
xcopy ..\..\modules\tep70\*.dll %RRS_DEV_ROOT%\modules\tep70\
xcopy ..\..\modules\vl60k\*.dll %RRS_DEV_ROOT%\modules\vl60k\
xcopy ..\..\modules\vl60pk\*.dll %RRS_DEV_ROOT%\modules\vl60pk\

xcopy ..\..\plugins\*.dll %RRS_DEV_ROOT%\plugins

rem Копируем SDK

xcopy /Q ..\CfgReader\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q ..\filesystem\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q ..\simulator\solver\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q ..\simulator\physics\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q ..\simulator\vehicle\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q ..\simulator\device\include\*.h %RRS_DEV_ROOT%\sdk\include\
xcopy /Q ..\viewer\display\include\*.h %RRS_DEV_ROOT%\sdk\include\
