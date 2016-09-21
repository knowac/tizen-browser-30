project(FindOnPageUI)

set(FindOnPageUI_SRCS
    FindOnPageUI.cpp
    )

set(FindOnPageUI_HEADERS
    FindOnPageUI.h
    )

include(Coreheaders)
include(EFLHelpers)
include(EWebKitHelpers)

add_library(${PROJECT_NAME} SHARED ${FindOnPageUI_SRCS})

if(TIZEN_BUILD)
    target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS})
endif(TIZEN_BUILD)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)

include(EDCCompile)
EDJ_TARGET(FindOnPage.edj
    ${CMAKE_CURRENT_SOURCE_DIR}/edc/FindOnPage.edc
    ${CMAKE_CURRENT_BINARY_DIR})
