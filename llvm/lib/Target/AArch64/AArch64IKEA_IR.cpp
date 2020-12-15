#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Value.h"

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
#include "AArch64IKEA.h"

using namespace llvm;
using namespace llvm::IKEA;

//#define DEBUG_TYPE "hello"
//STATISTIC(HelloCounter, "Counts number of functions greeted");

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

	IkeaMetaData fe;	
	fe.set_address(1, 0);
	dbgs() << "[H] " << fe.get_address(0) << "\n";

	dbgs() << "[IR] " << F.getName() << "\n";

	if (F.getName() == "dummy_xmit") {
		//F.dump();
		for (auto &BBI:F.getBasicBlockList()) {
			BasicBlock *BB = &BBI;
			for (auto &InstI:BB->getInstList()) {
				Instruction *insn = &InstI;
				//dbgs() << *insn << "\n";
				insn->dump();
				//Value *v = dyn_cast<Value>(insn);
				//dbgs() << *v << "\n";
				for (auto op = insn->op_begin(); op != insn->op_end(); ++op) {
					dbgs() << "[-] " << *(op->get()) << "\n";
				}
				if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(insn)) {
					//dbgs() << *GEP << "\n";
					//dbgs() << *GEP->getPointerOperandType() << "\n";
					for (auto arg = F.arg_begin(); arg != F.arg_end(); ++arg) {
						Value *val = dyn_cast<Value>(arg);
						if (val->getType() == GEP->getPointerOperandType()) {
							dbgs() << "[+] " << *val->getType() << "\n";
						}
					}
					for (auto op = GEP->idx_begin(); op != GEP->idx_end(); ++op) {}
				}
			}
		}
	}
	
	//F.dump();
	/*
	for (auto arg = F.arg_begin(); arg != F.arg_end(); ++arg) {
		//arg.dump();
		dbgs() << *arg << "\n";
		Value *val = dyn_cast<Value>(arg);
		dbgs() << *val << "\n";
		dbgs() << *val->getType() << "\n";
		//dbgs() << arg->getDereferenceableBytes() << "\n";
		//dbgs() << arg->getAttribute() << "\n";
		//dbgs() << *arg << arg->hasByValAttr() << arg->hasByRefAttr() << "\n";
		//dbgs() << *arg->getParamByValType() << "\n";
		//dbgs() << *arg->getParamStructRetType() << "\n";
		//dbgs() << *arg->getParamByRefType() << "\n";
	}
	*/
}
