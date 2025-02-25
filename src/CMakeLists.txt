set(LLVM_LINK_COMPONENTS
	${LLVM_TARGETS_TO_BUILD}
	Core
	Support
	Irreader
)

include(Utils)
include(AddLLVM)

add_subdirectory(frontend)
add_subdirectory(langspec)
add_subdirectory(backend)

