template<typename T>
struct OwnedPointer {
    T* value;
    OwnedPointer(T* _value = nullptr) : value(_value) {};
    OwnedPointer(const OwnedPointer<T>& copy) = delete;
    OwnedPointer& operator=(const OwnedPointer<T>& copy) = delete;
    OwnedPointer(OwnedPointer<T>&& move) : value(move.value) {
        move.value = nullptr;
    };
    OwnedPointer& operator=(OwnedPointer<T>&& move) {
        value = move.value;
        move.value = nullptr;
        return *this;
    }
    ~OwnedPointer() { if (value != nullptr) delete value; };
    OwnedPointer& operator=(T* _value) {
        if (value != nullptr) delete value;
        value = _value;
        return *this;
    };
    T* operator->() { return value; }
    const T* operator->() const { return value; }
    operator T*() { return value; }
    operator const T*() const { return value; }
    operator T&() { return *value; }
    operator const T&() const { return *value; }
};
#include <unordered_map>
#include <iostream>

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
    std::string name;
    llvm::Function* function;
    std::unordered_map<std::string, llvm::BasicBlock*> blocks{};
    std::unordered_map<std::string, llvm::Value*> vars{};
    bool hasBody = false;
    public:

    FunctionWrapper(ModuleWrapper* _module, llvm::Type* returnType, const std::string& _name, const std::unordered_map<std::string, llvm::Type*>& arguments, const bool& _hasBody);
    FunctionWrapper(const FunctionWrapper& copy) = delete;
    FunctionWrapper& operator=(const FunctionWrapper& copy) = delete;
    FunctionWrapper(FunctionWrapper&& move) = delete;
    FunctionWrapper& operator=(FunctionWrapper&& move) = delete;
    ~FunctionWrapper();
    void insertUnaryOperator(const std::string& varName, const llvm::Instruction::UnaryOps& op, llvm::Value* A, const std::string& blockName="");
    void insertBinaryOperator(const std::string& varName, const llvm::Instruction::BinaryOps& op, llvm::Value* A, llvm::Value* B, const std::string& blockName="");

    void insertGetElementPtr(const std::string& varName, llvm::Type* varType, const std::string& ptr, const std::initializer_list<llvm::Value*>& indices, const std::string& blockName="");
    void insertLoad(const std::string& varName, llvm::Type* ptrType, const std::string& ptr, const std::string& blockName="");
    void insertStore(const std::string& varName, llvm::Value* ptr, const std::string& blockName="");
    void insertCall(const std::string& varName, const std::string& function, std::initializer_list<llvm::Value*> arguments={});
    void insertCall(const std::string& function, std::initializer_list<llvm::Value*> arguments={});
    void insertReturn(llvm::Value* value, const std::string& blockName="");

    llvm::Value* getVar(const std::string& name);
private:
    void insert(const std::string& varName, llvm::Instruction* instr, const std::string& blockName="");
    llvm::Instruction* insert(llvm::Instruction* instr, const std::string& blockName="");
    llvm::Instruction* call(const std::string& varName, std::initializer_list<llvm::Value*> arguments={});
    llvm::Instruction* call(std::initializer_list<llvm::Value*> arguments={});
};
class ModuleWrapper {
    llvm::LLVMContext Context;
    llvm::Module *M;
    public:
    llvm::Type* void_t;
    llvm::Type* ptr_t;
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

    llvm::Type* fp16_t;
    llvm::Type* fp32_t;
    llvm::Type* fp64_t;
    llvm::Type* fp128_t;

    std::string name;
    std::unordered_map<std::string, OwnedPointer<FunctionWrapper>> functions{};

    ModuleWrapper(const std::string& _name);
    ModuleWrapper(const ModuleWrapper& copy) = delete;
    ModuleWrapper& operator=(const ModuleWrapper& copy) = delete;
    ModuleWrapper(ModuleWrapper&& move) = delete;
    ModuleWrapper& operator=(ModuleWrapper&& move) = delete;
    ~ModuleWrapper();

    FunctionWrapper& createFunction(llvm::Type* returnType, const std::string& name, const std::unordered_map<std::string, llvm::Type*>& arguments, const bool& hasBody);
    FunctionWrapper& getFunction(const std::string& name);
    
    friend FunctionWrapper;
};
using namespace llvm;