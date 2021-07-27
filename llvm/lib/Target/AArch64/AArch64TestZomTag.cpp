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
#include "llvm/IR/GlobalVariable.h"
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
static cl::opt<bool> option_tl_sparc("zometag-tl-sparc", cl::desc("Instrument tag loading ovh (sparc)"));

namespace llvm
{
  void initializeTestZomTagPass(PassRegistry &);
}

namespace
{
  class TestZomTag : public ModulePass
  {
  public:
    static char ID;
    TestZomTag() : ModulePass(ID) {
      initializeTestZomTagPass(*PassRegistry::getPassRegistry());    
    }
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<MachineModuleInfo>();
      // AU.setPreservesAll();
      ModulePass::getAnalysisUsage(AU);
    }
    StringRef getPassName() const override { return "zomtag-ptr"; }

    bool doInitialization(Module &M) override;
    bool runOnModule(Module &M) override;
    bool machineFunctionDo(MachineFunction &F, bool &initialized, GlobalValue *GV);
    void instrumentTagLoading(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi, GlobalValue *GV);
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

ModulePass *llvm::createAArch64TestZomTagPass()
{
  return new TestZomTag();
}

char TestZomTag::ID = 0;

bool TestZomTag::doInitialization(Module &M)
{
  return false;
}

bool TestZomTag::runOnModule(Module &M)
{
  //get global var
  // IRBuilder<> Builder(M.getContext());
  // M.getOrInsertGlobal("__security_cookie",
  //                     Type::getInt8PtrTy(M.getContext()));
  bool initialized = false;
  errs() << "@@@begin debug\n";
  M.getOrInsertGlobal("__mte_tag_mem", Type::getInt8PtrTy(M.getContext()));
  errs() << "@@@__mte_Tag_mem\n";
  GlobalVariable *GV = M.getNamedGlobal("__mte_tag_mem");
  errs() << "@@@get or insert global : 0x"<< GV << "\n";
  errs() << "@@@get or insert global2 : 0x"<< *GV << "\n";
  MachineModuleInfo &MMI = getAnalysis<MachineModuleInfo>();
  errs() << "@@@MachineModuleInfo\n";
  for (Function &F : M){
    MachineFunction &MF = MMI.getMachineFunction(F);
    errs() << "@@@get machine function\n";
    machineFunctionDo(MF, initialized, (GlobalValue*)GV);
    errs() << "@@@run machine funciton\n";
  }

    // MachineFunction::MachineFunction(Function &F,const LLVMTargetMachine &Target, const TargetSubtargetInfo &STI, unsigned FunctionNum, MachineModuleInfo &MMI)
    // runOnMahcineFunction(llvm::MachineFunction(&F, ));
}

bool TestZomTag::machineFunctionDo(MachineFunction &MF, bool& initialized, GlobalValue *GV)
{
  TM = &MF.getTarget();
  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  TRI = STI->getRegisterInfo();
  errs() << "@@@GV : " << GV <<"\n";

  zomtagUtils = ZomTagUtils::get(TRI, TII);

  for (auto &MBB : MF)
    {
      errs()<< "@@@MBB\n" ;
      for (auto MIi = MBB.instr_begin(); MIi != MBB.instr_end(); MIi++)
        {
	  errs()<< "@@@MTi\n";
          if (option_tl_nop || option_tl_imp1 ||
              option_tl_pre || option_tl_imp2){}
            // instrumentTagLoading(MBB, MIi, GV);

          // if (option_default)
          //   instrumentZoneIsolation(MBB, MIi);
        }
    }
  return true;
}

void TestZomTag::instrumentTagLoading(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi, GlobalValue* GV)
{

  unsigned dst;
  unsigned mem;
  const auto &DL = MIi->getDebugLoc();
  // const unsigned imm = 0x7fbe;

  if (zomtagUtils->isPrePostIndexed(*MIi))
    return;

  if (!(MIi->getOperand(0).isReg()) ||
      !(MIi->getOperand(1).isReg()))
    return;

  if (MIi->getNumOperands() > 4)
    return;

  if (zomtagUtils->isLoad(*MIi) || zomtagUtils->isStore(*MIi))
    {
      dst = MIi->getOperand(0).getReg();
      mem = MIi->getOperand(1).getReg();
      if (zomtagUtils->isLoadPair(*MIi) ||
          zomtagUtils->isStorePair(*MIi)){
	errs()<<"@@@isPair? \n";
        mem = MIi->getOperand(2).getReg();
	errs()<<"@@@mem "<<mem<<"\n";
	errs()<<"@@@MIi->getOperand(2).getReg()"<<MIi->getOperand(2).getReg()<<"\n";
	
      }

      if (mem != AArch64::SP && mem != AArch64::FP)
        {

	  errs()<< "@@@init\n";

	  errs()<< "@@@addGV begin\n";
	  // MOV X15, GV
	  BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVZXi), x15)
	    .addGlobalAddress(GV);
	  errs()<< "@@@addGV end\n";
	    //   .addImm(AArch64_AM::getShifterImm(AArch64_AM::LSL, 32));

          /*commented out for Vatalloc*/
          // MOV X15, 0x7fbe, LSL, 32
          // BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVZXi), x15)
          //   .addImm(imm)
          //   .addImm(AArch64_AM::getShifterImm(AArch64_AM::LSL, 32));

	  errs()<< "@@@option_tl_imp1 : "<<option_tl_imp1<<"\n";
	  errs()<< "@@@option_tl_imp2 : "<<option_tl_imp2<<"\n";
	  errs()<< "@@@option_tl_nop : "<<option_tl_nop<<"\n";
	  errs()<< "@@@option_tl_pre : "<<option_tl_pre<<"\n";
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
	      errs()<<"tl_imp1\n";
              if (!option_tl_sparc)
                BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), x15)
                  .addReg(x15)
                  .addReg(mem)
                  .addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 5));
				
              if (option_tl_sparc)
                BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), x15)
                  .addReg(x15)
                  .addReg(mem)
                  .addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 7));
				
              BuildMI(MBB, MIi, DL, TII->get(AArch64::LDRBBui), xzr)
                .addReg(x15)
                .addImm(0);
            }

          if (option_tl_imp2)
            {
              if (!option_tl_sparc)
                BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), x15)
                  .addReg(x15)
                  .addReg(xzr)
                  .addImm(0);
				
              if (option_tl_sparc)				
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
	      errs() <<"@@@@tl_pre\n";
              assert(mem.isReg());
				
              if (!option_tl_sparc)
                BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), x15)
                  .addReg(x15)
                  .addReg(mem)
                  .addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 5));
              if (option_tl_sparc)				
                BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXrs), x15)
                  .addReg(x15)
                  .addReg(mem)
                  .addImm(AArch64_AM::getShifterImm(AArch64_AM::LSR, 7));


              auto MIOpcode = zomtagUtils->isQReg(dst) ? AArch64::LDRQui : AArch64::LDARB;

              if ((dst == mem) ||
                  (dst == zomtagUtils->getCorrespondingReg(mem)) ||
                  (zomtagUtils->isStore(*MIi)))
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
