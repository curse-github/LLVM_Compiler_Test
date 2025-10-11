#include <unordered_map>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/TargetParser/Triple.h"

#include "llvm/Support/raw_ostream.h"

class ModuleWrapper;
class FunctionWrapper {
    ModuleWrapper* module;
    llvm::FunctionType *type;
    llvm::Function* function;
    public:
    std::string name;
    std::unordered_map<std::string, llvm::BasicBlock*> blocks{};

    FunctionWrapper(ModuleWrapper* _module, llvm::FunctionType* _type, const std::string& _name);
    FunctionWrapper(const FunctionWrapper& copy) = delete;
    FunctionWrapper& operator=(const FunctionWrapper& copy) = delete;
    FunctionWrapper(FunctionWrapper&& move) = delete;
    FunctionWrapper& operator=(FunctionWrapper&& move) = delete;
    ~FunctionWrapper();
    
    llvm::Instruction* insert(llvm::Instruction* instr, const std::string& block);
};
class ModuleWrapper {
    llvm::LLVMContext Context;
    llvm::Module *M;
    public:
    llvm::IntegerType* i1_t;
    llvm::IntegerType* i8_t;
    llvm::IntegerType* i16_t;
    llvm::IntegerType* i32_t;
    llvm::IntegerType* i64_t;
    llvm::IntegerType* i128_t;
    llvm::IntegerType* i256_t;
    llvm::IntegerType* i512_t;
    llvm::IntegerType* bool_t;
    llvm::IntegerType* char_t;

    std::string name;
    std::unordered_map<std::string, FunctionWrapper*> functions{};

    ModuleWrapper(const std::string& _name);
    ModuleWrapper(const ModuleWrapper& copy) = delete;
    ModuleWrapper& operator=(const ModuleWrapper& copy) = delete;
    ModuleWrapper(ModuleWrapper&& move) = delete;
    ModuleWrapper& operator=(ModuleWrapper&& move) = delete;
    ~ModuleWrapper();

    FunctionWrapper& getFunction(const std::string& name);
    llvm::Instruction* createReturn(llvm::Instruction* instr);
    
    friend FunctionWrapper;
};
using namespace llvm;