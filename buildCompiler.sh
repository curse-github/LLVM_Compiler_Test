rm -R out
mkdir out
clang++ -fdeclspec -emit-llvm -S ./lib/cppStdLib.cpp -o ./out/cppStdLib.ll
clang++ ./out/cppStdLib.ll -c -o ./out/cppStdLib.o
clang++ ./lib/stdLibLin.ll -c -o ./out/stdLib.o
includedir="$(llvm-config --includedir)"
libdir="$(llvm-config --libdir)"
libs="$(llvm-config --link-static --ldflags --libs core bitwriter irreader profiledata object support debuginfodwarf textapi)"
clang++ -fdeclspec -std=c++23 -O3 ./src/compiler.cpp -L$libdir $libs -I$includedir -I./include -o ./out/compiler.out
./out/compiler.out module
clang++ -O3 ./out/module.ll ./out/cppStdLib.o ./out/stdLib.o -o ./out/module.out