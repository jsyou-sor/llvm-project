#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/InitializePasses.h"
#include "../../Target/AArch64/AArch64Zt.h"

using namespace llvm;
#define DEBUG_TYPE "ZomtagMetaDataPass"

namespace
{
  struct ZomtagMetaData : public FunctionPass
  {
    static char ID;
    ZomtagMetaData() : FunctionPass(ID)
    {
      initializeZomtagMetaDataPass(*PassRegistry::getPassRegistry());
    }

    bool runOnFunction(Function &F) override
    {
      for (auto &BB : F)
      {
        for (auto &I : BB)
        {
          if(GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&I))
          {
            errs() << "\t[GEP]\t";
            I.dump();
            auto &C = F.getContext();
            MDNode *N = MDNode::get(C, MDString::get(C, "Metadata"));
            I.setMetadata(ZTMetaDataKind, N);
            errs() << "\n";
          }
        }
      }
      return false;
    }
  };
}

char ZomtagMetaData::ID = 0;
INITIALIZE_PASS(ZomtagMetaData, "zomtag-md-pass", "zomtag md pass", false, false)

FunctionPass *llvm::createZomtagMetaDataPass()
{
  return new ZomtagMetaData();
}
