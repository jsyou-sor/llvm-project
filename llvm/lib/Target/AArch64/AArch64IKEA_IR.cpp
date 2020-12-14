#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

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

using namespace llvm;

#define DEBUG_TYPE "hello"

STATISTIC(HelloCounter, "Counts number of functions greeted");

namespace llvm {
	void initializeAArch64IKEA_IRPass(PassRegistry &);
}

#define PASS_NAME "AArch64 IKEA IR Pass"

namespace {
	struct AArch64IKEA_IR : public FunctionPass {
		static char ID;
		AArch64IKEA_IR() : FunctionPass(ID) {
			initializeAArch64IKEA_IRPass(*PassRegistry::getPassRegistry());
		}

		/*
		bool runOnFunction(Function &F) override {
			++HelloCounter;
			errs() << "Hello: ";
			errs().write_escaped(F.getName()) << "\n";
			return false;
		}
		*/

		bool runOnFunction(Function &F) override;
		StringRef getPassName() const override {
			return PASS_NAME;
		}
	};
	char AArch64IKEA_IR::ID = 0;
}

INITIALIZE_PASS(AArch64IKEA_IR, "aarch64-ikea-ir-pass", PASS_NAME, false, false)

FunctionPass *llvm::createAArch64IKEA_IRPass() {
	dbgs() << "[+] AArch64IKEA_IR_Pass\n";
	return new AArch64IKEA_IR();
}

bool AArch64IKEA_IR::runOnFunction(Function &F) {
	dbgs() << "[IR] " << F.getName() << "\n";
}
