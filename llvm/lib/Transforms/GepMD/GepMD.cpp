#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "../../Target/AArch64/AArch64Zt.h"

using namespace llvm;

#define DEBUG_TYPE "GepMDPass"

namespace
{
  struct GepMDPass : public FunctionPass
  {
    static char ID;
    GepMDPass() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override
    {
      for (auto &BB : F)
      {
        for (auto &I : BB)
        {
/*
          if (I.getOpcode() == Instruction::Load)
          {
            errs() << "\n";
            I.dump();
            errs().write_escaped(I.getOpcodeName(I.getOpcode()));
            errs() << "\n";
            Type *Ty = I.getType();
            if (Ty->isPointerTy())
            {
              if (PointerType *PT = dyn_cast<PointerType>(Ty))
              {
                Type *ty = PT->getElementType();
                errs() << PT->getElementType()->isFunctionTy() << "\n";
                ty->dump();
              }
            }
            auto &C = F.getContext();
            MDNode *N = MDNode::get(C, MDString::get(C, "Metadata"));
            I.setMetadata("a", N);
          }
          else if (I.getOpcode() == Instruction::Store)
          {
            errs() << "\n";
            I.dump();
            errs().write_escaped(I.getOpcodeName(I.getOpcode()));
            errs() << "\n";
            Type *Ty = I.getOperand(0)->getType();
            if (Ty->isPointerTy())
            {
              if (PointerType *PT = dyn_cast<PointerType>(Ty))
              {
                Type *ty = PT->getElementType();
                errs() << PT->getElementType()->isFunctionTy() << "\n";
                ty->dump();
              }
            }
            auto &C = F.getContext();
            MDNode *N = MDNode::get(C, MDString::get(C, "Metadata"));
            I.setMetadata("a", N);
          }
*/
          //else if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&I))
          if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&I))
          {
            errs() << "\nGEP\t";
            I.dump();

            auto &C = F.getContext();
            MDNode *N = MDNode::get(C, MDString::get(C, "Metadata"));
            //MDNode *N = MDNode::get(C, MDString::get(C, "howdy"));
            I.setMetadata(ZTMetaDataKind, N);
            //errs().write_escaped(I.getOpcodeName(I.getOpcode()));
            errs() << "\n";
          }
        }
      }
      return false;
    }
  };
}

char GepMDPass::ID = 0;
static RegisterPass<GepMDPass> X("gep-md-pass", "GetElementPtr Metadata Pass");
