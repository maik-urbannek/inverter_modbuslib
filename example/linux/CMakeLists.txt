cmake_minimum_required(VERSION 3.4.3)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/example/linux/)

if (( NOT "${CMAKE_GUI_ONLY}" STREQUAL "true") AND ( NOT "${CMAKE_REQUEST_ONLY}" STREQUAL "true"))
    set(CMAKE_BUILD_RESPOND "true")
    set(CMAKE_BUILD_REQUEST "true")
    set(CMAKE_BUILD_GUI "true")
else()
    if ("${CMAKE_GUI_ONLY}" STREQUAL "true")
        set(CMAKE_BUILD_GUI "true")
    else()
        set(CMAKE_BUILD_GUI "false")
    endif()
    if ("${CMAKE_REQUEST_ONLY}" STREQUAL "true")
        set(CMAKE_BUILD_REQUEST "true")
    else()
        set(CMAKE_BUILD_REQUEST "false")
    endif()
    set(CMAKE_BUILD_RESPOND "false")
endif()

if ("${CMAKE_BUILD_RESPOND}" STREQUAL "true")
    add_library(main_responder                                                          OBJECT   main_Responder.cpp                                                                    )
    add_executable(Responder                                                            $<TARGET_OBJECTS:main_responder>                                                               )
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13.0)
            target_compile_options(main_responder   PUBLIC "-Wno-maybe-uninitialized")
        endif()
    endif()
    install(TARGETS Responder RUNTIME DESTINATION bin)
endif()

if ("${CMAKE_BUILD_REQUEST}" STREQUAL "true")
    add_library(main_sendrequest                                                        OBJECT   main_SendRequest.cpp                                                                  )
    add_executable(SendRequest                                                          $<TARGET_OBJECTS:main_sendrequest>                                                             )
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13.0)
            target_compile_options(main_sendrequest PUBLIC "-Wno-maybe-uninitialized")
        endif()
    endif()
    install(TARGETS SendRequest RUNTIME DESTINATION bin)
endif()

if ("${CMAKE_BUILD_GUI}" STREQUAL "true")
    add_subdirectory(gui)
endif()
