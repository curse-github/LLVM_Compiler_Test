#include "compiler.h"

FunctionWrapper::FunctionWrapper(ModuleWrapper* _module, llvm::Type* returnType, const std::string& _name, const std::initializer_list<llvm::Type*>& arguments) : module(_module), name(_name) {
    func_t = llvm::FunctionType::get(returnType, llvm::ArrayRef<llvm::Type*>(arguments), false);
    function = llvm::Function::Create(func_t, llvm::Function::ExternalLinkage, name, module->M);
    blocks.emplace("Entry", llvm::BasicBlock::Create(module->Context, "Entry", function));
}
FunctionWrapper::~FunctionWrapper() {}

llvm::Instruction* FunctionWrapper::insert(llvm::Instruction* instr, const std::string& blockName) {
    llvm::BasicBlock*& block = blocks[blockName];
    instr->insertInto(block, block->end());
    return instr;
}


ModuleWrapper::ModuleWrapper(const std::string& _name) : name(_name) {
    void_t = llvm::Type::getVoidTy(Context);
    label_t = llvm::Type::getLabelTy(Context);

    bool_t = i1_t = llvm::Type::getInt1Ty(Context);
    char_t = i8_t = llvm::Type::getInt8Ty(Context);
    i16_t = llvm::Type::getInt16Ty(Context);
    i32_t = llvm::Type::getInt32Ty(Context);
    i64_t = llvm::Type::getInt64Ty(Context);
    i128_t = llvm::Type::getInt128Ty(Context);
    i256_t = llvm::Type::getIntNTy(Context, 256);
    i512_t = llvm::Type::getIntNTy(Context, 512);
    
    f16_t = llvm::Type::getHalfTy(Context);
    f32_t = llvm::Type::getFloatTy(Context);
    f64_t = llvm::Type::getDoubleTy(Context);
    f128_t = llvm::Type::getFP128Ty(Context);
    

    M = new llvm::Module(name, Context);
    M->setTargetTriple(llvm::Triple(llvm::sys::getDefaultTargetTriple()));
    functions["main"] = new FunctionWrapper(this, i32_t, "main", {i32_t, llvm::ArrayType::get(llvm::ArrayType::get(char_t, 0), 0)});
}
ModuleWrapper::~ModuleWrapper() {
    std::error_code EC;
    llvm::raw_fd_ostream outFile("./out/" + name + ".ll", EC);
    M->print(outFile, nullptr);
    for (std::pair<std::string, FunctionWrapper*> function : functions)
        delete function.second;
    delete M;
}

FunctionWrapper& ModuleWrapper::getFunction(const std::string& name) {
    return *functions[name];
}
llvm::Instruction* ModuleWrapper::createReturn(llvm::Instruction* instr) {
    return llvm::ReturnInst::Create(Context, instr);
}

int main() {
    ModuleWrapper module("testModule");
    FunctionWrapper& mainFunc = module.getFunction("main");
    Value *Two = ConstantInt::get(module.i32_t, 2);
    Value *Three = ConstantInt::get(module.i32_t, 3);
    Instruction* Add = BinaryOperator::Create(Instruction::Add, Two, Three, "five");
    mainFunc.insert(Add, "Entry");
    mainFunc.insert(module.createReturn(Add), "Entry");
    return 0;
}
