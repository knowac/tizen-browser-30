project(BrowserApp)
cmake_minimum_required(VERSION 2.8)
#
########## Defines ###########
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res" CACHE STRING "The path of resources")
SET(EDJE_DIR "${RESDIR}/edje/")
SET(CERTS_DIR "${RESDIR}/certs/")
SET(LOCALE_DIR "${RESDIR}/locale/")
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
SET(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
set(BIN_NAME "browser")

if(NOT DEFINED PACKAGE_NAME)
    SET(PACKAGE_NAME "org.tizen.browser")
endif(NOT DEFINED PACKAGE_NAME)

if(NOT DEFINED MANIFESTDIR)
    SET(MANIFESTDIR ${TZ_SYS_RO_PACKAGES})
endif(NOT DEFINED MANIFESTDIR)

if(NOT DEFINED ICONDIR)
    SET(ICONDIR "${TZ_SYS_RO_ICONS}/default/small/")
endif(NOT DEFINED ICONDIR)

option(BUILD_UT "Build Unit tests" OFF)
option(TIZEN_BUILD "Build for Tizen. Default ON" ON)
option(DYN_INT_LIBS "Buils internal libs as dynamic" ON)
option(COVERAGE_STATS "Code coverage" OFF)

#Enable C++14 support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)


if(COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O0 -g -ggdb3")
elseif(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 -g -ggdb3")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -O0 -g -ggdb3")
else()
    message(FATAL_ERROR "The Compiler ${CMAKE_CXX_COMPILER} doesn't support C++14 nor C++11")
endif()

set(BOOST_LIBS regex filesystem system date_time)
if(BUILD_UT)
    set(BOOST_LIBS ${BOOST_LIBS} unit_test_framework)
ENDIF(BUILD_UT)
find_package(Boost 1.49.9 REQUIRED COMPONENTS ${BOOST_LIBS})

#RPATH settings
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:${CMAKE_INSTALL_PREFIX}/services")

#disable for merge - WebView is not ready for this
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIE -Wextra -Wno-variadic-macros -Wno-long-long -Werror")

if(COVERAGE_STATS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
endif(COVERAGE_STATS)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# TIZEN SPECIFIC
# set pkg-config modules
if(TIZEN_BUILD)
    include(FindPkgConfig)

    set(PLATFORM_TIZEN ON)
    SET(tizen_pkgs_list
        dlog
        capi-appfw-application
        capi-media-image-util
        efl-extension
        libssl
    )

   if (${PROFILE} MATCHES "mobile")
    SET(tizen_pkgs_list
        ${tizen_pkgs_list}
        capi-system-device
        capi-system-system-settings
        capi-content-media-content
        capi-appfw-app-manager
        notification
        appsvc
    )
    endif (${PROFILE} MATCHES "mobile")

    pkg_check_modules(tizen_pkgs REQUIRED "${tizen_pkgs_list}")

    FOREACH(flag ${tizen_pkgs_CFLAGS})
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    ENDFOREACH(flag)

    message("${CMAKE_CXX_FLAGS}")
    MACRO (TODAY RESULT)
        EXECUTE_PROCESS(COMMAND "date" "+%d-%m-%Y" OUTPUT_VARIABLE ${RESULT})
        STRING(REGEX REPLACE "(..)-(..)-(....).*" "\\1-\\2-\\3" ${RESULT} ${${RESULT}})
    ENDMACRO (TODAY)

    TODAY(BUILD_DATE)

    ADD_DEFINITIONS(
        -DBUILD_DATE=\"${BUILD_DATE}\"
        -DVERSION=\"${VERSION}\"
        -D__BUILD__
        -DM_VCS_ID=\"${VCS_ID}\"
        -DTIZEN_BUILD=\"${TIZEN_BUILD}\"
        -DCERTS_DIR=\"${CERTS_DIR}\"
        )

    if (${PROFILE} MATCHES "mobile")
        ADD_DEFINITIONS(-DPROFILE_MOBILE=1)
        ADD_DEFINITIONS(-DDUMMY_BUTTON=1)
    endif (${PROFILE} MATCHES "mobile")

endif(TIZEN_BUILD)

ADD_DEFINITIONS(-DEDJE_DIR=\"${EDJE_DIR}\")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#resources
add_subdirectory(res)

#services
add_subdirectory(services)

#browser core
add_subdirectory(core)

#compiling EDC files
#
#Temporary location of EDC files
#
# probably should be moved to Tizen section.
#to build unit tests add --define "build_unit_test ON" parameter to gbs build command
if(BUILD_UT)
  add_subdirectory(unit_tests)
ENDIF(BUILD_UT)

configure_file(${CMAKE_SOURCE_DIR}/cmakeconfig.h.cmake ${CMAKE_BINARY_DIR}/cmakeconfig.h)

if(TIZEN_BUILD)
if (${PROFILE} MATCHES "mobile")
configure_file(${CMAKE_SOURCE_DIR}/manifest.xml.in.mb ${CMAKE_BINARY_DIR}/${PACKAGE_NAME}.xml)
else (${PROFILE} MATCHES "mobile")
configure_file(${CMAKE_SOURCE_DIR}/manifest.xml.in ${CMAKE_BINARY_DIR}/${PACKAGE_NAME}.xml)
endif (${PROFILE} MATCHES "mobile")

install(FILES ${CMAKE_BINARY_DIR}/${PACKAGE_NAME}.xml DESTINATION ${MANIFESTDIR})
# install desktop file & icon
install(FILES ${CMAKE_SOURCE_DIR}/org.tizen.browser.png DESTINATION ${ICONDIR})
# install certs
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/certs/ DESTINATION ${CERTS_DIR})
endif(TIZEN_BUILD)

message(STATUS "------------------------------------------")
message(STATUS "Build for Tizen          (TIZEN_BUILD)     :" ${TIZEN_BUILD})
message(STATUS "Interal libs are dynamic (DYN_INT_LIBS)    :" ${DYN_INT_LIBS})
message(STATUS "Building Unit tests      (BUILD_UT)        :" ${BUILD_UT})
message(STATUS "Code Coverage statistics (COVERAGE_STATS)  :" ${COVERAGE_STATS})
message(STATUS "Device profile           (PROFILE)         :" ${PROFILE})
message(STATUS "------------------------------------------")
