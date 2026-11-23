# cpp-httplib, shared by the game and server/.
include(FetchContent)

set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF)
set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF)
set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF)
set(HTTPLIB_USE_ZSTD_IF_AVAILABLE OFF)
set(HTTPLIB_INSTALL OFF)

FetchContent_Declare(
	httplib
	GIT_REPOSITORY https://github.com/yhirose/cpp-httplib
	GIT_TAG v0.67.0
	GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(httplib)
