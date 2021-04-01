add_llvm_loadable_module( LLVMTest
  Test.cpp

  DEPENDS
  intrinsics_gen
  PLUGIN_TOOL
  opt
  )
