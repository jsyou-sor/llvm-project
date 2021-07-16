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
#define AARCH64_ZOMTAG_PASS_NAME "AArch64 Zomtag Pass"

using namespace llvm;

/* Options */
static cl::opt<bool> option_imprecise1("zometag-imprecise1",
		cl::desc("Instrument imprecise tag loading 1"));
static cl::opt<bool> option_imprecise2("zometag-imprecise2",
		cl::desc("Instrument imprecise tag loading 2"));
static cl::opt<bool> option_nop("zometag-nop",
		cl::desc("Instrument nop tag loading"));
static cl::opt<bool> option_precise("zometag-precise",
		cl::desc("Instrument precise tag loading"));

namespace llvm
{
	void initializeTestZomTagPass(PassRegistry &);
}

namespace
{
  class TestZomTag : public MachineFunctionPass 
  {
    public:
      static char ID;
      TestZomTag() : MachineFunctionPass(ID) {
        initializeTestZomTagPass(*PassRegistry::getPassRegistry());    
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

INITIALIZE_PASS(TestZomTag, "AArch64 Zomtag Pass", AARCH64_ZOMTAG_PASS_NAME, false, false)

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
  TM = &MF.getTarget();
  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  TRI = STI->getRegisterInfo();

  zomtagUtils = ZomTagUtils::get(TRI, TII);

  for (auto &MBB : MF)
  {
    for (auto MIi = MBB.instr_begin(); MIi != MBB.instr_end(); MIi++)
    {

			if (zomtagUtils->isLoad(*MIi))
			{
				unsigned x;
				x = MIi->getOperand(1).getReg();
				if (zomtagUtils->isLoadPair(*MIi))
					x = MIi->getOperand(2).getReg();
				if (x != AArch64::SP && x != AArch64::FP)
				{
					if (MIi->getOperand(1).isReg() && !(zomtagUtils->isLDRD(*MIi)))
					{
						const auto &DL = MIi->getDebugLoc();
						unsigned src = MIi->getOperand(1).getReg();
						unsigned target = MIi->getOperand(0).getReg();
					
						if (zomtagUtils->isLoadPair(*MIi))
							src = MIi->getOperand(2).getReg();
					
						const unsigned DstReg = AArch64::X15;
						const unsigned Imm = 0x7fbe;
						
						if (option_imprecise1)
						{
							BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVZXi), AArch64::X15)
											.addImm(Imm)
											.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSL, 32));

							if (!(zomtagUtils->isPrePostIndexed(*MIi)))
							{
								BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), AArch64::X15)
												.addReg(AArch64::X15)
												.addReg(src)
												.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 5));

								BuildMI(MBB, MIi, DL, TII->get(AArch64::LDRBBui), AArch64::XZR).addReg(AArch64::X15).addImm(0);
							}
						}

						if (option_imprecise2)
						{
							BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVZXi), AArch64::X15)
											.addImm(Imm)
											.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSL, 32));

							BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), AArch64::X15)
											.addReg(AArch64::X15)
											.addReg(AArch64::XZR)
											.addImm(0);

							BuildMI(MBB, MIi, DL, TII->get(AArch64::LDRBBui), AArch64::XZR).addReg(AArch64::X15).addImm(0);
						}

						if (option_nop)
						{
							BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVZXi), AArch64::X15)
											.addImm(Imm)
											.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSL, 32));
							BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), AArch64::X15)
											.addReg(AArch64::X15)
											.addReg(src)
											.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 5));
							BuildMI(MBB, MIi, DL, TII->get(AArch64::HINT)).addImm(0);
						}

						if (option_precise)
						{
							BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVZXi), AArch64::X15)
											.addImm(Imm)
											.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSL, 32));

							// Non-Pre/PostIndexed Load
							if (!(zomtagUtils->isPrePostIndexed(*MIi)) &&
									!(zomtagUtils->isQReg(MIi->getOperand(0).getReg())))
							{
								// ADD instrumentation
								// Check LDP
								if (!(zomtagUtils->isLoadPair(*MIi)))
									BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), AArch64::X15)
													.addReg(AArch64::X15)
													.addReg(MIi->getOperand(1).getReg())
													.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 5));
								else
									BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), AArch64::X15)
													.addReg(AArch64::X15)
													.addReg(MIi->getOperand(2).getReg())
													.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 5));
								
								// LDAR instrumentation
								if (!(zomtagUtils->isLoadPair(*MIi)))
								{
									if (MIi->getOperand(0).getReg() == MIi->getOperand(1).getReg() ||
											zomtagUtils->getCorrespondingReg(MIi->getOperand(1).getReg()) == MIi->getOperand(0).getReg())
										BuildMI(MBB, MIi, DL, TII->get(AArch64::LDARB), AArch64::XZR).addReg(AArch64::X15).addImm(0);
										//int a = 0;
									else
									{
										if (MIi->getNumOperands() <= 4)
											BuildMI(MBB, MIi, DL, TII->get(AArch64::LDARB), MIi->getOperand(0).getReg()).addReg(AArch64::X15).addImm(0);
									}
								}
								else // LDP
								{
									if (MIi->getOperand(0).getReg() == MIi->getOperand(2).getReg() ||
											zomtagUtils->getCorrespondingReg(MIi->getOperand(2).getReg()) == MIi->getOperand(0).getReg() ||
											MIi->getOperand(1).getReg() == MIi->getOperand(2).getReg() ||
											zomtagUtils->getCorrespondingReg(MIi->getOperand(2).getReg()) == MIi->getOperand(1).getReg())
										BuildMI(MBB, MIi, DL, TII->get(AArch64::LDARB), AArch64::XZR).addReg(AArch64::X15).addImm(0);
									else
										BuildMI(MBB, MIi, DL, TII->get(AArch64::LDARB), MIi->getOperand(0).getReg()).addReg(AArch64::X15).addImm(0);
								}
							}

							if (!(zomtagUtils->isPrePostIndexed(*MIi)) &&
									(zomtagUtils->isQReg(MIi->getOperand(0).getReg())))
							{
								if (!(zomtagUtils->isLoadPair(*MIi)))
								{
									BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), AArch64::X15)
													.addReg(AArch64::X15)
													.addReg(MIi->getOperand(1).getReg())
													.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 5));
									BuildMI(MBB, MIi, DL, TII->get(AArch64::LDRQui), MIi->getOperand(0).getReg()).addReg(AArch64::X15).addImm(0);
								}
							}

/*
							if ((zomtagUtils->isPrePostIndexed(*MIi)) &&
									!(zomtagUtils->isQReg(MIi->getOperand(0).getReg())))
							{
									MIi->print(errs());
									errs() << "\n";
									MIi->getOperand(0).print(errs());
									errs() << "\n";
									MIi->getOperand(1).print(errs());
									errs() << "\n";
							}
*/
						}
					}
				}
			}

			if (zomtagUtils->isStore(*MIi))
			{
				unsigned x;
				x = MIi->getOperand(1).getReg();

				if (zomtagUtils->isStorePair(*MIi))
					x = MIi->getOperand(2).getReg();

				if (x != AArch64::SP && x != AArch64::FP)
				{
					if (MIi->getOperand(1).isReg())
					{
/*
						const auto &DL = MIi->getDebugLoc();
						unsigned src = MIi->getOperand(1).getReg();
					
						if (zomtagUtils->isStorePair(*MIi))
							src = MIi->getOperand(2).getReg();
					
						const unsigned DstReg = AArch64::X15;
						const unsigned Imm = 0x7fbe;
						//const unsigned Imm = 0x1;
					
						BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVZXi), AArch64::X15)
										.addImm(Imm)
										.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSL, 32));

						BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), AArch64::X15)
										.addReg(AArch64::X15)
										.addReg(src)
										.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 5));

						BuildMI(MBB, MIi, DL, TII->get(AArch64::LDRBBui), AArch64::XZR).addReg(AArch64::X15).addImm(0);
*/

						const auto &DL = MIi->getDebugLoc();
						unsigned src = MIi->getOperand(1).getReg();
						unsigned target = MIi->getOperand(0).getReg();
					
						if (zomtagUtils->isStorePair(*MIi))
							src = MIi->getOperand(2).getReg();
					
						const unsigned DstReg = AArch64::X15;
						const unsigned Imm = 0x7fbe;
						
						if (option_imprecise1)
						{
							BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVZXi), AArch64::X15)
											.addImm(Imm)
											.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSL, 32));

							if (!(zomtagUtils->isPrePostIndexed(*MIi)))
							{
								BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), AArch64::X15)
												.addReg(AArch64::X15)
												.addReg(src)
												.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 5));

								BuildMI(MBB, MIi, DL, TII->get(AArch64::LDRBBui), AArch64::XZR).addReg(AArch64::X15).addImm(0);
							}
						}

						if (option_imprecise2)
						{
							BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVZXi), AArch64::X15)
											.addImm(Imm)
											.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSL, 32));

							BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), AArch64::X15)
											.addReg(AArch64::X15)
											.addReg(AArch64::XZR)
											.addImm(0);

							BuildMI(MBB, MIi, DL, TII->get(AArch64::LDRBBui), AArch64::XZR).addReg(AArch64::X15).addImm(0);
						}

						if (option_nop)
						{
							BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVZXi), AArch64::X15)
											.addImm(Imm)
											.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSL, 32));

							BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), AArch64::X15)
											.addReg(AArch64::X15)
											.addReg(src)
											.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 5));
							BuildMI(MBB, MIi, DL, TII->get(AArch64::HINT)).addImm(0);
						}

						if (option_precise)
						{
							BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVZXi), AArch64::X15)
											.addImm(Imm)
											.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSL, 32));

							// Non-Pre/PostIndexed Load
							if (!(zomtagUtils->isPrePostIndexed(*MIi)) &&
									!(zomtagUtils->isQReg(MIi->getOperand(0).getReg())))
							{
								// ADD instrumentation
								// Check STP
								if (!(zomtagUtils->isStorePair(*MIi)))
									BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), AArch64::X15)
													.addReg(AArch64::X15)
													.addReg(MIi->getOperand(1).getReg())
													.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 5));
								else
									BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), AArch64::X15)
													.addReg(AArch64::X15)
													.addReg(MIi->getOperand(2).getReg())
													.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 5));
								
								// LDAR instrumentation
								if (!(zomtagUtils->isStorePair(*MIi)))
								{
									if (MIi->getOperand(0).getReg() == MIi->getOperand(1).getReg() ||
											zomtagUtils->getCorrespondingReg(MIi->getOperand(1).getReg()) == MIi->getOperand(0).getReg())
										BuildMI(MBB, MIi, DL, TII->get(AArch64::LDARB), AArch64::XZR).addReg(AArch64::X15).addImm(0);
										//int a = 0;
									else
									{
										if (MIi->getNumOperands() <= 4)
											BuildMI(MBB, MIi, DL, TII->get(AArch64::LDARB), AArch64::XZR).addReg(AArch64::X15).addImm(0);
										else
											MIi->print(errs());
									}
								}
								else // STP
								{
									if (MIi->getOperand(0).getReg() == MIi->getOperand(2).getReg() ||
											zomtagUtils->getCorrespondingReg(MIi->getOperand(2).getReg()) == MIi->getOperand(0).getReg() ||
											MIi->getOperand(1).getReg() == MIi->getOperand(2).getReg() ||
											zomtagUtils->getCorrespondingReg(MIi->getOperand(2).getReg()) == MIi->getOperand(1).getReg())
										BuildMI(MBB, MIi, DL, TII->get(AArch64::LDARB), AArch64::XZR).addReg(AArch64::X15).addImm(0);
									else
										BuildMI(MBB, MIi, DL, TII->get(AArch64::LDARB), AArch64::XZR).addReg(AArch64::X15).addImm(0);
								}
							}

							if (!(zomtagUtils->isPrePostIndexed(*MIi)) &&
									(zomtagUtils->isQReg(MIi->getOperand(0).getReg())))
							{
								if (!(zomtagUtils->isStorePair(*MIi)))
								{
									BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), AArch64::X15)
													.addReg(AArch64::X15)
													.addReg(MIi->getOperand(1).getReg())
													.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 5));
									BuildMI(MBB, MIi, DL, TII->get(AArch64::LDRQui), AArch64::XZR).addReg(AArch64::X15).addImm(0);
								}
							}

/*
							if ((zomtagUtils->isPrePostIndexed(*MIi)) &&
									!(zomtagUtils->isQReg(MIi->getOperand(0).getReg())))
							{
									MIi->print(errs());
									errs() << "\n";
									MIi->getOperand(0).print(errs());
									errs() << "\n";
									MIi->getOperand(1).print(errs());
									errs() << "\n";
							}
*/
						}
					}
				}
			}

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

        BuildMI(MBB, MIi, DL, TII->get(op), dst).addReg(src).addReg(off_w).addImm(ext).addImm(amount);
      
        auto tmp = MIi;
        MIi--;
        tmp->removeFromParent();
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

