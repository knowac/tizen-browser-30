project(HistoryUI)

set(HistoryUI_SRCS
    HistoryUI.cpp
    HistoryUIFocusManager.cpp
    HistoryPeriod.cpp
    HistoryDaysListManager/HistoryDaysListManagerMob.cpp
    HistoryDaysListManager/mob/HistoryDayItemMob.cpp
    HistoryDaysListManager/mob/WebsiteHistoryItem/WebsiteHistoryItemMob.cpp
    HistoryDaysListManager/mob/WebsiteHistoryItem/WebsiteHistoryItemTitleMob.cpp
    HistoryDaysListManager/mob/WebsiteHistoryItem/WebsiteHistoryItemVisitItemsMob.cpp
    )

include(Coreheaders)
include(EFLHelpers)

include_directories(${CMAKE_SOURCE_DIR}/services/FavoriteService)

add_library(${PROJECT_NAME} SHARED ${HistoryUI_SRCS})

if(TIZEN_BUILD)
    target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS})
endif(TIZEN_BUILD)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)

include(EDCCompile)
set(edcFiles
    colorClasses.edc
    History_mob.edc
    HistoryDaysList_mob.edc
    WebsiteHistoryItem_mob.edc
    WebsiteHistoryItemTitle_mob.edc
    WebsiteHistoryItemVisitItems_mob.edc
)

foreach(edec ${edcFiles})
    string(REGEX REPLACE "(_mob)?.edc" ".edj" target_name ${edec})
    EDJ_TARGET(${target_name}
        ${CMAKE_CURRENT_SOURCE_DIR}/edc/${edec}
        ${CMAKE_CURRENT_BINARY_DIR})
endforeach(edec)
