@echo off & setlocal EnableDelayedExpansion

del batch.sql

echo 开始执行...

@set source=\.
for %%i in (sqlscripts\*.sql) do (
　　set file=%%~fi
　　set file=!file:/=/!
　　echo %source% !file!
　　echo %source% !file! >> batch.sql
)

echo 执行完毕!

pause