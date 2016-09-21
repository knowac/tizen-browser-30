project(browser-ut)
cmake_minimum_required(VERSION 2.8)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

include(Coreheaders)
include(EFLHelpers)
include(EWebKitHelpers)
include_directories( ${CMAKE_SOURCE_DIR})
include_directories( ${CMAKE_SOURCE_DIR}/services/SimpleUI)
include_directories( ${CMAKE_SOURCE_DIR}/services/BookmarkService)
include_directories( ${CMAKE_SOURCE_DIR}/services/HistoryService)
include_directories( ${CMAKE_SOURCE_DIR}/services/WebPageUI)
include_directories( ${CMAKE_SOURCE_DIR}/services/WebEngineService)
include_directories( ${CMAKE_SOURCE_DIR}/services/StorageService)
include_directories( ${CMAKE_SOURCE_DIR}/services/StorageService/Storage/include)
include_directories( ${CMAKE_SOURCE_DIR}/services/StorageService/src)
include_directories( ${CMAKE_SOURCE_DIR}/services/HistoryService)
include_directories( ${CMAKE_SOURCE_DIR}/services/PlatformInputManager)

set(UNIT_TESTS_SRCS
#    ut_HomeGenerator.cpp
    ut_Action.cpp
    ut_Config.cpp
    ut_logger.cpp
    ut_main.cpp
    ut_FavoriteService.cpp
    ut_BookmarkItem.cpp
    ut_PlatformInputManager.cpp
    ut_StorageService.cpp
    ut_coreService.cpp
    ut_SessionStorage.cpp
#    ut_WebEngineService.cpp
    )

if(TIZEN_BUILD)
    set(UNIT_TESTS_SRCS ${UNIT_TESTS_SRCS} ut_FavoriteService.cpp)
    set(UNIT_TESTS_SRCS ${UNIT_TESTS_SRCS} ut_StorageService.cpp)
endif(TIZEN_BUILD)

ADD_EXECUTABLE(${PROJECT_NAME} ${UNIT_TESTS_SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} browserCore)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
#add_dependencies(${PROJECT_NAME} HomeGenerator)
#target_link_libraries(${PROJECT_NAME} HomeGenerator)

if(TIZEN_BUILD)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME}
#        BookmarksUI
        BookmarkService
        HistoryService
        PlatformInputManager
        SimpleUI
        WebPageUI
        StorageService
        WebEngineService
        SessionStorage
    )
endif(TIZEN_BUILD)

# TODO Below line commented because of many warrings in code. In the future this flags should be enabled
#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fpie -Wall -Werror")
INSTALL(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

