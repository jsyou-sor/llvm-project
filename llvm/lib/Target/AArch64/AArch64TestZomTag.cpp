#include <iostream>

#include "AArch64.h"
#include "AArch64Subtarget.h"
#include "AArch64TargetMachine.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/FastISel.h"
#include "llvm/CodeGen/FunctionLoweringInfo.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/MC/MCSymbol.h"

#include "MCTargetDesc/AArch64AddressingModes.h"

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

      //bool isAddSub(MachineInstr &MI);
  };
} // end anonymous namespace

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
  //errs() << "function " << MF.getName() << '\n';

  TM = &MF.getTarget();
  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  TRI = STI->getRegisterInfo();

  zomtagUtils = ZomTagUtils::get(TRI, TII);

  for (auto &MBB : MF)
  {
    for (auto MIi = MBB.instr_begin(); MIi != MBB.instr_end(); MIi++)
    {
      if (zomtagUtils->isInterestingLoad(*MIi))
      {
        //MIi->print(errs());
        const auto &DL = MIi->getDebugLoc();
        const auto op = zomtagUtils->getCorrespondingLoad(MIi->getOpcode());
        const unsigned dst = MIi->getOperand(0).getReg();
        const unsigned src = MIi->getOperand(1).getReg();
        const unsigned off_x = MIi->getOperand(2).getReg();
        const unsigned off_w = zomtagUtils->getCorrespondingReg(off_x);
        //const int64_t ext = MIi->getOperand(3).getImm();
        const int64_t ext = AArch64_AM::SXTW;
        const int64_t amount = MIi->getOperand(4).getImm();

        BuildMI(MBB, MIi, DL, TII->get(op),dst).addReg(src).addReg(off_w).addImm(ext).addImm(amount);
        
        auto tmp = MIi;
        MIi--;
        tmp->removeFromParent();
      }

      if (zomtagUtils->isInterestingStore(*MIi))
      {
        //MIi->print(errs());
        const auto &DL = MIi->getDebugLoc();
        const auto op = zomtagUtils->getCorrespondingStore(MIi->getOpcode());
        const unsigned dst = MIi->getOperand(0).getReg();
        const unsigned src = MIi->getOperand(1).getReg();
        const unsigned off_x = MIi->getOperand(2).getReg();
        const unsigned off_w = zomtagUtils->getCorrespondingReg(off_x);
        //const int64_t ext = MIi->getOperand(3).getImm();
        const int64_t ext = AArch64_AM::SXTW;
				const int64_t amount = MIi->getOperand(4).getImm();

        //BuildMI(MBB, MIi, DL, TII->get(op), dst).addReg(src).addReg(off_w).addImm(ext).addImm(amount);
      
        //auto tmp = MIi;
        //MIi--;
        //tmp->removeFromParent();
      }

			if (zomtagUtils->isAddSub(*MIi))
			{
				auto op_src = MIi->getOperand(MIi->getNumOperands() - 1);
				if (op_src.isMetadata())
				{
					const unsigned ptr_reg = MIi->getOperand(1).getReg();
					const MCInstrDesc &II = TII->get(AArch64::ANDSXri);							// ANDSXri
					const auto &DL = MIi->getDebugLoc();
					int64_t imm = 0x100000000;
				
					//auto tmp = MIi + 1;	
	
					//MIi->print(errs());
					//MachineInstrBuilder MIB =
					//BuildMI(MBB, MIi, DL, II, AArch64::XZR)
						//.addReg(ptr_reg)
						//.addImm(AArch64_AM::encodeLogicalImmediate(imm, 64));
					//errs() << "********** TST ";
					//MIB->print(errs());

				}
			}
    }
  }
  return true;
}

