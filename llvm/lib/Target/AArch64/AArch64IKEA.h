#ifndef LLVM_LIB_TARGET_AARCH64_IKEA_H
#define LLVM_LIB_TARGET_AARCH64_IKEA_H

#include "AArch64.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"

#define SIZE 100
//char *ikea_func_name[SIZE];
//uint64_t ikea_address[SIZE];
//uint64_t ikea_size[SIZE];

namespace llvm {
namespace IKEA {

class IkeaMetaData;

class IkeaMetaData {
private:
	char *ikea_func_name[SIZE];
	uint64_t ikea_address[SIZE];
	uint64_t ikea_size[SIZE];
public:
	void set_address(uint64_t value, int index) { ikea_address[index] = value; };
	uint64_t get_address(int index) { return ikea_address[index]; };

};

} // end namespace ikea
} // end namespace llvm

#endif // LLVM_LIB_TARGET_AARCH64_IKEA_H
