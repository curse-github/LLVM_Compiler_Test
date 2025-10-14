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
    public:

    FunctionWrapper(ModuleWrapper* _module, llvm::Type* returnType, const std::string& _name, const std::initializer_list<std::pair<std::string, llvm::Type*>>& arguments);
    FunctionWrapper(const FunctionWrapper& copy) = delete;
    FunctionWrapper& operator=(const FunctionWrapper& copy) = delete;
    FunctionWrapper(FunctionWrapper&& move) = delete;
    FunctionWrapper& operator=(FunctionWrapper&& move) = delete;
    ~FunctionWrapper();
    void insertUnaryOperator(const std::string& varName, const llvm::Instruction::UnaryOps& op, llvm::Value* A);
    void insertBinaryOperator(const std::string& varName, const llvm::Instruction::BinaryOps& op, llvm::Value* A, llvm::Value* B);

    // accepts:
    //     llvm::CmpInst::Predicate::FCMP_ULT
    void insertICmp(const std::string& varName, llvm::Value* A, const llvm::CmpInst::Predicate& op, llvm::Value* B);
    void insertGetElementPtr(const std::string& varName, llvm::Type* varType, const std::string& ptr, const std::initializer_list<llvm::Value*>& indices);
    void insertLoad(const std::string& varName, llvm::Type* ptrType, const std::string& ptr);
    void insertStore(const std::string& varName, const std::string& ptr);
    void insertBr(const std::string& branchBlockName);
    void insertBr(llvm::Value* condition, const std::string& trueBranchBlockName, const std::string& falseBranchBlockName);
    void insertCall(const std::string& varName, const std::string& function, std::initializer_list<llvm::Value*> arguments={});
    void insertCall(const std::string& function, std::initializer_list<llvm::Value*> arguments={});
    void insertReturn(llvm::Value* value);

    llvm::Value* getVar(const std::string& name);
    void addBlock(const std::string& name);
    llvm::BasicBlock* getBlock(const std::string& name);
    std::string activeBlock = "_____";
    void setActiveBlock(const std::string& name);
private:
    void insert(const std::string& varName, llvm::Instruction* instr);
    llvm::Instruction* insert(llvm::Instruction* instr);
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
    std::unordered_map<std::string, llvm::Value*> globals{};
    std::unordered_map<unsigned int, llvm::Value*> i32constants{};
    std::unordered_map<unsigned long long int, llvm::Value*> i64constants{};

    ModuleWrapper(const std::string& _name);
    ModuleWrapper(const ModuleWrapper& copy) = delete;
    ModuleWrapper& operator=(const ModuleWrapper& copy) = delete;
    ModuleWrapper(ModuleWrapper&& move) = delete;
    ModuleWrapper& operator=(ModuleWrapper&& move) = delete;
    ~ModuleWrapper();

    FunctionWrapper& createFunction(llvm::Type* returnType, const std::string& name, const std::initializer_list<std::pair<std::string, llvm::Type*>>& arguments);
    FunctionWrapper& getFunction(const std::string& name);
    void createGlobalStr(const std::string& varName, const std::string& value);
    llvm::Value* getVar(const std::string& name);
    llvm::Value* getI32(const int& i);
    llvm::Value* getI64(const long long int& i);
    llvm::Value* getUI32(const unsigned int& i);
    llvm::Value* getUI64(const unsigned long long int& i);
    
    friend FunctionWrapper;
};
using namespace llvm;