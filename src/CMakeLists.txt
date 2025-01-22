set(LLVM_LINK_COMPONENTS
	${LLVM_TARGETS_TO_BUILD}
	Core
	Support
	Irreader
)

include(Utils)
include(AddLLVM)

add_subdirectory(front)
add_subdirectory(semantix)
add_subdirectory(main)

