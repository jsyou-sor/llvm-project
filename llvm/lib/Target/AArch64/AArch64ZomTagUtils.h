#ifndef LLVM_ZOMTAGUTILS_H
#define LLVM_ZOMTAGUTILS_H

#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "AArch64.h"
#include "AArch64RegisterInfo.h"
#include "AArch64InstrInfo.h"

namespace llvm 
{
  class ZomTagUtils;
  typedef std::shared_ptr<ZomTagUtils> ZomTagUtils_ptr;
  
  class ZomTagUtils {
  
    const TargetInstrInfo *TII;
    const TargetRegisterInfo *TRI;
    ZomTagUtils() = delete;
  
  public:
    ZomTagUtils(const TargetRegisterInfo *TRI, const TargetInstrInfo *TII);
    static inline ZomTagUtils_ptr get(const TargetRegisterInfo *TRI,
      const TargetInstrInfo *TII)
    {
      return std::make_shared<ZomTagUtils>(TRI, TII);
    }

		bool isAddSub(MachineInstr &MI);
    bool isXReg(const unsigned reg);
    bool isLoad(const MachineInstr &MI);
    bool isStore(const MachineInstr &MI);
		unsigned getCorrespondingLoad(const unsigned opCode);
    unsigned getCorrespondingStore(const unsigned opCode);
    unsigned getCorrespondingLoadStore(const unsigned opCode);
		unsigned getCorrespondingReg(const unsigned XReg);
    bool isInterestingLoad(const MachineInstr &MI);
    bool isInterestingStore(const MachineInstr &MI);
    bool isRegisterOffsetLoad(const MachineInstr &MI);
		bool isLoadPair(const MachineInstr &MI);
		bool isStorePair(const MachineInstr &MI);
		bool isPrePostIndexed(const MachineInstr &MI);
		unsigned getQReg(const unsigned XReg);
		bool isQReg(const unsigned reg);
		bool isLDRD(const MachineInstr &MI);
  };
}

#endif /* LLVM_ZOMTAGUTILS_H */
