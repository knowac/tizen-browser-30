project(PlatformInputManager)

set(PlatformInputManager_SRCS
    PlatformInputManager.cpp
    )

include(Coreheaders)
include(EFLHelpers)

add_library(${PROJECT_NAME} SHARED ${PlatformInputManager_SRCS})

if(TIZEN_BUILD)
    target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS})
endif(TIZEN_BUILD)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)
