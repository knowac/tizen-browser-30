project(TabUI)

set(TabUI_SRCS
    TabUI.cpp
    PasswordUI.cpp
    )

include(Coreheaders)
include(EFLHelpers)

include_directories(${CMAKE_SOURCE_DIR}/services/FavoriteService)

add_library(${PROJECT_NAME} SHARED ${TabUI_SRCS})

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
        TabUI_mob.edc
        PasswordUI.edc
        )
else (${PROFILE} MATCHES "mobile")  # tv profile
    set(edcFiles
        TabUI.edc
        )
endif (${PROFILE} MATCHES "mobile")

foreach(edec ${edcFiles})
    string(REGEX REPLACE "(_mob)?.edc" ".edj" target_name ${edec})
    EDJ_TARGET(${target_name}
        ${CMAKE_CURRENT_SOURCE_DIR}/edc/${edec}
        ${CMAKE_CURRENT_BINARY_DIR})
endforeach(edec)

#EDJ_TARGET(TabUI.edj
#    ${CMAKE_CURRENT_SOURCE_DIR}/edc/TabUI.edc
#    ${CMAKE_CURRENT_BINARY_DIR})
