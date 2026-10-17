# Source files
if(CONFIG_COMPACT)
    add_custom_command(
        OUTPUT slides.txt
        COMMAND
            tail -n +2 ${CMAKE_CURRENT_SOURCE_DIR}/slides.txt > slides.txt
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/slides.txt)

    set(ASSETS
        slides.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/lavndr73.bmp
        ${CMAKE_CURRENT_SOURCE_DIR}/lavndr99.bmp)
else()
    set(ASSETS
        ${CMAKE_CURRENT_SOURCE_DIR}/slides.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/presskey.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/windows.ico
        ${CMAKE_CURRENT_SOURCE_DIR}/*.bmp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.enc
        ${CMAKE_CURRENT_SOURCE_DIR}/*.mid)
endif()

# DOS drivers
get_target_property(DRIVERS lavender ANDREA_DRIVERS)
if(DRIVERS MATCHES "-NOTFOUND$")
    set(DRIVERS "")
endif()

# Asset archive
add_custom_command(
    OUTPUT data.zip
    COMMAND
        zip -0 -r -j data.zip
        ${ASSETS}
        ${DRIVERS}
    DEPENDS ${ASSETS})

# Bundle
add_custom_command(
    OUTPUT sshow${CMAKE_EXECUTABLE_SUFFIX}
    COMMAND ${CMAKE_COMMAND}
        -DEXECUTABLE_PATH=$<TARGET_FILE:lavender>
        -DARCHIVE_PATH=data.zip
        -DBUNDLE_PATH=sshow${CMAKE_EXECUTABLE_SUFFIX}
        -DMAX_SIZE=${MAX_BUNDLE_SIZE}
        -P ${CMAKE_SOURCE_DIR}/cmake/scripts/bundle.cmake
    DEPENDS lavender data.zip)

add_custom_target(bundle DEPENDS sshow${CMAKE_EXECUTABLE_SUFFIX})
if(NOT CMAKE_SYSTEM_PROCESSOR STREQUAL ALPHA)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/sshow${CMAKE_EXECUTABLE_SUFFIX}" DESTINATION .)
endif()
