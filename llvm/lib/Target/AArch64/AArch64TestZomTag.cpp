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
#define x15 AArch64::X15
#define xzr AArch64::XZR

using namespace llvm;

/* Prototypes */
// void instrumentTagLoading(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);

/* Options */
static cl::opt<bool> option_default("zometag-default", cl::desc("Instrument default zometag"));
static cl::opt<bool> option_tl("zometag-tl", cl::desc("Instrument tag loading overhead"));
static cl::opt<bool> option_tl_nop("zometag-tl-nop", cl::desc("Instrument tag loading ovh (nop)"));
static cl::opt<bool> option_tl_pre("zometag-tl-pre", cl::desc("Instrument tag loading ovh (pre)"));
static cl::opt<bool> option_tl_imp1("zometag-tl-imp1", cl::desc("Instrument tag loading ovh (imp1)"));
static cl::opt<bool> option_tl_imp2("zometag-tl-imp2", cl::desc("Instrument tag loading ovh (imp2)"));

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
			void instrumentTagLoading(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);
			void instrumentZoneIsolation(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);

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

			instrumentTagLoading(MBB, MIi);

			if (option_default)
				instrumentZoneIsolation(MBB, MIi);
    }
  }
  return true;
}

void TestZomTag::instrumentTagLoading(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi)
{

	unsigned dst;
	unsigned mem;
	const auto &DL = MIi->getDebugLoc();
	const unsigned imm = 0x7fbe;

	if (zomtagUtils->isPrePostIndexed(*MIi))
		return;

	if (zomtagUtils->isLoad(*MIi))
	{
		if (!(zomtagUtils->isLoadPair(*MIi)))
		{
			dst = MIi->getOperand(0).getReg();
			mem = MIi->getOperand(1).getReg();

			if (mem != AArch64::SP && mem != AArch64::FP)
			{
				// MOV X15, 0x7fbe, LSL, 32
				BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVZXi), x15)
						.addImm(imm)
						.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSL, 32));
				
				if (option_tl_nop)
				{
					BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), x15)
							.addReg(x15)
							.addReg(mem)
							.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 5));
					BuildMI(MBB, MIi, DL, TII->get(AArch64::HINT)).addImm(0);
				}
				if (option_tl_imp1)
				{
					BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), x15)
							.addReg(x15)
							.addReg(mem)
							.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 5));
					BuildMI(MBB, MIi, DL, TII->get(AArch64::LDRBBui), xzr)
							.addReg(x15)
							.addImm(0);
				}
				if (option_tl_imp2)
				{
					BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), x15)
							.addReg(x15)
							.addReg(xzr)
							.addImm(0);
					BuildMI(MBB, MIi, DL, TII->get(AArch64::LDRBBui), xzr)
							.addReg(x15)
							.addImm(0);
				}
				if (option_tl_pre)
				{
					assert(mem.isReg());
					BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), x15)
						.addReg(x15)
						.addReg(mem)
						.addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 5));

					auto MIOpcode = zomtagUtils->isQReg(dst) ? AArch64::LDRQui : AArch64::LDARB;

					if ((dst == mem) ||
							(dst == zomtagUtils->getCorrespondingReg(mem)))
						BuildMI(MBB, MIi, DL, TII->get(MIOpcode), xzr)
							.addReg(x15)
							.addImm(0);
					else
						BuildMI(MBB, MIi, DL, TII->get(MIOpcode), dst)
							.addReg(x15)
							.addImm(0);
				}
			}
		}
		
		else // Load Pair
		{

		}
	}

	return;
}

void TestZomTag::instrumentZoneIsolation(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi)
{
	if (zomtagUtils->isInterestingLoad(*MIi) || zomtagUtils->isInterestingStore(*MIi))
	{
		const auto &DL = MIi->getDebugLoc();
		const auto op = zomtagUtils->getCorrespondingLoadStore(MIi->getOpcode());
		const unsigned dst = MIi->getOperand(0).getReg();
		const unsigned src = MIi->getOperand(1).getReg();
		const unsigned off_x = MIi->getOperand(2).getReg();
		const unsigned off_w = zomtagUtils->getCorrespondingReg(off_x);
		const int64_t ext = AArch64_AM::SXTW;
		const int64_t sft = MIi->getOperand(4).getImm();

		BuildMI(MBB, MIi, DL, TII->get(op), dst).addReg(src).addReg(off_w).addImm(ext).addImm(sft);

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
/*	
			auto tmp = MIi + 1;	
	
			MIi->print(errs());
			MachineInstrBuilder MIB =
			BuildMI(MBB, MIi, DL, II, AArch64::XZR)
							.addReg(ptr_reg)
							.addImm(AArch64_AM::encodeLogicalImmediate(imm, 64));
			errs() << "********** TST ";
			MIB->print(errs());
*/
		}
	}
	return;
}
