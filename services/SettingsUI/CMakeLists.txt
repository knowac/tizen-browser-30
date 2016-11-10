project(SettingsUI)

set(SettingsUI_SRCS
    SettingsManager.cpp
    SettingsUI.cpp
    SettingsMain.cpp
    SettingsHomePage.cpp
    SettingsPrivacy.cpp
    SettingsAdvanced.cpp
    SettingsDelPersData.cpp
    SettingsAFProfile.cpp
    SettingsAFCreator.cpp
    SettingsUserAgent.cpp
    AutoFillForm/AutoFillFormItem.cpp
    )
set(SettingsUI_HEADERS
    SettingsManager.h
    SettingsUI.h
    SettingsMain.h
    SettingsHomePage.h
    SettingsPrivacy.h
    SettingsAdvanced.h
    SettingsDelPersData.h
    SettingsAFProfile.h
    SettingsAFCreator.h
    SettingsUserAgent.h
    SettingsPrettySignalConnector.h
    AutoFillForm/AutoFillFormItem.h
    )

include(Coreheaders)
include(EFLHelpers)
include(EWebKitHelpers)

add_library(${PROJECT_NAME} SHARED ${SettingsUI_SRCS})

if(TIZEN_BUILD)
    target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS})
endif(TIZEN_BUILD)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)

include(EDCCompile)

EDJ_TARGET(SettingsMobileUI.edj
    ${CMAKE_CURRENT_SOURCE_DIR}/edc/Settings_mob.edc
    ${CMAKE_CURRENT_BINARY_DIR})
