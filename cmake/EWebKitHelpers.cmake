#FIXME: Do not use CMAKE_CXX_FLAGS

include(FindPkgConfig)

SET(ewebkit2_pkgs_list
#    ewebkit2
    chromium-efl
    )

pkg_check_modules(ewebkit2_pkgs QUIET ${ewebkit2_pkgs_list})

IF (${ewebkit2_pkgs_FOUND})

include_directories(${ewebkit2_pkgs_INCLUDE_DIRS})

FOREACH(flag ${ewebkit2_pkgs_CFLAGS})
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

set(EWEBKIT2_LDFLAGS ${ewebkit2_pkgs_LDFLAGS})
add_definitions(-DUSE_EWEBKIT)

ELSE()

message(WARNING "EWebKit not found")

ENDIF()
