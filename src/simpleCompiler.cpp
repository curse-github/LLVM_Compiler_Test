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

int main(int argc, char** argv) {
    llvm::LLVMContext Context;
    llvm::Type* void_t = llvm::Type::getVoidTy(Context);
    llvm::Type* ptr_t = llvm::PointerType::get(Context, 0);
    llvm::Type* char_t = llvm::Type::getInt8Ty(Context);
    llvm::Type* i32_t = llvm::Type::getInt32Ty(Context);
    llvm::Type* i64_t = llvm::Type::getInt64Ty(Context);
    llvm::Module *Module = new llvm::Module(argv[1], Context);
    Module->setTargetTriple(llvm::Triple(llvm::sys::getDefaultTargetTriple()));

    llvm::FunctionType* printlnUInt_t = llvm::FunctionType::get(void_t, { i32_t }, false);
    llvm::Function* printlnUInt = llvm::Function::Create(printlnUInt_t, llvm::Function::ExternalLinkage, "printlnUInt", Module);
    llvm::FunctionType* printlnStr_t = llvm::FunctionType::get(void_t, { ptr_t }, false);
    llvm::Function* printlnStr = llvm::Function::Create(printlnStr_t, llvm::Function::ExternalLinkage, "printlnStr", Module);
    llvm::FunctionType* strToUInt_t = llvm::FunctionType::get(i32_t, { ptr_t }, false);
    llvm::Function* strToUInt = llvm::Function::Create(strToUInt_t, llvm::Function::ExternalLinkage, "strToUInt", Module);

    std::string str0_val = "program requires at least 1 argument.";
    llvm::GlobalVariable* str0 = new llvm::GlobalVariable(*Module, llvm::ArrayType::get(char_t, static_cast<unsigned int>(str0_val.size())+1u), true, llvm::GlobalValue::PrivateLinkage, 0, "str0");
    str0->setInitializer(llvm::ConstantDataArray::getString(Context, str0_val, true));

    llvm::FunctionType* main_t = llvm::FunctionType::get(i32_t, { i32_t, ptr_t }, false);
    llvm::Function* main = llvm::Function::Create(main_t, llvm::Function::ExternalLinkage, "main", Module);
    llvm::Argument* main_argc = main->getArg(0);
    main_argc->setName("argc");
    llvm::Argument* main_argv = main->getArg(1);
    main_argv->setName("argv");

    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(Context, "", main);
    llvm::BasicBlock* ifBlock = llvm::BasicBlock::Create(Context, "if.0", main);
    llvm::BasicBlock* errBlock = llvm::BasicBlock::Create(Context, "err.0", main);
    llvm::BasicBlock* retBlock = llvm::BasicBlock::Create(Context, "ret", main);

    llvm::Instruction* cond = llvm::ICmpInst::Create(llvm::Instruction::OtherOps::ICmp, llvm::ICmpInst::Predicate::FCMP_ULT, main_argc, llvm::ConstantInt::get(i32_t, 2), "cond");
    cond->insertInto(entryBlock, entryBlock->end());
    llvm::BranchInst::Create(errBlock, ifBlock, cond)->insertInto(entryBlock, entryBlock->end());

    llvm::Instruction* numStrP = llvm::GetElementPtrInst::Create(llvm::ArrayType::get(ptr_t, 0), main_argv, { llvm::ConstantInt::get(i64_t, 0), llvm::ConstantInt::get(i64_t, 1) }, llvm::GEPNoWrapFlags::none(), "numStrP", nullptr);
    numStrP->insertInto(ifBlock, ifBlock->end());
    llvm::Instruction* numStr = (new llvm::LoadInst(ptr_t, numStrP, "numStr", ifBlock));
    llvm::CallInst* num = llvm::CallInst::Create(strToUInt, { numStr }, "num");
    num->setTailCall();
    num->insertInto(ifBlock, ifBlock->end());
    llvm::Instruction* output = llvm::BinaryOperator::Create(llvm::Instruction::Add, num, llvm::ConstantInt::get(i32_t, 5), "output");
    output->insertInto(ifBlock, ifBlock->end());
    llvm::CallInst* call = llvm::CallInst::Create(printlnUInt, { output }, "");
    call->setTailCall();
    call->insertInto(ifBlock, ifBlock->end());
    llvm::BranchInst::Create(retBlock)->insertInto(ifBlock, ifBlock->end());

    llvm::CallInst* errorPrint = llvm::CallInst::Create(printlnStr, { str0 }, "");
    errorPrint->setTailCall();
    errorPrint->insertInto(errBlock, errBlock->end());
    llvm::ReturnInst::Create(Context, llvm::ConstantInt::get(i32_t, 1))->insertInto(errBlock, errBlock->end());

    llvm::ReturnInst::Create(Context, llvm::ConstantInt::get(i32_t, 0))->insertInto(retBlock, retBlock->end());

    std::error_code EC;
    llvm::raw_fd_ostream outFile(std::string("./out/") + argv[1] + ".ll", EC);
    Module->print(outFile, nullptr);
    delete Module;
    return 0;
}
