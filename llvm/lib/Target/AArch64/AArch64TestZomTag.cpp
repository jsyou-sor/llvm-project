#include <iostream>

#include "AArch64.h"
#include "AArch64Subtarget.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "aarch64-zomtag"

using namespace llvm;

namespace
{
  class TestZomTag : public MachineFunctionPass 
  {
    public:
      static char ID;
      TestZomTag() : MachineFunctionPass(ID) {
        //initializeTestZomTagPass(*PassRegistry::getPassRegistry());    
      }
      StringRef getPassName() const override { return "skeleton AArch64 zomtag pass"; }

      bool doInitialization(Module &M) override;
      bool runOnMachineFunction(MachineFunction &F) override;

    private:
      bool m_init_done;
  };
  //char TestZomTag::ID = 0;
} // end anonymous namespace

//INITIALIZE_PASS(TestZomTag, "aarch64-zomtag-pass", "AArch64 test zomtag pass", false, false)

FunctionPass *llvm::createAArch64TestZomTagPass()
{
  return new TestZomTag();
}

char TestZomTag::ID = 0;

bool TestZomTag::doInitialization(Module &M)
{
  m_init_done = false;
  return false;
}

bool TestZomTag::runOnMachineFunction(MachineFunction &MF)
{
  DEBUG(dbgs() << getPassName() << '\n');
  std::cerr << "Hello World, compiling a function\n";
  return true;
}
