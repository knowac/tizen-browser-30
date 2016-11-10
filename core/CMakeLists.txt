project(browserApp)

include(Coreheaders)
include(EFLHelpers)
include(EWebKitHelpers)

find_package(PNG REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${PNG_INCLUDE_DIRS}
    )

### browserCore library
### current platform CMake desn't support clean o build
set(browserCore_SRCS
    BasicUI/EAction.cpp
    BasicUI/Action.cpp
    BasicUI/MenuButton.cpp
    BasicUI/NaviframeWrapper.cpp
    Config/Config.cpp
    Logger/Logger.cpp
    Logger/LoggerTools.cpp
    Logger/TextLogger.cpp
    Logger/Useloggers.cpp
    ServiceManager/Debug/BrowserAssert.cpp
    ServiceManager/ServiceFactory.cpp
    ServiceManager/ServiceLib.cpp
    ServiceManager/ServiceLoader.cpp
    ServiceManager/ServiceManager.cpp
    AbstractWebEngine/TabId.cpp
    AbstractWebEngine/WebConfirmation.cpp
    Tools/EflTools.cpp
    Tools/BrowserImage.cpp
    Tools/Blob.cpp
    Tools/BookmarkItem.cpp
    Tools/BookmarkFolder.cpp
    Tools/QuickAccessItem.cpp
    Tools/CapiWebErrorCodes.cpp
    Tools/FeedItem.cpp
    Tools/FeedChannel.cpp
    Tools/StringTools.cpp
    )

if(${PROFILE} MATCHES "mobile")
    set(browserCore_SRCS
        ${browserCore_SRCS}
        DownloadControl/DownloadControl.cpp
        )
else(${PROFILE} MATCHES "mobile")
    set(browserCore_SRCS
        ${browserCore_SRCS}
        Tools/FocusManager.cpp
        )
endif(${PROFILE} MATCHES "mobile")

if(TIZEN_BUILD)
    set(browserCore_SRCS
        ${browserCore_SRCS}
        Logger/DLOGLogger.cpp
        )
endif(TIZEN_BUILD)

if(TIZEN_BUILD)
    set(${browserCore_SRCS} ${browserCore_SRCS}
    DLOGLogger.cpp)
endif(TIZEN_BUILD)

configure_file(Config/ConfigValues.h.in Config/ConfigValues.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/Config)

if(DYN_INT_LIBS)
    add_library(browserCore SHARED ${browserCore_SRCS})
else(DYN_INT_LIBS)
    add_library(browserCore STATIC ${browserCore_SRCS})
endif(DYN_INT_LIBS)

target_link_libraries(browserCore dl)
target_link_libraries(browserCore ${Boost_LIBRARIES})
target_link_libraries(browserCore ${EFL_LDFLAGS})
target_link_libraries(browserCore ${EWEBKIT2_LDFLAGS})

if(TIZEN_BUILD)
    target_link_libraries(browserCore ${tizen_pkgs_LDFLAGS})
endif(TIZEN_BUILD)

install(TARGETS browserCore
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
### end browserCore library


### browser binary
add_executable(${BIN_NAME} main.cpp)
add_dependencies(${BIN_NAME} browserCore)
target_link_libraries(${BIN_NAME} browserCore)
target_link_libraries(${BIN_NAME} ${EFL_LDFLAGS})
target_link_libraries(${BIN_NAME} ${Boost_LIBRARIES})


if(TIZEN_BUILD)
    target_link_libraries(${BIN_NAME} ${tizen_pkgs_LDFLAGS} "-pie"
    BookmarkFlowUI
    BookmarkManagerUI
    BookmarkService
    CertificateService
    FindOnPageUI
    HistoryService
    HistoryUI
    PlatformInputManager
    QuickAccess
    SettingsUI
    SimpleUI
    StorageService
    TabService
    TabUI
    WebEngineService
    WebPageUI
)
endif(TIZEN_BUILD)

install(TARGETS ${BIN_NAME} RUNTIME DESTINATION bin)



