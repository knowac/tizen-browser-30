PROJECT(HistoryService)

set(HistoryService_SOURCES
	HistoryService.cpp
	HistoryItem.cpp
	HistoryServiceTools.cpp
)

include(Coreheaders)
include(EFLHelpers)

include_directories(${CMAKE_SOURCE_DIR}/services/StorageService)
include_directories(${CMAKE_SOURCE_DIR}/services/StorageService/Storage/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${PROJECT_NAME} SHARED ${HistoryService_SOURCES})

if(TIZEN_BUILD)
    target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS})
endif(TIZEN_BUILD)

add_dependencies(${PROJECT_NAME} StorageService)
target_link_libraries(${PROJECT_NAME} StorageService)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)
