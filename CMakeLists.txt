cmake_minimum_required(VERSION 3.5) # CMake install : https://cmake.org/download/
project(KeruisUtils LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "D:/Qt/6.5.3/mingw_64") # Qt Kit Dir
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_QMAKE_NO_WINDOW_MAIN TRUE)

find_package(Qt6 COMPONENTS 
                Widgets 
                Svg
                REQUIRED) # Qt COMPONENTS

aux_source_directory(./src srcs)

add_executable(${PROJECT_NAME}
    WIN32 # If you need a terminal for debug, please comment this statement 
    src/RedialMenu/RedialMenu.cpp
    src/FloatingBall/FloatingBall.cpp
    ${srcs} 
) 

target_link_libraries(${PROJECT_NAME} PRIVATE 
                        Qt6::Widgets
                        Qt6::Svg
                        ) # Qt5 Shared Library