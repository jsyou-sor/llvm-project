add_llvm_loadable_module( libZomTag
  ZomTag.cpp

  DEPENDS
  intrinsics_gen
  PLUGIN_TOOL
  opt
  )
