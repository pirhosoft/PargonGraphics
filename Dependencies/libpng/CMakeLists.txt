set(CMAKE_FOLDER Dependencies)

enable_language(C)
enable_language(ASM)

set(SOURCES
	Source/png.c
	Source/pngerror.c
	Source/pngget.c
	Source/pngmem.c
	Source/pngpread.c
	Source/pngread.c
	Source/pngrio.c
	Source/pngrtran.c
	Source/pngrutil.c
	Source/pngset.c
	Source/pngtest.c
	Source/pngtrans.c
	Source/pngwio.c
	Source/pngwrite.c
	Source/pngwtran.c
	Source/pngwutil.c
	Source/arm/arm_init.c
	Source/arm/filter_neon.S
	Source/arm/filter_neon_intrinsics.c
)

add_library(libpng STATIC)
target_include_directories(libpng PUBLIC Source)
target_include_directories(libpng PUBLIC ../zlib/Source)
target_sources(libpng PRIVATE "${SOURCES}")
set_target_properties(libpng PROPERTIES LINKER_LANGUAGE C)
