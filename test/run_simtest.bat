cmake --build build
copy /y /b %~dp0\..\build\simtest_engine.exe %~dp0\simulator\simtest_engine\
copy /y /b %~dp0\..\build\yaml-cppd.dll %~dp0\simulator\simtest_engine\
copy /y /b %~dp0\..\build\simtest_config.exe %~dp0\simulator\
copy /y /b %~dp0\..\build\yaml-cppd.dll %~dp0\simulator\
%~dp0\simulator\simtest_engine\simtest_engine.exe
%~dp0\simulator\simtest_config.exe

::cmake --build build && copy /y /b build\gcloud_engine.exe stockfish-belgium-c2d\stockfish-belgium-c2d && stockfish-belgium-c2d\stockfish-belgium-c2d\gcloud_engine.exe
