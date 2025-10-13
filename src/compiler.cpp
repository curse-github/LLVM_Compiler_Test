#include "compiler.h"

FunctionWrapper::FunctionWrapper(ModuleWrapper* _module, llvm::Type* returnType, const std::string& _name, const std::unordered_map<std::string, llvm::Type*>& arguments, const bool& _hasBody) : module(_module), name(_name), hasBody(_hasBody) {
    std::vector<std::string> argumentNames(arguments.size());
    std::vector<llvm::Type*> argumentTypes(arguments.size(), nullptr);
    size_t i = 0;
    for(const std::pair<const std::string, llvm::Type*>& argument : arguments) {
        argumentNames[i] = argument.first;
        argumentTypes[i] = argument.second;
        i++;
    }
    func_t = llvm::FunctionType::get(returnType, llvm::ArrayRef<llvm::Type*>(argumentTypes.data(), argumentTypes.data()+argumentTypes.size()), false);
    function = llvm::Function::Create(func_t, llvm::Function::ExternalLinkage, name, module->M);
    i = 0;
    for (llvm::Argument& arg : function->args()) {
        arg.setName(argumentNames[i]);
        vars[argumentNames[i]] = &arg;
        i++;
    }
    if (hasBody) blocks.emplace("", llvm::BasicBlock::Create(module->Context, "", function));
}
FunctionWrapper::~FunctionWrapper() {}

void FunctionWrapper::insert(const std::string& varName, llvm::Instruction* instr, const std::string& blockName) {
    if (!hasBody) throw std::runtime_error("");
    llvm::BasicBlock*& block = blocks[blockName];
    instr->insertInto(block, block->end());
    vars[varName] = instr;
}
llvm::Instruction* FunctionWrapper::insert(llvm::Instruction* instr, const std::string& blockName) {
    if (!hasBody) throw std::runtime_error("");
    llvm::BasicBlock*& block = blocks[blockName];
    instr->insertInto(block, block->end());
    return instr;
}
void FunctionWrapper::insertUnaryOperator(const std::string& varName, const llvm::Instruction::UnaryOps& op, llvm::Value* A, const std::string& blockName) {
    insert(varName, UnaryOperator::Create(op, A, varName), blockName);
}
void FunctionWrapper::insertBinaryOperator(const std::string& varName, const llvm::Instruction::BinaryOps& op, llvm::Value* A, llvm::Value* B, const std::string& blockName) {
    insert(varName, BinaryOperator::Create(op, A, B, varName), blockName);
}

void FunctionWrapper::insertGetElementPtr(const std::string& varName, llvm::Type* varType, const std::string& ptr, const std::initializer_list<llvm::Value*>& indices, const std::string& blockName) {
    insert(varName, llvm::GetElementPtrInst::Create(varType, getVar(ptr), indices, llvm::GEPNoWrapFlags::none(), varName, nullptr));
}
void FunctionWrapper::insertLoad(const std::string& varName, llvm::Type* ptrType, const std::string& ptr, const std::string& blockName) {
    vars[varName] = new llvm::LoadInst(ptrType, getVar(ptr), varName, blocks[blockName]);
}
void FunctionWrapper::insertStore(const std::string& varName, llvm::Value* ptr, const std::string& blockName) {
    insert(new StoreInst(getVar(varName), ptr, nullptr), blockName);
}
void FunctionWrapper::insertCall(const std::string& varName, const std::string& function, std::initializer_list<llvm::Value*> arguments) {
    insert(varName, module->getFunction(function).call(varName, arguments));
}
void FunctionWrapper::insertCall(const std::string& function, std::initializer_list<llvm::Value*> arguments) {
    insert(module->getFunction(function).call(arguments));
}
void FunctionWrapper::insertReturn(llvm::Value* value, const std::string& blockName) {
    insert(llvm::ReturnInst::Create(module->Context, value), blockName);
}

llvm::Value* FunctionWrapper::getVar(const std::string& name) {
    if (vars.count(name) == 0) throw std::runtime_error("variable name not found.");
    return vars[name];
}
llvm::Instruction* FunctionWrapper::call(const std::string& varName, std::initializer_list<llvm::Value*> arguments) {
    llvm::CallInst* instr = llvm::CallInst::Create(function, arguments, varName);
    instr->setTailCall();
    return instr;
}
llvm::Instruction* FunctionWrapper::call(std::initializer_list<llvm::Value*> arguments) {
    llvm::CallInst* instr = llvm::CallInst::Create(function, arguments);
    instr->setTailCall();
    return instr;
}



ModuleWrapper::ModuleWrapper(const std::string& _name) : name(_name) {
    void_t = llvm::Type::getVoidTy(Context);
    ptr_t = llvm::PointerType::get(Context, 0);
    label_t = llvm::Type::getLabelTy(Context);

    bool_t = i1_t = llvm::Type::getInt1Ty(Context);
    char_t = i8_t = llvm::Type::getInt8Ty(Context);
    i16_t = llvm::Type::getInt16Ty(Context);
    i32_t = llvm::Type::getInt32Ty(Context);
    i64_t = llvm::Type::getInt64Ty(Context);
    i128_t = llvm::Type::getInt128Ty(Context);
    i256_t = llvm::Type::getIntNTy(Context, 256);
    i512_t = llvm::Type::getIntNTy(Context, 512);

    fp16_t = llvm::Type::getHalfTy(Context);
    fp32_t = llvm::Type::getFloatTy(Context);
    fp64_t = llvm::Type::getDoubleTy(Context);
    fp128_t = llvm::Type::getFP128Ty(Context);
    
    M = new llvm::Module(name, Context);
    M->setTargetTriple(llvm::Triple(llvm::sys::getDefaultTargetTriple()));
}
ModuleWrapper::~ModuleWrapper() {
    std::error_code EC;
    llvm::raw_fd_ostream outFile("./out/" + name + ".ll", EC);
    M->print(outFile, nullptr);
    delete M;
}

FunctionWrapper& ModuleWrapper::createFunction(llvm::Type* returnType, const std::string& name, const std::unordered_map<std::string, llvm::Type*>& arguments, const bool& hasBody) {
    functions[name] = new FunctionWrapper(this, returnType, name, arguments, hasBody);
    return getFunction(name);
}
FunctionWrapper& ModuleWrapper::getFunction(const std::string& name) {
    if (functions.count(name) == 0) throw std::runtime_error("function name not found.");
    return *functions[name];
}

void addStdLib(ModuleWrapper& module) {
    // print functions
    module.createFunction(module.void_t, "printChar", {{"1", module.i32_t}}, false);
    module.createFunction(module.void_t, "printStr", {{"1", module.i32_t}}, false);
    module.createFunction(module.void_t, "printUInt", {{"1", module.i32_t}}, false);
    module.createFunction(module.void_t, "printUInt64", {{"1", module.i32_t}}, false);
    module.createFunction(module.void_t, "printInt", {{"1", module.i32_t}}, false);
    module.createFunction(module.void_t, "printInt64", {{"1", module.i32_t}}, false);
    module.createFunction(module.void_t, "printFloat", {{"1", module.fp32_t}}, false);
    module.createFunction(module.void_t, "printDouble", {{"1", module.fp64_t}}, false);
    // module.createFunction(module.void_t, "printFP128", {{"1", module.fp128_t}}, false);
    module.createFunction(module.void_t, "printUInt128", {{"1", module.i128_t}}, false);
    module.createFunction(module.void_t, "printUInt256", {{"1", module.i256_t}}, false);
    module.createFunction(module.void_t, "printUInt512", {{"1", module.i512_t}}, false);
    // println functions
    module.createFunction(module.void_t, "printlnChar", {{"1", module.i32_t}}, false);
    module.createFunction(module.void_t, "printlnStr", {{"1", module.i32_t}}, false);
    module.createFunction(module.void_t, "printlnUInt", {{"1", module.i32_t}}, false);
    module.createFunction(module.void_t, "printlnUInt64", {{"1", module.i32_t}}, false);
    module.createFunction(module.void_t, "printlnInt", {{"1", module.i32_t}}, false);
    module.createFunction(module.void_t, "printlnInt64", {{"1", module.i32_t}}, false);
    module.createFunction(module.void_t, "printlnFloat", {{"1", module.fp32_t}}, false);
    module.createFunction(module.void_t, "printlnDouble", {{"1", module.fp64_t}}, false);
    module.createFunction(module.void_t, "println", {}, false);
    // module.createFunction(module.void_t, "printlnFP128", {{"1", module.fp128_t}}, false);
    module.createFunction(module.void_t, "printlnUInt128", {{"1", module.i128_t}}, false);
    module.createFunction(module.void_t, "printlnUInt256", {{"1", module.i256_t}}, false);
    module.createFunction(module.void_t, "printlnUInt512", {{"1", module.i512_t}}, false);
    // string parsing functions
    module.createFunction(module.i32_t, "strToInt", {{"1", module.ptr_t}}, false);
    module.createFunction(module.i32_t, "strToUInt", {{"1", module.ptr_t}}, false);
}
int main(int argc, char* argv[]) {
    ModuleWrapper module(argv[1]);
    addStdLib(module);
    FunctionWrapper& mainFunc = module.createFunction(module.i32_t, "main", {{"argc", module.i32_t},{"argv", module.ptr_t}}, true);
    mainFunc.insertGetElementPtr("numStrP", llvm::ArrayType::get(module.ptr_t, 0), "argv", {ConstantInt::get(module.i64_t, 0), ConstantInt::get(module.i64_t, 1)});
    mainFunc.insertLoad("numStr", module.ptr_t, "numStrP");
    mainFunc.insertCall("num", "strToUInt", {mainFunc.getVar("numStr")});
    mainFunc.insertBinaryOperator("output", Instruction::Add, mainFunc.getVar("num"), ConstantInt::get(module.i32_t, 5));
    mainFunc.insertCall("printlnUInt", {mainFunc.getVar("output")});
    mainFunc.insertReturn(ConstantInt::get(module.i32_t, 0));
    return 0;
}
