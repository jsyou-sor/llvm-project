#include "AArch64.h"
#include "AArch64Subtarget.h"
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

#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"
#include "AArch64IKEA.h"

using namespace llvm;
using namespace llvm::IKEA;

#define DEBUG_TYPE "aarch64-ikea-memory_isolation_pass"

static cl::opt<bool>
EnableMPTSFI("mpt-sfi-mode", cl::Hidden, cl::ZeroOrMore,
             cl::desc("use SFI for MPT instrumentation"),
             cl::init(false));

namespace llvm {
  void initializeAArch64IKEAPass(PassRegistry &);
}

#define AARCH64_IKEAPASS_NAME "AArch64 IKEA Pass"

#define X15   AArch64::X15
#define SP    AArch64::SP
#define FP    AArch64::FP

namespace {
  struct AArch64IKEA : public MachineFunctionPass {
    static char ID;
    AArch64IKEA() : MachineFunctionPass(ID) {
      initializeAArch64IKEAPass(*PassRegistry::getPassRegistry());
    }

    int count = 0;

    const TargetMachine *TM             = nullptr;
    const AArch64Subtarget *STI         = nullptr;
    const AArch64InstrInfo *TII         = nullptr;
    const AArch64RegisterInfo *TRI      = nullptr;

    // Add defined functions here (like .h)
		bool runOnFunction(Function &F);
    bool runOnMachineFunction(MachineFunction &Fn) override;
    StringRef getPassName() const override {
      return AARCH64_IKEAPASS_NAME;
    }
  };
  char AArch64IKEA::ID = 0;
}

INITIALIZE_PASS(AArch64IKEA, "aarch64-ikea-memory_isolation_pass", AARCH64_IKEAPASS_NAME, false, false)

FunctionPass *llvm::createAArch64IKEAPass() {
  dbgs() << "[+] AArch64IKEAPass\n";
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
bool isbranch(const MachineInstr &MI) {
  const auto Opcode = MI.getOpcode();
  switch(Opcode) {
  default:
    return false;
  case AArch64::BLR:
  case AArch64::BR:
    return true;
  }
}
/*
bool instrumentLoad(const MachineInstr &MI) {
  const auto &DL = MI.getDebugLoc();
  unsigned src = 0;
 
  if (MI.getNumOperands() == 3) {
    src = MI.getOperand(1).getReg();
    if (src != SP && src != FP)
      BuildMI(MFI, MBBI, DL, TII->get(AArch64::BFMXri), X15).addReg(X15).addReg(src).addImm(0).addImm(55);
    else {
      // TODO: Instrument for SP
    }
  }
  
  if (MI.getNumOperands() == 4) {
    src = MI.getOperand(2).getReg();
    if (src != SP && src != FP)
      BuildMI(MFI, MBBI, DL, TII->get(AArch64::BFMXri), X15).addReg(X15).addReg(src).addImm(0).addImm(55);
    else {
      // TODO: Instrument for SP
    }
  }
    //Modified = true;
  return true;
}
*/

bool AArch64IKEA::runOnFunction(Function &F) {
	dbgs() << "IR Pass\n";
	dbgs() << "[IR]: " << F.getName() << "\n";
	return false;
}


bool AArch64IKEA::runOnMachineFunction(MachineFunction &Fn) {

  dbgs() << getPassName() << ", function " << Fn.getName() << "\n";
  IkeaMetaData be;
	dbgs() << "[BACKEND] " << be.get_address(0) << "\n";
	
	TM = &Fn.getTarget();
  STI = &Fn.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  TRI = STI->getRegisterInfo();
  bool Modified = false;

  //Fn.dump();

  if (Fn.getName().compare(StringRef("__sfputc_r")) == 0)
    return Modified;

  for (MachineBasicBlock &MFI : Fn) {
    
    MachineBasicBlock::iterator MBBI = MFI.begin(), MBBIE = MFI.end();
    
    while (MBBI != MBBIE) {

      MachineBasicBlock::iterator NMBBI = std::next(MBBI); 
      MachineInstr &MI = *MBBI;
      const auto &DL = MI.getDebugLoc();
      unsigned src = 0;

			//Instruction &insn = *MBBI;

      // Load Instrumentation
      if (isLoad(MI)) {
        if (MI.getNumOperands() == 3) {

          if(!MI.getOperand(1).isReg()){
            MBBI = NMBBI;
            continue;
          }
          
          src = MI.getOperand(1).getReg();
          if (src != SP && src != FP)
            BuildMI(MFI, MBBI, DL, TII->get(AArch64::BFMXri), X15).addReg(X15).addReg(src).addImm(0).addImm(55);
          else {
            // TODO: Instrument for SP
          }
        }
        if (MI.getNumOperands() == 4) {
          
          if(!MI.getOperand(2).isReg()){
            MBBI = NMBBI;
            continue;
          }
          
          src = MI.getOperand(2).getReg();
          if (src != SP && src != FP)
            BuildMI(MFI, MBBI, DL, TII->get(AArch64::BFMXri), X15).addReg(X15).addReg(src).addImm(0).addImm(55);
          else {
            // TODO: Instrument for SP
          }
        }
        Modified = true;
      }
      
      /*
      if (isLoad(MI))
        Modified = instrumentLoad(MI);
      */

      // Store instrumentation
      if (isStore(MI)) {
        if (MI.getNumOperands() == 3) {
          
          if(!MI.getOperand(1).isReg()){
            MBBI = NMBBI;
            continue;
          }
          
          src = MI.getOperand(1).getReg();
          if (src != SP && src != FP) {
            // TODO: Why add LDR for STR instrumentation?
            BuildMI(MFI, MBBI, DL, TII->get(AArch64::LDRXui), AArch64::XZR).addReg(src).addImm(0);
            BuildMI(MFI, MBBI, DL, TII->get(AArch64::BFMXri), X15).addReg(X15).addReg(src).addImm(0).addImm(55);
          }
          else {
            // TODO: instrument for SP
          }
        }
        if (MI.getNumOperands() == 4) {
          
          if(!MI.getOperand(2).isReg()){
            MBBI = NMBBI;
            continue;
          }
          
          src = MI.getOperand(2).getReg();
          if (src != SP && src != FP) {
            BuildMI(MFI, MBBI, DL, TII->get(AArch64::LDRXui), AArch64::XZR).addReg(src).addImm(0);                
            BuildMI(MFI, MBBI, DL, TII->get(AArch64::BFMXri), X15).addReg(X15).addReg(src).addImm(0).addImm(55);
          }
          else {
            // TODO: instrument for SP
          }
        }
      }
      
//--------------- CFI ---------------
      
      // Indirect branch instrumentation
      if (isbranch(MI)) {
        if(!MI.getOperand(0).isReg()){
          MBBI = NMBBI;
          continue;
        }
        
        src = MI.getOperand(0).getReg();
        BuildMI(MFI, MBBI, DL, TII->get(AArch64::BFMXri), X15).addReg(X15).addReg(src).addImm(0).addImm(55);
        // FIXME: This load might not work when running on board
        // FIXME: Use second BuildMI for overhead evaluation
        //BuildMI(MFI, MBBI, DL, TII->get(AArch64::LDRXui), AArch64::XZR).addReg(X15).addImm(0);
        BuildMI(MFI, MBBI, DL, TII->get(AArch64::LDRXui), AArch64::XZR).addReg(src).addImm(0);
      }

      //RET instrumentation
      if (MI.getOpcode() == AArch64::RET) {
        if(!MI.getOperand(0).isReg()){
          MBBI = NMBBI;
          continue;
        }
        
        src = MI.getOperand(0).getReg();
        BuildMI(MFI, MBBI, DL, TII->get(AArch64::BFMXri), X15).addReg(X15).addReg(src).addImm(0).addImm(55);
        // FIXME: This load might not work when running on board
        // FIXME: Use second BuildMI for overhead evaluation
        //BuildMI(MFI, MBBI, DL, TII->get(AArch64::LDRXui), AArch64::XZR).addReg(X15).addImm(0);
        BuildMI(MFI, MBBI, DL, TII->get(AArch64::LDRXui), AArch64::XZR).addReg(src).addImm(0);
      }

      //Indirect CALL instrumentation
      if (MI.getOpcode() == 0) {
        // TODO: Instrument for indirecto call        
        assert(1);
      }

      MBBI = NMBBI;
    }
  }

  return Modified;
}
