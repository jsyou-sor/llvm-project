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

#include "AArch64ZomTagUtils.h"

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
      StringRef getPassName() const override { return "zomtag-ptr"; }

      bool doInitialization(Module &M) override;
      bool runOnMachineFunction(MachineFunction &F) override;

    private:
      const TargetMachine *TM = nullptr;
      const AArch64Subtarget *STI = nullptr;
      const AArch64InstrInfo *TII = nullptr;
      const AArch64RegisterInfo *TRI = nullptr;

      ZomTagUtils_ptr zomtagUtils = nullptr;

      //bool isLoad(MachineInstr &MI);
      bool isStore(MachineInstr &MI);
      bool isAddSub(MachineInstr &MI);
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
  return false;
}

bool TestZomTag::runOnMachineFunction(MachineFunction &MF)
{
  //DEBUG(dbgs() << getPassName() << '\n');
  errs() << "function " << MF.getName() << '\n';

  TM = &MF.getTarget();
  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  TRI = STI->getRegisterInfo();

  zomtagUtils = ZomTagUtils::get(TRI, TII);

  for (auto &MBB : MF)
  {
    for (auto &MI : MBB)
    {
      //MI.dump();

      if (isAddSub(MI))
      {
        //errs() << "found " << TII->getName(MI.getOpcode()) << "\n";
        auto op_src = MI.getOperand(MI.getNumOperands() - 1);
        //errs() << "\t\top_src is ";
        //errs() << (op_src.isMetadata() ? "(metadata)" : "(non-metadata) ");
        //if (op_src.isMetadata() &&
        if ( 
            ((MI.getOpcode() == AArch64::ADDXrr) ||
            (MI.getOpcode() == AArch64::ADDWrr) ||
            (MI.getOpcode() == AArch64::ADDSXrr) ||
            (MI.getOpcode() == AArch64::ADDSWrr) ||
            (MI.getOpcode() == AArch64::SUBXrr) ||
            (MI.getOpcode() == AArch64::SUBWrr) ||
            (MI.getOpcode() == AArch64::SUBSXrr) ||
            (MI.getOpcode() == AArch64::SUBSWrr)) 
        )
          MI.dump();
        if (MI.getOperand(MI.getNumOperands() - 1).isMetadata())
          MI.dump();
      }
      if (zomtagUtils->isLoad(MI))
      {
        if (MI.getOperand(0).isReg() &&
            MI.getOperand(1).isReg() &&
            MI.getOperand(2).isReg())
        {
          // TODO:
          // Add condition to filter LDP (Load Pair) instructions
          if (MI.getOpcode() != AArch64::LDPXi && MI.getOpcode() != AArch64::LDPWi)
          {
            const auto offset_reg = MI.getOperand(2).getReg();
            if (zomtagUtils->isXReg(offset_reg))
              MI.dump();
          }
        }
      }
    }
  }
  return true;
}

bool TestZomTag::isStore(MachineInstr &MI)
{
  switch(MI.getOpcode())
  {
    default:
      return false;
    case AArch64::STRWpost:
    case AArch64::STURQi:
    case AArch64::STURXi:
    case AArch64::STURDi:
    case AArch64::STURWi:
    case AArch64::STURSi:
    case AArch64::STURHi:
    case AArch64::STURHHi:
    case AArch64::STURBi:
    case AArch64::STURBBi:
    case AArch64::STPQi:
    case AArch64::STNPQi:
    case AArch64::STRQui:
    case AArch64::STPXi:
    case AArch64::STPDi:
    case AArch64::STNPXi:
    case AArch64::STNPDi:
    case AArch64::STRXui:
    case AArch64::STRDui:
    case AArch64::STPWi:
    case AArch64::STPSi:
    case AArch64::STNPWi:
    case AArch64::STNPSi:
    case AArch64::STRWui:
    case AArch64::STRSui:
    case AArch64::STRHui:
    case AArch64::STRHHui:
    case AArch64::STRBui:
    case AArch64::STRBBui:
      return true;
  }
}

/*
bool TestZomTag::isLoad(MachineInstr &MI)
{
  switch(MI.getOpcode())
  {
    default:
      return false;
    case AArch64::LDPXi:
    case AArch64::LDPDi:
    case AArch64::LDRWpost:
    case AArch64::LDURQi:
    case AArch64::LDURXi:
    case AArch64::LDURDi:
    case AArch64::LDURWi:
    case AArch64::LDURSi:
    case AArch64::LDURSWi:
    case AArch64::LDURHi:
    case AArch64::LDURHHi:
    case AArch64::LDURSHXi:
    case AArch64::LDURSHWi:
    case AArch64::LDURBi:
    case AArch64::LDURBBi:
    case AArch64::LDURSBXi:
    case AArch64::LDURSBWi:
    case AArch64::LDPQi:
    case AArch64::LDNPQi:
    case AArch64::LDRQui:
    case AArch64::LDNPXi:
    case AArch64::LDNPDi:
    case AArch64::LDRXui:
    case AArch64::LDRDui:
    case AArch64::LDPWi:
    case AArch64::LDPSi:
    case AArch64::LDNPWi:
    case AArch64::LDNPSi:
    case AArch64::LDRWui:
    case AArch64::LDRSui:
    case AArch64::LDRSWui:
    case AArch64::LDRHui:
    case AArch64::LDRHHui:
    case AArch64::LDRBui:
    case AArch64::LDRBBui:
      return true;
  }
}
*/

bool TestZomTag::isAddSub(MachineInstr &MI)
{
  switch(MI.getOpcode())
  {
    default:
      return false;
    case AArch64::SUBWri:
    case AArch64::SUBXri:
    case AArch64::ADDWri:
    case AArch64::ADDXri:
    case AArch64::SUBSWri:
    case AArch64::SUBSXri:
    case AArch64::ADDSWri:
    case AArch64::ADDSXri:
    case AArch64::SUBWrr:
    case AArch64::SUBXrr:
    case AArch64::ADDWrr:
    case AArch64::ADDXrr:
    case AArch64::SUBSWrr:
    case AArch64::SUBSXrr:
    case AArch64::ADDSWrr:
    case AArch64::ADDSXrr:
      return true;
  }
}
