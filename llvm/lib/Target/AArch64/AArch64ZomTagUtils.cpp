#include <llvm/IR/Constants.h>
#include "AArch64ZomTagUtils.h"

using namespace llvm;

ZomTagUtils::ZomTagUtils(const TargetRegisterInfo *TRI,
  const TargetInstrInfo *TII):TII(TII),TRI(TRI)
{
};

bool ZomTagUtils::isLoad(const MachineInstr &MI)
{
  const auto opCode = MI.getOpcode();
  switch(opCode)
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
    //case AArch64::LDURSBWi:
    //case AArch64::LDURSHWi:
    //case AArch64::LDURWi:
    //case AArch64::LDURXi:
    //case AArch64::LDURSBXi:
    //case AArch64::LDURSHXi:
    //case AArch64::LDURSWi:
    //case AArch64::LDURXi:
    case AArch64::LDRSBWui:
    case AArch64::LDRSHWui:
    //case AArch64::LDRWui:
    //case AArch64::LDRXui:
    case AArch64::LDRSBWroX:
    case AArch64::LDRSHWroX:
    case AArch64::LDRWroX:
    case AArch64::LDRXroX:
    case AArch64::LDRSBXroX:
    case AArch64::LDRSHXroX:
    case AArch64::LDRSWroX:
    //case AArch64::LDRXroX:
    case AArch64::LDRSBWroW:
    case AArch64::LDRSHWroW:
    case AArch64::LDRWroW:
    case AArch64::LDRXroW:
    case AArch64::LDRSBXroW:
    case AArch64::LDRSHXroW:
    case AArch64::LDRSWroW:
    //case AArch64::LDRXroW:
    //case AArch64::LDURBBi:
    //case AArch64::LDURHHi:
    //case AArch64::LDURWi:
    //case AArch64::LDURXi:
    //case AArch64::LDURBBi:
    //case AArch64::LDURHHi:
    //case AArch64::LDURWi:
    //case AArch64::LDURXi:
    //case AArch64::LDRBBui:
    //case AArch64::LDRHHui:
    //case AArch64::LDRWui:
    //case AArch64::LDRXui:
    //case AArch64::LDRBBui:
    //case AArch64::LDRHHui:
    //case AArch64::LDRWui:
    //case AArch64::LDRXui:
    case AArch64::LDRBBroX:
    case AArch64::LDRHHroX:
    //case AArch64::LDRWroX:
    //case AArch64::LDRXroX:
    //case AArch64::LDRBBroX:
    //case AArch64::LDRHHroX:
    //case AArch64::LDRWroX:
    //case AArch64::LDRXroX:
    case AArch64::LDRBBroW:
    case AArch64::LDRHHroW:
    //case AArch64::LDRWroW:
    //case AArch64::LDRXroW:
    //case AArch64::LDRBBroW:
    //case AArch64::LDRHHroW:
    //case AArch64::LDRWroW:
    //case AArch64::LDRXroW:
    //case AArch64::LDURSi:
    //case AArch64::LDURDi:
    //case AArch64::LDRSui:
    //case AArch64::LDRDui:
    case AArch64::LDRSroX:
    case AArch64::LDRDroX:
    case AArch64::LDRSroW:
    case AArch64::LDRDroW:
      return true; 
  }
}
