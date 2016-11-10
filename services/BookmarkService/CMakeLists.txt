project(BookmarkService)

set(BookmarkService_SRCS
    BookmarkService.cpp
    )

include(Coreheaders)
include(EFLHelpers)
include(FindPkgConfig)

include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/services/StorageService)
include_directories(${CMAKE_SOURCE_DIR}/services/StorageService/Storage/include)
include_directories(${CMAKE_SOURCE_DIR}/core/AbstractInterfaces)

add_library(${PROJECT_NAME} SHARED ${BookmarkService_SRCS})

if(TIZEN_BUILD)
    target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS})
endif(TIZEN_BUILD)

SET(bp_pkgs_list
        capi-web-bookmark
        capi-web-history
        capi-web-tab
    )

pkg_check_modules(browser-provider REQUIRED "${bp_pkgs_list}")

#pkg_check_modules(browser-provider REQUIRED "capi-web-bookmark")
include_directories(${browser-provider_INCLUDE_DIRS})

add_dependencies(${PROJECT_NAME} StorageService)
target_link_libraries(${PROJECT_NAME} StorageService)
target_link_libraries(${PROJECT_NAME} ${browser-provider_LIBRARIES})

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)

install(FILES ${IMAGES} DESTINATION res)
