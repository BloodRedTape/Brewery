cmake_minimum_required(VERSION 3.10)

project(Brewery)

set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(StraitXPackages/Base)
add_subdirectory(StraitXPackages/ImGui)
add_subdirectory(thirdparty/sqlite-amalgamation)

set(BREWERY_SOURCES
    sources/main.cpp
    thirdparty/implot/implot.cpp
    thirdparty/implot/implot_demo.cpp
    thirdparty/implot/implot_items.cpp
)

set(BREWERY_INCLUDE
    sources/
	thirdparty/implot/
)

add_executable(Brewery ${BREWERY_SOURCES})
target_link_libraries(Brewery StraitXBase StraitXImGui SQLite3)
target_include_directories(Brewery
    PUBLIC ${BREWERY_INCLUDE}
    PUBLIC thirdparty/sqlite-amalgamation
)