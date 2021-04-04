if (NOT LLVM_REQUIRES_RTTI)
  if (NOT LLVM_REQUIRES_EH)
    set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/GepMD.exports)
  endif()
endif()

add_llvm_loadable_module( LLVMGepMDPass
  GepMD.cpp

  DEPENDS
  intrinsics_gen
  PLUGIN_TOOL
  opt
  )
