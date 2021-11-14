cmake_minimum_required(VERSION 3.20)
  
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(SimplePackage CXX C)

    # GoogleTest requires at least C++11
    set(CMAKE_CXX_STANDARD 17)
    
    add_subdirectory(src)

    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
    )

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(SimplePackage_unitTests
            tests/TEST_SimplePackage.cpp
            )

    target_link_libraries(
            SimplePackage_unitTests
            gtest_main
            gmock
            SimplePackage
    )

    include(GoogleTest)
    gtest_discover_tests(SimplePackage_unitTests)

    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fno-rtti -Werror=unused-parameter")
else()
    SET(ONLY_LIBRARY TRUE)
    add_subdirectory(src)
    UNSET(ONLY_LIBRARY)
endif()
