//<<<<<<< HEAD
// #include ""
// AAA
//=======
#include "AArch64.h"
#include "AArch64Subtarget.h"
//#include "llvm/CodeGen/MachineOperand.h"
//#include "llvm/CodeGen/MachineBasicBlock.h"
//#include "llvm/CodeGen/MachineFunctionPass.h"
//#include "llvm/CodeGen/MachineRegisterInfo.h"
//#include "llvm/CodeGen/MachineInstrBuilder.h"
//#include "llvm/CodeGen/MachineInstr.h"

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

#include "MCTargetDesc/AArch64AddressingModes.h"

using namespace llvm;

#define DEBUG_TYPE "aarch64-ikea-memory_isolation_pass"

static cl::opt<bool>
EnableMPTSFI("mpt-sfi-mode", cl::Hidden, cl::ZeroOrMore,
             cl::desc("use SFI for MPT instrumentation"),
             cl::init(false));

namespace llvm {
  void initializeAArch64IKEAPass(PassRegistry &);
}

#define AARCH64_IKEAPASS_NAME "AArch64 IKEA Pass"

namespace {
  struct AArch64IKEA : public MachineFunctionPass {
    static char ID;
    AArch64IKEA() : MachineFunctionPass(ID) {
      initializeAArch64IKEAPass(*PassRegistry::getPassRegistry());
    }

    //const TargetInstrInfo   *TII;
    //const AArch64Subtarget  *STI;
    //MachineRegisterInfo     *MRI;
    int count = 0;

    // PARTS
    const TargetMachine *TM             = nullptr;
    const AArch64Subtarget *STI         = nullptr;
    const AArch64InstrInfo *TII         = nullptr;
    const AArch64RegisterInfo *TRI     = nullptr;

    // Add defined functions here (like .h)

    bool runOnMachineFunction(MachineFunction &Fn) override;
    StringRef getPassName() const override {
      return AARCH64_IKEAPASS_NAME;
    }
  };
  char AArch64IKEA::ID = 0;
}

INITIALIZE_PASS(AArch64IKEA, "aarch64-ikea-memory_isolation_pass", AARCH64_IKEAPASS_NAME, false, false)

FunctionPass *llvm::createAArch64IKEAPass() {
  dbgs() << "AArch64IKEAPass\n";
  return new AArch64IKEA();
}

bool isLoad(const MachineInstr &MI) {
  const auto Opcode = MI.getOpcode();
  switch(Opcode) {
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

bool isStore(const MachineInstr &MI) {
  const auto Opcode = MI.getOpcode();
  switch(Opcode) {
  default:
    return false;
    //case AArch64::LDRW:
    //case AArch64::LDRX:
    //case AArch64::LDRB:
    //case AArch64::LDRH:
    //return true;
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

bool is32bit_reg(unsigned reg) {
  switch(reg) {
  default:
    return false;
  case AArch64::W0:
  case AArch64::W1:
  case AArch64::W2:
  case AArch64::W3:
  case AArch64::W4:
  case AArch64::W5:
  case AArch64::W6:
  case AArch64::W7:
  case AArch64::W8:
  case AArch64::W9:
  case AArch64::W10:
  case AArch64::W11:
  case AArch64::W12:
  case AArch64::W13:
  case AArch64::W14:
  case AArch64::W15:
  case AArch64::W16:
  case AArch64::W17:
  case AArch64::W18:
  case AArch64::W19:
  case AArch64::W20:
  case AArch64::W21:
  case AArch64::W22:
  case AArch64::W23:
  case AArch64::W24:
  case AArch64::W25:
  case AArch64::W26:
  case AArch64::W27:
  case AArch64::W28:
  case AArch64::W29:
  case AArch64::W30:
  case AArch64::WSP:
  case AArch64::WZR:
    return true;
  }
}

bool AArch64IKEA::runOnMachineFunction(MachineFunction &Fn) {

  dbgs() << getPassName() << ", function " << Fn.getName() << "\n";
  TM = &Fn.getTarget();
  STI = &Fn.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  TRI = STI->getRegisterInfo();
  bool Modified = false;

  Fn.dump();

  if (Fn.getName().compare(StringRef("__sfputc_r")) == 0)
    return Modified;

  for (MachineBasicBlock &MFI : Fn) {
    MachineBasicBlock::iterator MBBI = MFI.begin(), MBBIE = MFI.end();
    while (MBBI != MBBIE) {
      MachineBasicBlock::iterator NMBBI = std::next(MBBI);
      MachineInstr &MI = *MBBI;

      if (isLoad(MI)) {

        const auto &DL = MI.getDebugLoc();

        //if (MI.getOperand(1).isReg()) {
        if (MI.getNumOperands() == 3) {
          unsigned mod = MI.getOperand(1).getReg();

          /* TODO:
           * [0] Where to add the pass
           *      - addPass@PreEmitPass   => gets all the load (including SP referenced ones)
           * 
           * [1] Reserve X15 register
           *      - Reserved.set(AArch64::X15) && addPass@PreRegAlloc => error
           *      - markSuperRegs(Reserved, AArch64::X15) @ AArch64RegisterInfo.cpp
                 * 
                 * [2] Get immediates also
                 *      - Or we can check/verify immediates large enough to change the tag bits are not added
                 *        during memory access addressing
                 * 
                 * [3] Change ADD to MOV && MOV only bottom 60 bits
                 *      - EXTR X15, addr, X15, 60
                 *      - EXTR X15, X15, X15, 4     == ROR X15, X15, 4 (after compilation)
                 * 
                 * [4] Exchange LD memory operand to X15 (no need for the overhead experiment)
                 *      - ARMv8-A Addressing Modes
                 *          + Simple:       LDR X0, [X1]
                 *          + Offset:       LDR X0, [X1, #12]
                 *          + Pre-index:    LDR X0, [X1, #12]!      // X1 = X1 + 12 before memory access
                 *          + Post-index:   LDR X0, [X1], #12       // X1 = X1 + 12 after memory access
                 *          + LD/ST Pair:   LDP X0, X1, [X2]        // X0 = [X2], X1 = [X2 + 8]
                 *                          LDP X0, X1, [SP], #16   // X0 = [SP], X1 = [SP + 8], SP = SP + 16
                 *
                 * [5] Tag stack at function beginning
                 * [6] Instrument memory access via SP
                 *      - What about memory access via FP(X29)?
                 */

          const MCInstrDesc extr = TII->get(AArch64ISD::EXTR);
          if (mod != AArch64::SP) {

            BuildMI(MFI, MBBI, DL, TII->get(AArch64::EXTRXrri), AArch64::X15).addReg(mod).addReg(AArch64::X15).addImm(60);
            BuildMI(MFI, MBBI, DL, TII->get(AArch64::EXTRXrri), AArch64::X15).addReg(AArch64::X15).addReg(AArch64::X15).addImm(4);

            /*
             * Change address operand to X15 (if MTE is available)
             * unsigned idx = 0;
             * for (unsigned i = 0; i < MI.getNumOperands(); i++)
             *      if (!MI.getOperand(i).isReg())
             *          idx = i - 1;
             *  MI.getOperand(idx).ChangeToRegister(AArch64::X15, 1, 0, 0, 0, 0, 0);
             */
          }
          else {
            // Have to move SP to GPR(X14) first
          }
        }
        Modified = true;
      }
      MBBI = NMBBI;
    }
  }

  return Modified;
}
//>>>>>>> 166f31090903fbf1746e32d088c322fce0189441
