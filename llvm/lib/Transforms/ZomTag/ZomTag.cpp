#include <assert.h>
#include <stdio.h>

#include <iostream>
#include <map>
#include <vector>
#include <set>

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/SpecialCaseList.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/TargetLibraryInfo.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;
using namespace std;

static void getInterestingInsts(Instruction *I)
{
  if (StoreInst *Store = dyn_cast<StoreInst>(I))
  {
    Value *Val = Store->getValueOperand();
    if (Val->getType()->isPointerTy())
    {
      I->print(errs());
      errs() << "\n";
    }
  }
}

/* ZomTag LLVM Pass */
namespace
{

struct ZomTag : public FunctionPass
{
  static char ID;
  ZomTag() : FunctionPass(ID) { }

  virtual bool runOnFunction(Function &F)
  {
    dbgs() << "ZomTag Pass\n";
    
    //const DataLayout *DL = &M.getDataLayout();
    for (auto &BB : F)
    {
      for (auto &I : BB)
      {
        getInterestingInsts(&I);
      }
    }
    return false;
  }
};

}

char ZomTag::ID = 0;
static void registerZomTagPass(const PassManagerBuilder &,
                               legacy::PassManagerBase &PM)
{
  PM.add(new ZomTag());
}

static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible, registerZomTagPass);
