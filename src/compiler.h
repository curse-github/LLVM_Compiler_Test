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
    llvm::FunctionType* func_t;
    std::string name;
    llvm::Function* function;
    std::unordered_map<std::string, llvm::BasicBlock*> blocks{};
    std::unordered_map<std::string, llvm::Value*> vars{};
    public:
    unsigned int lastConditionIndex = 0;

    FunctionWrapper(ModuleWrapper* _module, llvm::Type* returnType, const std::string& _name, const std::initializer_list<std::pair<std::string, llvm::Type*>>& arguments);
    FunctionWrapper(const FunctionWrapper& copy) = delete;
    FunctionWrapper& operator=(const FunctionWrapper& copy) = delete;
    FunctionWrapper(FunctionWrapper&& move) = delete;
    FunctionWrapper& operator=(FunctionWrapper&& move) = delete;
    ~FunctionWrapper();

    llvm::Value* getVarOrNumber(const std::string& str);
    llvm::Value* insertAdd(const std::string& varName, const std::string& A, const std::string& B);
    llvm::Value* insertSub(const std::string& varName, const std::string& A, const std::string& B);
    llvm::Value* insertMul(const std::string& varName, const std::string& A, const std::string& B);
    llvm::Value* insertUDiv(const std::string& varName, const std::string& A, const std::string& B);
    llvm::Value* insertUMod(const std::string& varName, const std::string& A, const std::string& B);
    // accepts:
    //     llvm::CmpInst::Predicate::FCMP_ULT
    //     llvm::CmpInst::Predicate::FCMP_ULE
    //     llvm::CmpInst::Predicate::FCMP_UEQ
    //     llvm::CmpInst::Predicate::FCMP_UGE
    //     llvm::CmpInst::Predicate::FCMP_UGT
    //     llvm::CmpInst::Predicate::FCMP_UNE
    llvm::Value* insertICmp(const std::string& varName, const std::string& A, const llvm::CmpInst::Predicate& op, const std::string& B);
    llvm::Value* insertGetElementPtr(const std::string& varName, llvm::Type* varType, const std::string& ptr, std::initializer_list<const char*> indices);
    llvm::Value* insertLoad(const std::string& varName, llvm::Type* ptrType, const std::string& ptr);
    void insertStore(const std::string& varName, const std::string& ptr);
    llvm::Value* insertBr(const std::string& branchBlockName);
    llvm::Value* insertBr(const std::string& condition, const std::string& trueBranchBlockName, const std::string& falseBranchBlockName);
    llvm::Value* insertCall(const std::string& varName, const std::string& function, std::initializer_list<const char*> arguments={});
    llvm::Value* insertCall(const std::string& function, std::initializer_list<const char*> arguments={});
    llvm::Value* insertReturn(const std::string& value);

    llvm::Value* getVar(const std::string& name);
    void addBlock(const std::string& name);
    llvm::BasicBlock* getBlock(const std::string& name);
    void setActiveBlock(const std::string& name);
private:
    std::string activeBlock = "_____";
    llvm::Instruction* insert(llvm::Instruction* instr);
    llvm::Value* insert(const std::string& varName, llvm::Instruction* instr);
    llvm::Value* insertUnaryOperator(const std::string& varName, const llvm::Instruction::UnaryOps& op, const std::string& A);
    llvm::Value* insertBinaryOperator(const std::string& varName, const llvm::Instruction::BinaryOps& op, const std::string& A, const std::string& B);
    llvm::Instruction* call(const std::string& varName, llvm::ArrayRef<llvm::Value*> arguments={});
    llvm::Instruction* call(llvm::ArrayRef<llvm::Value*> arguments={});
};
class ModuleWrapper {
    llvm::LLVMContext Context;
    llvm::Module *Module;
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