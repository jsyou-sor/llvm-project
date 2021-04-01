#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace 
{

struct Test : public ModulePass 
{    
  static char ID;
  Test() : ModulePass(ID) { }
  
  virtual bool runOnModule(Module &M) 
  {  
    errs() << "Test Pass\n";
    return false;
  }
};

}

char Test::ID = 0;
//static RegisterPass<Test> X("test","test pass");
static void registerTestPass(const PassManagerBuilder &,
                             legacy::PassManagerBase &PM)
{
  PM.add(new Test());
}

static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible, registerTestPass);
