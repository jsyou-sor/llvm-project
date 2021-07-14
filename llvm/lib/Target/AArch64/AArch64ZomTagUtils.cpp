#include <llvm/IR/Constants.h>
#include "AArch64ZomTagUtils.h"

using namespace llvm;

ZomTagUtils::ZomTagUtils(const TargetRegisterInfo *TRI,
  const TargetInstrInfo *TII):TII(TII),TRI(TRI)
{
};

bool ZomTagUtils::isAddSub(MachineInstr &MI)
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

unsigned ZomTagUtils::getCorrespondingStore(const unsigned opCode)
{
  switch(opCode)
  {
    default:
      return opCode;
    case AArch64::STRBBroX:
      return AArch64::STRBBroW;
    case AArch64::STRHHroX:
      return AArch64::STRHHroW;
    case AArch64::STRWroX:
      return AArch64::STRWroW;
    case AArch64::STRXroX:
      return AArch64::STRXroW;
    case AArch64::STRSroX:
      return AArch64::STRSroW;
    case AArch64::STRDroX:
      return AArch64::STRDroW; 
  }
}

bool ZomTagUtils::isInterestingStore(const MachineInstr &MI)
{
  const auto opCode = MI.getOpcode();
  switch(opCode)
  {
    default:
      return false;
    case AArch64::STRBBroX:
    case AArch64::STRHHroX:
    case AArch64::STRWroX:
    case AArch64::STRXroX:
    case AArch64::STRSroX:
    case AArch64::STRDroX:
      return true;
  }
}

bool ZomTagUtils::isInterestingLoad(const MachineInstr &MI)
{
  const auto opCode = MI.getOpcode();
  switch(opCode)
  {
    default:
      return false;
    case AArch64::LDRSBWroX:
    case AArch64::LDRSHWroX:
    case AArch64::LDRWroX:
    case AArch64::LDRXroX:
    case AArch64::LDRSBXroX:
    case AArch64::LDRSHXroX:
    case AArch64::LDRSWroX:
    case AArch64::LDRBBroX:
    case AArch64::LDRHHroX:
    case AArch64::LDRSroX:
    case AArch64::LDRDroX:
      return true;
  }
}

bool ZomTagUtils::isRegisterOffsetLoad(const MachineInstr &MI)
{
  const auto opCode = MI.getOpcode();
  switch(opCode)
  {
    default:
      return false;
    case AArch64::LDRSBWroW:
    case AArch64::LDRSHWroW:
    case AArch64::LDRWroW:
    case AArch64::LDRXroW:
    case AArch64::LDRSBXroW:
    case AArch64::LDRSHXroW:
    case AArch64::LDRSWroW:
    case AArch64::LDRBBroW:
    case AArch64::LDRHHroW:
    case AArch64::LDRSroW:
    case AArch64::LDRDroW:
      return true; 
  }
}

bool ZomTagUtils::isPrePostIndexed(const MachineInstr &MI)
{
	const auto opCode = MI.getOpcode();
	switch(opCode)
	{
		default:
			return false;
		case AArch64::LDRSBWpre:
  	case AArch64::LDRSHWpre:
  	case AArch64::STRBBpre:
  	case AArch64::LDRBBpre:
  	case AArch64::STRHHpre:
  	case AArch64::LDRHHpre:
  	case AArch64::STRWpre:
  	case AArch64::LDRWpre:
  	case AArch64::LDRSBWpost:
  	case AArch64::LDRSHWpost:
  	case AArch64::STRBBpost:
  	case AArch64::LDRBBpost:
  	case AArch64::STRHHpost:
  	case AArch64::LDRHHpost:
  	case AArch64::STRWpost:
  	case AArch64::LDRWpost:
  	case AArch64::LDRSBXpre:
  	case AArch64::LDRSHXpre:
	  case AArch64::STRXpre:
  	case AArch64::LDRSWpre:
  	case AArch64::LDRXpre:
  	case AArch64::LDRSBXpost:
  	case AArch64::LDRSHXpost:
  	case AArch64::STRXpost:
  	case AArch64::LDRSWpost:
  	case AArch64::LDRXpost:
  	case AArch64::LDRQpre:
  	case AArch64::STRQpre:
  	case AArch64::LDRQpost:
  	case AArch64::STRQpost:
  	case AArch64::LDRDpre:
  	case AArch64::STRDpre:
  	case AArch64::LDRDpost:
  	case AArch64::STRDpost:
  	case AArch64::LDRSpre:
  	case AArch64::STRSpre:
  	case AArch64::LDRSpost:
  	case AArch64::STRSpost:
  	case AArch64::LDRHpre:
  	case AArch64::STRHpre:
  	case AArch64::LDRHpost:
  	case AArch64::STRHpost:
  	case AArch64::LDRBpre:
  	case AArch64::STRBpre:
  	case AArch64::LDRBpost:
  	case AArch64::STRBpost:
			return true;
	}
}

unsigned ZomTagUtils::getQReg(const unsigned XReg)
{
	switch(XReg)
	{
		default:
			return XReg;
		case AArch64::X0:
      return AArch64::Q0;
    case AArch64::X1:
      return AArch64::Q1;
    case AArch64::X2:
      return AArch64::Q2;
    case AArch64::X3:
      return AArch64::Q3;
    case AArch64::X4:
      return AArch64::Q4;
    case AArch64::X5:
      return AArch64::Q5;
    case AArch64::X6:
      return AArch64::Q6;
    case AArch64::X7:
      return AArch64::Q7;
    case AArch64::X8:
      return AArch64::Q8;
    case AArch64::X9:
      return AArch64::Q9;
    case AArch64::X10:
      return AArch64::Q10;
    case AArch64::X11:
      return AArch64::Q11;
    case AArch64::X12:
      return AArch64::Q12;
    case AArch64::X13:
      return AArch64::Q13;
    case AArch64::X14:
      return AArch64::Q14;
    case AArch64::X15:
      return AArch64::Q15;
    case AArch64::X16:
      return AArch64::Q16;
    case AArch64::X17:
      return AArch64::Q17;
    case AArch64::X18:
      return AArch64::Q18;
    case AArch64::X19:
      return AArch64::Q19;
    case AArch64::X20:
      return AArch64::Q20;
    case AArch64::X21:
      return AArch64::Q21;
    case AArch64::X22:
      return AArch64::Q22;
    case AArch64::X23:
      return AArch64::Q23;
    case AArch64::X24:
      return AArch64::Q24;
    case AArch64::X25:
      return AArch64::Q25;
    case AArch64::X26:
      return AArch64::Q26;
    case AArch64::X27:
      return AArch64::Q27;
    case AArch64::X28:
      return AArch64::Q28;
		case AArch64::FP:
			return AArch64::Q29;
		case AArch64::LR:
			return AArch64::Q30;
	}
}

unsigned ZomTagUtils::getCorrespondingReg(const unsigned XReg)
{
  switch(XReg)
  {
    default:
      return XReg;
    case AArch64::X0:
      return AArch64::W0;
    case AArch64::X1:
      return AArch64::W1;
    case AArch64::X2:
      return AArch64::W2;
    case AArch64::X3:
      return AArch64::W3;
    case AArch64::X4:
      return AArch64::W4;
    case AArch64::X5:
      return AArch64::W5;
    case AArch64::X6:
      return AArch64::W6;
    case AArch64::X7:
      return AArch64::W7;
    case AArch64::X8:
      return AArch64::W8;
    case AArch64::X9:
      return AArch64::W9;
    case AArch64::X10:
      return AArch64::W10;
    case AArch64::X11:
      return AArch64::W11;
    case AArch64::X12:
      return AArch64::W12;
    case AArch64::X13:
      return AArch64::W13;
    case AArch64::X14:
      return AArch64::W14;
    case AArch64::X15:
      return AArch64::W15;
    case AArch64::X16:
      return AArch64::W16;
    case AArch64::X17:
      return AArch64::W17;
    case AArch64::X18:
      return AArch64::W18;
    case AArch64::X19:
      return AArch64::W19;
    case AArch64::X20:
      return AArch64::W20;
    case AArch64::X21:
      return AArch64::W21;
    case AArch64::X22:
      return AArch64::W22;
    case AArch64::X23:
      return AArch64::W23;
    case AArch64::X24:
      return AArch64::W24;
    case AArch64::X25:
      return AArch64::W25;
    case AArch64::X26:
      return AArch64::W26;
    case AArch64::X27:
      return AArch64::W27;
    case AArch64::X28:
      return AArch64::W28;
		case AArch64::FP:
			return AArch64::W29;
		case AArch64::LR:
			return AArch64::W30;
  }
}

unsigned ZomTagUtils::getCorrespondingLoad(const unsigned opCode)
{
  switch(opCode)
  {
    default:
      return 0;
    case AArch64::LDRSBWroX:
      return AArch64::LDRSBWroW;
    case AArch64::LDRSHWroX:
      return AArch64::LDRSHWroW;
    case AArch64::LDRWroX:
      return AArch64::LDRWroW;
    case AArch64::LDRXroX:
      return AArch64::LDRXroW;
    case AArch64::LDRSBXroX:
      return AArch64::LDRSBXroW;
    case AArch64::LDRSHXroX:
      return AArch64::LDRSHXroW;
    case AArch64::LDRSWroX:
      return AArch64::LDRSWroW;
    case AArch64::LDRBBroX:
      return AArch64::LDRBBroW;
    case AArch64::LDRHHroX:
      return AArch64::LDRHHroW;
    case AArch64::LDRSroX:
      return AArch64::LDRSroW;
    case AArch64::LDRDroX:
      return AArch64::LDRDroW;
  }
}

bool ZomTagUtils::isXReg(const unsigned reg)
{
  switch(reg)
  {
    default:
      return false;
    case AArch64::X0:
    case AArch64::X1:
    case AArch64::X2:
    case AArch64::X3:
    case AArch64::X4:
    case AArch64::X5:
    case AArch64::X6:
    case AArch64::X7:
    case AArch64::X8:
    case AArch64::X9:
    case AArch64::X10:
    case AArch64::X11:
    case AArch64::X12:
    case AArch64::X13:
    case AArch64::X14:
    case AArch64::X15:
    case AArch64::X16:
    case AArch64::X17:
    case AArch64::X18:
    case AArch64::X19:
    case AArch64::X20:
    case AArch64::X21:
    case AArch64::X22:
    case AArch64::X23:
    case AArch64::X24:
    case AArch64::X25:
    case AArch64::X26:
    case AArch64::X27:
    case AArch64::X28:
    case AArch64::FP:   // X29
    case AArch64::LR:   // X30
      return true;
  }
}

bool ZomTagUtils::isQReg(const unsigned reg)
{
  switch(reg)
  {
    default:
      return false;
    case AArch64::Q0:
    case AArch64::Q1:
    case AArch64::Q2:
    case AArch64::Q3:
    case AArch64::Q4:
    case AArch64::Q5:
    case AArch64::Q6:
    case AArch64::Q7:
    case AArch64::Q8:
    case AArch64::Q9:
    case AArch64::Q10:
    case AArch64::Q11:
    case AArch64::Q12:
    case AArch64::Q13:
    case AArch64::Q14:
    case AArch64::Q15:
    case AArch64::Q16:
    case AArch64::Q17:
    case AArch64::Q18:
    case AArch64::Q19:
    case AArch64::Q20:
    case AArch64::Q21:
    case AArch64::Q22:
    case AArch64::Q23:
    case AArch64::Q24:
    case AArch64::Q25:
    case AArch64::Q26:
    case AArch64::Q27:
    case AArch64::Q28:
    case AArch64::Q29:   // X29
    case AArch64::Q30:   // X30
		case AArch64::D0:
    case AArch64::D1:
    case AArch64::D2:
    case AArch64::D3:
    case AArch64::D4:
    case AArch64::D5:
    case AArch64::D6:
    case AArch64::D7:
    case AArch64::D8:
    case AArch64::D9:
    case AArch64::D10:
    case AArch64::D11:
    case AArch64::D12:
    case AArch64::D13:
    case AArch64::D14:
    case AArch64::D15:
    case AArch64::D16:
    case AArch64::D17:
    case AArch64::D18:
    case AArch64::D19:
    case AArch64::D20:
    case AArch64::D21:
    case AArch64::D22:
    case AArch64::D23:
    case AArch64::D24:
    case AArch64::D25:
    case AArch64::D26:
    case AArch64::D27:
    case AArch64::D28:
    case AArch64::D29:   // X29
    case AArch64::D30:   // X30
		case AArch64::S0:
    case AArch64::S1:
    case AArch64::S2:
    case AArch64::S3:
    case AArch64::S4:
    case AArch64::S5:
    case AArch64::S6:
    case AArch64::S7:
    case AArch64::S8:
    case AArch64::S9:
    case AArch64::S10:
    case AArch64::S11:
    case AArch64::S12:
    case AArch64::S13:
    case AArch64::S14:
    case AArch64::S15:
    case AArch64::S16:
    case AArch64::S17:
    case AArch64::S18:
    case AArch64::S19:
    case AArch64::S20:
    case AArch64::S21:
    case AArch64::S22:
    case AArch64::S23:
    case AArch64::S24:
    case AArch64::S25:
    case AArch64::S26:
    case AArch64::S27:
    case AArch64::S28:
    case AArch64::S29:   // X29
    case AArch64::S30:   // X30
		case AArch64::H0:
    case AArch64::H1:
    case AArch64::H2:
    case AArch64::H3:
    case AArch64::H4:
    case AArch64::H5:
    case AArch64::H6:
    case AArch64::H7:
    case AArch64::H8:
    case AArch64::H9:
    case AArch64::H10:
    case AArch64::H11:
    case AArch64::H12:
    case AArch64::H13:
    case AArch64::H14:
    case AArch64::H15:
    case AArch64::H16:
    case AArch64::H17:
    case AArch64::H18:
    case AArch64::H19:
    case AArch64::H20:
    case AArch64::H21:
    case AArch64::H22:
    case AArch64::H23:
    case AArch64::H24:
    case AArch64::H25:
    case AArch64::H26:
    case AArch64::H27:
    case AArch64::H28:
    case AArch64::H29:   // X29
    case AArch64::H30:   // X30
		case AArch64::B0:
    case AArch64::B1:
    case AArch64::B2:
    case AArch64::B3:
    case AArch64::B4:
    case AArch64::B5:
    case AArch64::B6:
    case AArch64::B7:
    case AArch64::B8:
    case AArch64::B9:
    case AArch64::B10:
    case AArch64::B11:
    case AArch64::B12:
    case AArch64::B13:
    case AArch64::B14:
    case AArch64::B15:
    case AArch64::B16:
    case AArch64::B17:
    case AArch64::B18:
    case AArch64::B19:
    case AArch64::B20:
    case AArch64::B21:
    case AArch64::B22:
    case AArch64::B23:
    case AArch64::B24:
    case AArch64::B25:
    case AArch64::B26:
    case AArch64::B27:
    case AArch64::B28:
    case AArch64::B29:   // X29
    case AArch64::B30:   // X30
      return true;
  }
}


bool ZomTagUtils::isLoadPair(const MachineInstr &MI)
{
	const auto opCode = MI.getOpcode();
	switch(opCode)
	{
		default:
			return false;
		case AArch64::LDPXi:
		case AArch64::LDPDi:
		case AArch64::LDPQi:
		case AArch64::LDNPQi:
		case AArch64::LDNPXi:
		case AArch64::LDNPDi:
		case AArch64::LDPWi:
		case AArch64::LDPSi:
		case AArch64::LDNPWi:
		case AArch64::LDNPSi:
			return true;
	}
}

bool ZomTagUtils::isStorePair(const MachineInstr &MI)
{
	const auto opCode = MI.getOpcode();
	switch(opCode)
	{
		default:
			return false;
		case AArch64::STPQi:
		case AArch64::STNPQi:
		case AArch64::STPXi:
		case AArch64::STPDi:
		case AArch64::STNPXi:
		case AArch64::STNPDi:
		case AArch64::STPWi:
		case AArch64::STPSi:
		case AArch64::STNPWi:
		case AArch64::STNPSi:
			return true;
	}
}

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
    case AArch64::LDRSBWui:
    case AArch64::LDRSHWui:
    case AArch64::LDRSBWroX:
    case AArch64::LDRSHWroX:
    case AArch64::LDRWroX:
    case AArch64::LDRXroX:
    case AArch64::LDRSBXroX:
    case AArch64::LDRSHXroX:
    case AArch64::LDRSWroX:
    case AArch64::LDRSBWroW:
    case AArch64::LDRSHWroW:
    case AArch64::LDRWroW:
    case AArch64::LDRXroW:
    case AArch64::LDRSBXroW:
    case AArch64::LDRSHXroW:
    case AArch64::LDRSWroW:
    case AArch64::LDRBBroX:
    case AArch64::LDRHHroX:
    case AArch64::LDRBBroW:
    case AArch64::LDRHHroW:
    case AArch64::LDRSroX:
    case AArch64::LDRDroX:
    case AArch64::LDRSroW:
    case AArch64::LDRDroW:
      return true; 
  }
}

bool ZomTagUtils::isStore(const MachineInstr &MI)
{
  const auto opCode = MI.getOpcode();
	switch(opCode)
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
 
