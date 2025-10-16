rm -R out
mkdir out
clang++-21 -fdeclspec -emit-llvm -S ./lib/cppStdLib.cpp -o ./out/cppStdLib.ll
clang++-21 ./out/cppStdLib.ll -c -o ./out/cppStdLib.o
clang++-21 ./lib/stdLibLin.ll -c -o ./out/stdLib.o
includedir="$(llvm-config-21 --includedir)"
libdir="$(llvm-config-21 --libdir)"
libs="$(llvm-config-21 --link-static --ldflags --libs core bitwriter irreader profiledata object support debuginfodwarf textapi)"
# clang++-21 -fdeclspec -std=c++23 -O3 ./src/simpleCompiler.cpp -L$libdir $libs -I$includedir -I./include -D_LINUX=1 -o ./out/simpleCompiler.out
# ./out/simpleCompiler.out simpleModule
# clang++-21 -O3 ./out/simpleModule.ll ./out/cppStdLib.o ./out/stdLib.o -o ./out/simpleModule.out
clang++-21 -fdeclspec -std=c++23 -O3 ./src/compiler.cpp -L$libdir $libs -I$includedir -I./include -D_LINUX=1 -o ./out/compiler.out
./out/compiler.out module
clang++-21 -O3 ./out/module.ll ./out/cppStdLib.o ./out/stdLib.o -o ./out/module.out