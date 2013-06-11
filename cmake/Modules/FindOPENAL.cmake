# -*- cmake -*-

set(OPENAL ON CACHE BOOL "Enable OpenAL")

if (OPENAL)
	include(FindPkgConfig)
	include(FindOpenAL)
	pkg_check_modules(OPENAL_LIB REQUIRED openal)
	pkg_check_modules(FREEALUT_LIB REQUIRED freealut)
	set(OPENAL_LIBRARIES
		openal
		alut
		)
endif (OPENAL)

if (OPENAL)
	message(STATUS "Building with OpenAL audio support")
endif (OPENAL)
