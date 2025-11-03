@echo off & setlocal EnableDelayedExpansion

del auth_batch.sql
del characters_batch.sql
del world_batch.sql

echo start...

@set source=\.
for %%i in (auth\*.sql) do (
  set file=%%~fi
  set file=!file:/=/!
  echo %source% !file!
  echo %source% !file! >> auth_batch.sql
)

for %%i in (characters\*.sql) do (
  set file=%%~fi
  set file=!file:/=/!
  echo %source% !file!
  echo %source% !file! >> characters_batch.sql
)

for %%i in (world\*.sql) do (
  set file=%%~fi
  set file=!file:/=/!
  echo %source% !file!
  echo %source% !file! >> world_batch.sql
)

echo done!

pause