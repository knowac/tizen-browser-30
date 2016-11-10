project(BookmarkFlowUI)

set(BookmarkFlowUI_SRCS
    BookmarkFlowUI.cpp
    )

include(Coreheaders)
include(EFLHelpers)

add_library(${PROJECT_NAME} SHARED ${BookmarkFlowUI_SRCS})

if(TIZEN_BUILD)
    target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS})
endif(TIZEN_BUILD)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)

include(EDCCompile)
#please do not add edc/ directory
if (${PROFILE} MATCHES "mobile")    # mobile profile
    set(edcFiles
        BookmarkFlowUI_mob.edc
        )
else (${PROFILE} MATCHES "mobile")  # tv profile
    set(edcFiles
        BookmarkFlowUI.edc
        )
endif (${PROFILE} MATCHES "mobile")

foreach(edec ${edcFiles})
    string(REGEX REPLACE "(_mob)?.edc" ".edj" target_name ${edec})
    EDJ_TARGET(${target_name}
        ${CMAKE_CURRENT_SOURCE_DIR}/edc/${edec}
        ${CMAKE_CURRENT_BINARY_DIR})
endforeach(edec)

