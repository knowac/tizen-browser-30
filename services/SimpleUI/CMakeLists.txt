project(SimpleUI)

set(SimpleUI_SRCS
    SimpleUI.cpp
    InputPopup.cpp
    SimplePopup.cpp
    NotificationPopup.cpp
    ViewManager.cpp
    RadioPopup.cpp
    ProgressiveWebApp.cpp
    )

if (${PROFILE} MATCHES "mobile")
set(SimpleUI_SRCS ${SimpleUI_SRCS}
    TextPopup_mob.cpp
    ContentPopup_mob.cpp
    )
endif (${PROFILE} MATCHES "mobile")

include(Coreheaders)
include(EFLHelpers)
include(EWebKitHelpers)

include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/services)
include_directories(${CMAKE_SOURCE_DIR}/services/WebPageUI)
include_directories(${CMAKE_SOURCE_DIR}/services/WebEngineService)
include_directories(${CMAKE_SOURCE_DIR}/services/WebEngineService/src)
include_directories(${CMAKE_SOURCE_DIR}/services/StorageService)
include_directories(${CMAKE_SOURCE_DIR}/services/BookmarkFlowUI)
include_directories(${CMAKE_SOURCE_DIR}/services/BookmarkManagerUI)
include_directories(${CMAKE_SOURCE_DIR}/services/FavoriteService)
include_directories(${CMAKE_SOURCE_DIR}/services/HistoryService)
include_directories(${CMAKE_SOURCE_DIR}/services/TabService)
include_directories(${CMAKE_SOURCE_DIR}/services/HistoryUI)
include_directories(${CMAKE_SOURCE_DIR}/services/QuickAccess)
include_directories(${CMAKE_SOURCE_DIR}/services/SettingsUI)
include_directories(${CMAKE_SOURCE_DIR}/services/TabUI)
include_directories(${CMAKE_SOURCE_DIR}/services/PlatformInputManager)
include_directories(${CMAKE_SOURCE_DIR}/services/CertificateService)
include_directories(${CMAKE_SOURCE_DIR}/services/FindOnPageUI)

include(FindPkgConfig)

pkg_check_modules(capi_net QUIET capi-network-connection)

add_library(${PROJECT_NAME} SHARED ${SimpleUI_SRCS})

add_dependencies(${PROJECT_NAME} WebPageUI)
add_dependencies(${PROJECT_NAME} StorageService)
add_dependencies(${PROJECT_NAME} HistoryService)
add_dependencies(${PROJECT_NAME} TabService)
add_dependencies(${PROJECT_NAME} BookmarkFlowUI)
add_dependencies(${PROJECT_NAME} BookmarkManagerUI)
add_dependencies(${PROJECT_NAME} QuickAccess)
add_dependencies(${PROJECT_NAME} HistoryUI)
add_dependencies(${PROJECT_NAME} TabUI)
add_dependencies(${PROJECT_NAME} SettingsUI)
add_dependencies(${PROJECT_NAME} PlatformInputManager)
add_dependencies(${PROJECT_NAME} CertificateService)
add_dependencies(${PROJECT_NAME} FindOnPageUI)

target_link_libraries(${PROJECT_NAME} WebPageUI)
target_link_libraries(${PROJECT_NAME} StorageService)
target_link_libraries(${PROJECT_NAME} HistoryService)
target_link_libraries(${PROJECT_NAME} TabService)
target_link_libraries(${PROJECT_NAME} QuickAccess)
target_link_libraries(${PROJECT_NAME} HistoryUI)
target_link_libraries(${PROJECT_NAME} TabUI)
target_link_libraries(${PROJECT_NAME} SettingsUI)
target_link_libraries(${PROJECT_NAME} BookmarkFlowUI)
target_link_libraries(${PROJECT_NAME} BookmarkManagerUI)
target_link_libraries(${PROJECT_NAME} PlatformInputManager)
target_link_libraries(${PROJECT_NAME} CertificateService)
target_link_libraries(${PROJECT_NAME} FindOnPageUI)
target_link_libraries(${PROJECT_NAME} ${EFL_LDFLAGS})

IF (${capi_net_FOUND})
    target_link_libraries(${PROJECT_NAME} ${capi_net_LDFLAGS})
ENDIF (${capi_net_FOUND})

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)

include(EDCCompile)

#please do not add edc/ directory
if (${PROFILE} MATCHES "mobile")
set(edcFiles
    InputPopup_mob.edc
    NotificationPopup_mob.edc
    Tooltip.edc
    ViewManager.edc
    TextPopup_mob.edc
    ContentPopup_mob.edc
    )
else (${PROFILE} MATCHES "mobile")
set(edcFiles
    InputPopup.edc
    NotificationPopup.edc
    Tooltip.edc
    ViewManager.edc
    )
endif (${PROFILE} MATCHES "mobile")

foreach(edec ${edcFiles})
    string(REGEX REPLACE "(_mob)?.edc" ".edj" target_name ${edec})
    EDJ_TARGET(${target_name}
               ${CMAKE_CURRENT_SOURCE_DIR}/edc/${edec}
               ${CMAKE_CURRENT_BINARY_DIR})
endforeach(edec)
