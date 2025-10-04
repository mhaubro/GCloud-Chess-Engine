cmake --build build
:: Move to location of unittest .cpp file for convenience
copy /y /b %~dp0\..\build\unittest.exe %~dp0\unit
%~dp0\unit\unittest.exe
