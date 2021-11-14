add_library(SimplePackage STATIC
        SimplePackage.cpp)

target_include_directories(SimplePackage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(DEFINED ${ONLY_LIBRARY})
    target_include_directories(SimplePackage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../)
else()
    target_include_directories(SimplePackage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../SimpleStream)
endif()