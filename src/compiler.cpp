#include "compiler.h"

FunctionWrapper::FunctionWrapper(ModuleWrapper* _module, llvm::Type* returnType, const std::string& _name, const std::initializer_list<std::pair<std::string, llvm::Type*>>& arguments) : module(_module), name(_name) {
    std::vector<std::string> argumentNames(arguments.size());
    std::vector<llvm::Type*> argumentTypes(arguments.size(), nullptr);
    size_t i = 0;
    for(const std::pair<const std::string, llvm::Type*>& argument : arguments) {
        argumentNames[i] = argument.first;
        argumentTypes[i] = argument.second;
        i++;
    }
    func_t = llvm::FunctionType::get(returnType, llvm::ArrayRef<llvm::Type*>(argumentTypes.data(), argumentTypes.data()+argumentTypes.size()), false);
    function = llvm::Function::Create(func_t, llvm::Function::ExternalLinkage, name, module->Module);
    i = 0;
    for (llvm::Argument& arg : function->args()) {
        arg.setName(argumentNames[i]);
        vars[argumentNames[i]] = &arg;
        i++;
    }
}
FunctionWrapper::~FunctionWrapper() {}

llvm::Value* FunctionWrapper::getVarOrNumber(const std::string& str) {
    return getVar(str);
}
llvm::Value* FunctionWrapper::insertAdd(const std::string& varName, const std::string& A, const std::string& B) {
    return insertBinaryOperator(varName, llvm::Instruction::BinaryOps::Add, A, B);
}
llvm::Value* FunctionWrapper::insertSub(const std::string& varName, const std::string& A, const std::string& B) {
    return insertBinaryOperator(varName, llvm::Instruction::BinaryOps::Sub, A, B);
}
llvm::Value* FunctionWrapper::insertMul(const std::string& varName, const std::string& A, const std::string& B) {
    return insertBinaryOperator(varName, llvm::Instruction::BinaryOps::Mul, A, B);
}
llvm::Value* FunctionWrapper::insertUDiv(const std::string& varName, const std::string& A, const std::string& B) {
    return insertBinaryOperator(varName, llvm::Instruction::BinaryOps::UDiv, A, B);
}
llvm::Value* FunctionWrapper::insertUMod(const std::string& varName, const std::string& A, const std::string& B) {
    return insertBinaryOperator(varName, llvm::Instruction::BinaryOps::URem, A, B);
}
llvm::Value* FunctionWrapper::insertICmp(const std::string& varName, const std::string& A, const llvm::CmpInst::Predicate& op, const std::string& B) {
    return insert(varName, llvm::ICmpInst::Create(llvm::Instruction::OtherOps::ICmp, op, getVarOrNumber(A), getVarOrNumber(B), varName));
}
llvm::Value* FunctionWrapper::insertGetElementPtr(const std::string& varName, llvm::Type* varType, const std::string& ptr, std::initializer_list<const char*> indices) {
    std::vector<llvm::Value*> indicesValues;
    indicesValues.reserve(indices.size()+1);
    indicesValues.push_back(module->getI64(0));
    for (const std::string& index : indices)
        indicesValues.push_back(getVarOrNumber(index));
    return insert(varName, llvm::GetElementPtrInst::Create(varType, getVar(ptr), indicesValues, llvm::GEPNoWrapFlags::none(), varName, nullptr));
}
llvm::Value* FunctionWrapper::insertLoad(const std::string& varName, llvm::Type* ptrType, const std::string& ptr) {
    return vars[varName] = new llvm::LoadInst(ptrType, getVar(ptr), varName, getBlock(activeBlock));
}
void FunctionWrapper::insertStore(const std::string& varName, const std::string& ptr) {
    insert(new llvm::StoreInst(getVar(varName), getVar(ptr), nullptr));
}
llvm::Value* FunctionWrapper::insertBr(const std::string& branchBlockName) {
    return insert(llvm::BranchInst::Create(getBlock(branchBlockName)));
}
llvm::Value* FunctionWrapper::insertBr(const std::string& condition, const std::string& trueBranchBlockName, const std::string& falseBranchBlockName) {
    return insert(llvm::BranchInst::Create(getBlock(trueBranchBlockName), getBlock(falseBranchBlockName), getVarOrNumber(condition)));
}
llvm::Value* FunctionWrapper::insertCall(const std::string& varName, const std::string& function, std::initializer_list<const char*> arguments) {
    std::vector<llvm::Value*> argumentValues;
    argumentValues.reserve(arguments.size());
    for (const std::string& arg : arguments)
        argumentValues.push_back(getVarOrNumber(arg));
    return insert(varName, module->getFunction(function).call(varName, argumentValues));
}
llvm::Value* FunctionWrapper::insertCall(const std::string& function, std::initializer_list<const char*> arguments) {
    std::vector<llvm::Value*> argumentValues;
    argumentValues.reserve(arguments.size());
    for (const std::string& arg : arguments)
        argumentValues.push_back(getVarOrNumber(arg));
    return insert(module->getFunction(function).call(argumentValues));
}
llvm::Value* FunctionWrapper::insertReturn(const std::string& value) {
    return insert(llvm::ReturnInst::Create(module->Context, getVarOrNumber(value)));
}

llvm::Value* FunctionWrapper::getVar(const std::string& name) {
    try {
        return module->getI32(std::stoi(name));
    } catch(const std::invalid_argument& e) {
        if (vars.count(name) == 0) return module->getVar(name);
        return vars[name];
    }
    
}
void FunctionWrapper::addBlock(const std::string& name) {
    blocks.emplace(name, llvm::BasicBlock::Create(module->Context, name, function));
}
llvm::BasicBlock* FunctionWrapper::getBlock(const std::string& name) {
    if (blocks.count(name) == 0) blocks.emplace(name, llvm::BasicBlock::Create(module->Context, name, function));
    return blocks[name];
}
void FunctionWrapper::setActiveBlock(const std::string& name) {
    if (blocks.count(name) == 0) blocks.emplace(name, llvm::BasicBlock::Create(module->Context, name, function));
    activeBlock = name;
}


llvm::Instruction* FunctionWrapper::insert(llvm::Instruction* instr) {
    if (activeBlock == "_____") setActiveBlock("");
    llvm::BasicBlock*& block = blocks[activeBlock];
    instr->insertInto(block, block->end());
    return instr;
}
llvm::Value* FunctionWrapper::insert(const std::string& varName, llvm::Instruction* instr) {
    return vars[varName] = insert(instr);
}
llvm::Value* FunctionWrapper::insertUnaryOperator(const std::string& varName, const llvm::Instruction::UnaryOps& op, const std::string& A) {
    return insert(varName, llvm::UnaryOperator::Create(op, getVarOrNumber(A), varName));
}
llvm::Value* FunctionWrapper::insertBinaryOperator(const std::string& varName, const llvm::Instruction::BinaryOps& op, const std::string& A, const std::string& B) {
    return insert(varName, llvm::BinaryOperator::Create(op, getVarOrNumber(A), getVarOrNumber(B), varName));
}
llvm::Instruction* FunctionWrapper::call(const std::string& varName, llvm::ArrayRef<llvm::Value*> arguments) {
    llvm::CallInst* instr = llvm::CallInst::Create(function, arguments, varName);
    instr->setTailCall();
    return instr;
}
llvm::Instruction* FunctionWrapper::call(llvm::ArrayRef<llvm::Value*> arguments) {
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
    
    Module = new llvm::Module(name, Context);
    Module->setTargetTriple(llvm::Triple(llvm::sys::getDefaultTargetTriple()));
}
ModuleWrapper::~ModuleWrapper() {
    std::error_code EC;
    llvm::raw_fd_ostream outFile("./out/" + name + ".ll", EC);
    Module->print(outFile, nullptr);
    delete Module;
}

FunctionWrapper& ModuleWrapper::createFunction(llvm::Type* returnType, const std::string& name, const std::initializer_list<std::pair<std::string, llvm::Type*>>& arguments) {
    functions[name] = new FunctionWrapper(this, returnType, name, arguments);
    return getFunction(name);
}
FunctionWrapper& ModuleWrapper::getFunction(const std::string& name) {
    if (functions.count(name) == 0) throw std::runtime_error("function name not found.");
    return *functions[name];
}
void ModuleWrapper::createGlobalStr(const std::string& varName, const std::string& value) {
    llvm::GlobalVariable* str = new llvm::GlobalVariable(*Module, llvm::ArrayType::get(char_t, static_cast<unsigned int>(value.size())+1u), true, llvm::GlobalValue::PrivateLinkage, 0, varName);
    str->setInitializer(llvm::ConstantDataArray::getString(Context, value, true));
    globals[varName] = str;
}
llvm::Value* ModuleWrapper::getVar(const std::string& name) {
    if (globals.count(name) == 0) throw std::runtime_error("variable name not found.");
    return globals[name];
}
llvm::Value* ModuleWrapper::getI32(const int& i) {
    if (i32constants.count(i) == 0) i32constants[i] = llvm::ConstantInt::get(i32_t, i);
    return i32constants[i];
}
llvm::Value* ModuleWrapper::getI64(const long long int& i) {
    if (i64constants.count(i) == 0) i64constants[i] = llvm::ConstantInt::get(i64_t, i);
    return i64constants[i];
}
llvm::Value* ModuleWrapper::getUI32(const unsigned int& i) {
    if (i32constants.count(i) == 0) i32constants[i] = llvm::ConstantInt::get(i32_t, i);
    return i32constants[i];
}
llvm::Value* ModuleWrapper::getUI64(const unsigned long long int& i) {
    if (i64constants.count(i) == 0) i64constants[i] = llvm::ConstantInt::get(i64_t, i);
    return i64constants[i];
}


void addStdLib(ModuleWrapper& module) {
    // print functions
    module.createFunction(module.void_t, "printChar", {{"1", module.char_t}});
    module.createFunction(module.void_t, "printStr", {{"1", module.ptr_t}});
    module.createFunction(module.void_t, "printUInt", {{"1", module.i32_t}});
    module.createFunction(module.void_t, "printUInt64", {{"1", module.i64_t}});
    module.createFunction(module.void_t, "printInt", {{"1", module.i32_t}});
    module.createFunction(module.void_t, "printInt64", {{"1", module.i64_t}});
    module.createFunction(module.void_t, "printFloat", {{"1", module.fp32_t}});
    module.createFunction(module.void_t, "printDouble", {{"1", module.fp64_t}});
    // module.createFunction(module.void_t, "printFP128", {{"1", module.fp128_t}});
    module.createFunction(module.void_t, "printUInt128", {{"1", module.i128_t}});
    module.createFunction(module.void_t, "printUInt256", {{"1", module.i256_t}});
    module.createFunction(module.void_t, "printUInt512", {{"1", module.i512_t}});
    // println functions
    module.createFunction(module.void_t, "printlnChar", {{"1", module.char_t}});
    module.createFunction(module.void_t, "printlnStr", {{"1", module.ptr_t}});
    module.createFunction(module.void_t, "printlnUInt", {{"1", module.i32_t}});
    module.createFunction(module.void_t, "printlnUInt64", {{"1", module.i64_t}});
    module.createFunction(module.void_t, "printlnInt", {{"1", module.i32_t}});
    module.createFunction(module.void_t, "printlnInt64", {{"1", module.i64_t}});
    module.createFunction(module.void_t, "printlnFloat", {{"1", module.fp32_t}});
    module.createFunction(module.void_t, "printlnDouble", {{"1", module.fp64_t}});
    module.createFunction(module.void_t, "println", {});
    // module.createFunction(module.void_t, "printlnFP128", {{"1", module.fp128_t}});
    module.createFunction(module.void_t, "printlnUInt128", {{"1", module.i128_t}});
    module.createFunction(module.void_t, "printlnUInt256", {{"1", module.i256_t}});
    module.createFunction(module.void_t, "printlnUInt512", {{"1", module.i512_t}});
    // string parsing functions
    module.createFunction(module.i32_t, "strToInt", {{"1", module.ptr_t}});
    module.createFunction(module.i32_t, "strToUInt", {{"1", module.ptr_t}});
}
int main(int argc, char** argv) {
    ModuleWrapper module(argv[1]);
    addStdLib(module);
    module.createGlobalStr("str0", "program requires at least 1 argument.");
    FunctionWrapper& mainFunc = module.createFunction(module.i32_t, "main", {{"argc", module.i32_t},{"argv", module.ptr_t}});
    mainFunc.insertICmp("cond", "argc", llvm::ICmpInst::Predicate::FCMP_ULT, "2");
    mainFunc.insertBr("cond", "err.0", "if.0");

    mainFunc.setActiveBlock("if.0");
    mainFunc.insertGetElementPtr("numStrP", llvm::ArrayType::get(module.ptr_t, 0), "argv", { "1" });
    mainFunc.insertLoad("numStr", module.ptr_t, "numStrP");
    mainFunc.insertCall("num", "strToUInt", { "numStr" });
    mainFunc.insertAdd("output", "num", "5");
    mainFunc.insertCall("printlnUInt", { "output" });
    mainFunc.insertBr("ret");

    mainFunc.setActiveBlock("err.0");
    mainFunc.insertCall("printlnStr",{ "str0" });
    mainFunc.insertBr("ret");

    mainFunc.setActiveBlock("ret");
    mainFunc.insertReturn("0");
    return 0;
}
