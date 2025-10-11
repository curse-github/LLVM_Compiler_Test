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
    llvm::FunctionType *func_t;
    llvm::Function* function;
    public:
    std::string name;
    std::unordered_map<std::string, llvm::BasicBlock*> blocks{};

    FunctionWrapper(ModuleWrapper* _module, llvm::Type* returnType, const std::string& _name, const std::initializer_list<llvm::Type*>& arguments);
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
    llvm::Type* void_t;
    llvm::Type* label_t;

    llvm::Type* bool_t;
    llvm::Type* i1_t;
    llvm::Type* char_t;
    llvm::Type* i8_t;
    llvm::Type* i16_t;
    llvm::Type* i32_t;
    llvm::Type* i64_t;
    llvm::Type* i128_t;
    llvm::Type* i256_t;
    llvm::Type* i512_t;

    llvm::Type* f16_t;
    llvm::Type* f32_t;
    llvm::Type* f64_t;
    llvm::Type* f128_t;

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