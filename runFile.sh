rm -R out
mkdir out
clang++ -O3 -fdeclspec -emit-llvm ./lib/compiler.cpp -o ./out/compiler.o
clang++ ./out/compiler.o -o ./out/compiler.out