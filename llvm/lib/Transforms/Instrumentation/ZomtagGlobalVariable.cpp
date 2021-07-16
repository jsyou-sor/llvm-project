#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/SpecialCaseList.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Transforms/Instrumentation.h"

extern "C"
{
//#include "stack.h"
}

using namespace llvm;
using namespace std;
//int total_GV;
#define DEBUG_TYPE "ZomtagGlobalVariablePass"

static bool isInterestingGlobal(GlobalVariable *GV);
static void makeGlobalVariableZomtag(Module *m, GlobalVariable *GV);

namespace
{
	struct ZomtagGlobalVariable : public ModulePass
	{
		public:
			static char ID;
			ZomtagGlobalVariable() : ModulePass(ID)
			{
				initializeZomtagGlobalVariablePass(*PassRegistry::getPassRegistry());
			}

			bool runOnModule(Module &M) override;
			bool isInterestingGlobal(GlobalVariable *GV);
			void makeGlobalVariableZomtag(Module *M, GlobalVariable *GV);
			int total_GV = 0;
			size_t region = 0;
	};
}

bool ZomtagGlobalVariable::isInterestingGlobal(GlobalVariable *GV)
{
	if (GV->hasSection())
		return false;
	if (GV->getAlignment() > 16)
	{
		errs() << "!!!\n";
		return false;
	}
	if (GV->isThreadLocal())
		return false;
	switch (GV->getLinkage())
	{
		case GlobalValue::ExternalLinkage:
		case GlobalValue::InternalLinkage:
		case GlobalValue::PrivateLinkage:
		case GlobalValue::WeakAnyLinkage:
		case GlobalValue::WeakODRLinkage:
		case GlobalValue::CommonLinkage:
			break;
		default:
			return false;
	}
	total_GV++;
	GV->print(errs());
	errs() << "\n";
	return true;
}

void ZomtagGlobalVariable::makeGlobalVariableZomtag(Module *M, GlobalVariable *GV)
{
	if (GV->isDeclaration())
		return;
	if (!isInterestingGlobal(GV))
		return;

	if (GV->hasCommonLinkage())
		GV->setLinkage(llvm::GlobalValue::WeakAnyLinkage);

	const DataLayout *DL = &M->getDataLayout();
	Type *Ty = GV->getType();
	PointerType *PtrTy = dyn_cast<PointerType>(Ty);
	assert(PtrTy != nullptr);
	Ty = PtrTy->getElementType();
	size_t size = DL->getTypeAllocSize(Ty);

	string section("zomtag_section_test_");
	//if (GV->isConstant())
		//section += "const_";
	//section += "test";
	region = total_GV / 16;
	errs() << region << "\n";
	section += to_string(region);		

	GV->setSection(section);
}

bool ZomtagGlobalVariable::runOnModule(Module &M)
{
/*
	for (auto &GV : M.getGlobalList())
		makeGlobalVariableZomtag(&M, &GV);
	errs() << "total_GV: " << total_GV << "\n";
*/
	return false;
}

char ZomtagGlobalVariable::ID = 0;
INITIALIZE_PASS(ZomtagGlobalVariable, "zomtag-global-variable-pass", "zomtag global var. pass", false, false)

//namespace llvm
//{
	ModulePass *llvm::createZomtagGlobalVariablePass()
	{
		return new ZomtagGlobalVariable();
	}
//}
