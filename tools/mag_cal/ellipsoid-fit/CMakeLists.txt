# Define library. Only source files here!
project(ellipsoid-fit)

find_package(Eigen3 REQUIRED)

include_directories(include/ellipsoid_fit EIGEN3_INCLUDE_DIR)

add_library(ellipsoid-fit
    src/ellipsoid_fit/common.cpp
    src/ellipsoid_fit/fit.cpp
    src/ellipsoid_fit/generate.cpp)



# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(ellipsoid-fit PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${Eigen3_INCLUDE_DIRS}
    PRIVATE src)

# If we have compiler requirements for this library, list them
# here
# target_compile_features(lib
#     PUBLIC cxx_auto_type
#     PRIVATE cxx_variadic_templates)

# Depend on a library that we defined in the top-level file
# target_link_libraries(mylib1
#     boost
#     MyOtherLibrary)

# 'make install' to the correct locations (provided by GNUInstallDirs).
# install(TARGETS mylib1 EXPORT MyLibraryConfig
#     ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})
# # This is for Windows
# install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# # This makes the project importable from the install directory
# # Put config file in per-project dir (name MUST match), can also
# # just go into 'cmake'.
# install(EXPORT MyLibraryConfig DESTINATION share/MyLibrary/cmake)

# This makes the project importable from the build directory
export(TARGETS ellipsoid-fit FILE MyLibraryConfig.cmake)

target_link_libraries(ellipsoid-fit
    ${Eigen3_LIBRARIES})

