set(CMAKE_FOLDER Dependencies)

enable_language(C)

set(SOURCES
	Source/adler32.c
	Source/compress.c
	Source/crc32.c
	Source/deflate.c
	Source/gzclose.c
	Source/gzlib.c
	Source/gzread.c
	Source/gzwrite.c
	Source/infback.c
	Source/inffast.c
	Source/inflate.c
	Source/inftrees.c
	Source/trees.c
	Source/uncompr.c
	Source/zutil.c
)

add_library(zlib STATIC)
target_include_directories(zlib PUBLIC Source)
target_sources(zlib PRIVATE "${SOURCES}")
set_target_properties(zlib PROPERTIES LINKER_LANGUAGE C)
