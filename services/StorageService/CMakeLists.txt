PROJECT(StorageService)

set(StorageServiceSOURCES
    StorageException.cpp
    StorageExceptionInitialization.cpp
    DriverManager.cpp
    Field.cpp
    FoldersStorage.cpp
    QuickAccessStorage.cpp
    CertificateStorage.cpp
    SQLTransactionScope.cpp
    DBTools.cpp
    SQLDatabase.cpp
    SettingsStorage.cpp
    StorageService.cpp
)

include(Coreheaders)
include(EFLHelpers)

include_directories(${CMAKE_SOURCE_DIR}/core/Tools)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${PROJECT_NAME} SHARED ${StorageServiceSOURCES})
target_link_libraries(${PROJECT_NAME} -lboost_date_time)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)
