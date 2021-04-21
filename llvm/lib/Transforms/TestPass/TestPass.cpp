//#include "llvm/Transforms/Instrumentation/TestSanitizer.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/IR/PassManager.h"

#include <vector>
#include <string>

using namespace llvm;

#define DEBUG_TYPE "testsan"

namespace 
{
  class TestSanitizerModule : public ModulePass
  {
    public:
      static char ID;
      TestSanitizerModule() : ModulePass(ID) {}
      bool runOnModule(Module &M) override;
  };
  class TestSanitizerFunction : public FunctionPass
  {
    public:
      static char ID;
      TestSanitizerFunction() : FunctionPass(ID) {}
      bool runOnFunction(Function &F) override;
  };
  class TestSanitizerBlock : public BasicBlockPass
  {
    public:
      static char ID;
      TestSanitizerBlock() : BasicBlockPass(ID) {}
      bool runOnBasicBlock(BasicBlock &B) override;
  };
} // end anon namesapce

bool TestSanitizerModule::runOnModule(Module &module)
{
  errs() << "This is the LLVM Module for source file: " << module.getSourceFileName() << "\n";
  for (auto &func : module)
  {
    errs() << "Hello function: " << func.getName() << "!\n";
  }
  return false;
}

bool TestSanitizerFunction::runOnFunction(Function &func)
{
  errs() << "Instrumenting function entry!\n";

  LLVMContext &context = func.getContext();
  IRBuilder<> builder(context);

  builder.SetInsertPoint(&(func.getEntryBlock().front()));

  std::vector<llvm::Type *> name_set_func_args;
  name_set_func_args.push_back(llvm::Type::getInt8Ty(context)->getPointerTo());

  ArrayRef<Type*> name_set_args_ref(name_set_func_args);

  FunctionType *name_set_func_type =
    FunctionType::get(llvm::Type::getVoidTy(context), name_set_args_ref, false);

  Constant *name_set_func = func.getParent()->getOrInsertFunction("testsan_HelloFunction", name_set_func_type);
  Value *name_val = builder.CreateGlobalStringPtr(func.getName());
  Value *args[] = {name_val};
  builder.CreateCall(name_set_func, args);

  if (func.getName().compare("main") == 0)
  {
    for (auto &block : func)
    {
      for (auto &inst : block)
      {
        if (auto *potential_ret = dyn_cast<ReturnInst>(&inst))
        {
          builder.SetInsertPoint(potential_ret);
          std::vector<llvm::Type *>summary_func_args;
          ArrayRef<Type *>summary_args_ref(summary_func_args);

          FunctionType *summary_func_type =
            FunctionType::get(llvm::Type::getVoidTy(context), summary_args_ref, false);

          Constant *summary_func = func.getParent()->getOrInsertFunction("testsan_EndOfMain", summary_func_type);
          builder.CreateCall(summary_func, {});
        }
      }
    }
  }
  return true;
}

bool TestSanitizerBlock::runOnBasicBlock(BasicBlock &BB)
{
  LLVMContext &context = BB.getContext();
  Module *current_module = BB.getModule();

  PointerType *byte_ptr_type = llvm::Type::getInt8Ty(context)->getPointerTo();
  std::vector<llvm::Type *>log_allocation_args;
  log_allocation_args.push_back(byte_ptr_type);
  ArrayRef<Type*>log_allocation_ref(log_allocation_args);

  FunctionType *log_allocation_type = FunctionType::get(byte_ptr_type, log_allocation_ref, false);

  Constant *log_allocation_func = current_module->getOrInsertFunction("testsan_AfterMalloc", log_allocation_type);

  errs() << "Checking instructions\n";
  for (auto &Inst : BB)
  {
    if (auto *potential_malloc = dyn_cast<CallInst>(&Inst))
    {
      errs() << "Call instruction\n";
      Function *called_func = potential_malloc->getCalledFunction();
      std::string func_name = called_func->getName();
      if (func_name.compare("malloc") == 0)
      {
        errs() << "Malloc found!\n";
        IRBuilder<> builder(potential_malloc);
        builder.SetInsertPoint(potential_malloc->getNextNode());
        builder.CreateCall(log_allocation_func, potential_malloc);
      }
    }
  }
  return true;
}

char TestSanitizerModule::ID = 0;
char TestSanitizerFunction::ID = 0;
char TestSanitizerBlock::ID = 0;

static RegisterPass<TestSanitizerModule> TestMod("testmod", "testing module pass");
static RegisterPass<TestSanitizerFunction> TestFunc("testfunc", "testing function pass");
static RegisterPass<TestSanitizerBlock> TestBlock("testblock", "testing block pass");
