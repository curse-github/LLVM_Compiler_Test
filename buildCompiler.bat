:: @echo off
del /Q .\out\*
rmdir out
mkdir out
FOR /F "tokens=*" %%g IN ('..\builds\llvm-build\Release\bin\llvm-config.exe --libdir') do (SET libdir=%%g)
FOR /F "tokens=*" %%g IN ('..\builds\llvm-build\Release\bin\llvm-config.exe --link-static --ldflags --libs core bitwriter irreader profiledata object support debuginfodwarf textapi') do (SET libs=%%g)
clang++ -std=c++17 -O3 ./src/compiler.cpp -L%libdir% %libs% -lntdll -I..\builds\llvm-build\include -I.\include -o ./out/compiler.exe
out\compiler.exe testModule
clang++ -O3 ./out/testModule.ll -o ./out/testModule.exe
pause