project(BookmarkDetailsUI)

set(BookmarkDetailsUI_SRCS
    BookmarkDetailsUI.cpp
    )

include(Coreheaders)
include(EFLHelpers)

include_directories(${CMAKE_SOURCE_DIR}/services/FavoriteService)

add_library(${PROJECT_NAME} SHARED ${BookmarkDetailsUI_SRCS})

if(TIZEN_BUILD)
    target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS})
endif(TIZEN_BUILD)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)

#please do not add edc/ directory
if (${PROFILE} MATCHES "mobile")    # mobile profile
set(edcFiles
    BookmarkDetailsUI_mob.edc
    )
else (${PROFILE} MATCHES "mobile")  # tv profile
set(edcFiles
    BookmarkDetailsUI.edc
    )
endif (${PROFILE} MATCHES "mobile")

foreach(edec ${edcFiles})
    string(REGEX REPLACE "(_mob)?.edc" ".edj" target_name ${edec})
    EDJ_TARGET(${target_name}
               ${CMAKE_CURRENT_SOURCE_DIR}/edc/${edec}
               ${CMAKE_CURRENT_BINARY_DIR})
endforeach(edec)

