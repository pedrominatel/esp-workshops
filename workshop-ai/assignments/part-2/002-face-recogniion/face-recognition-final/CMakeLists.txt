# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# ESP-WHO: set IDF_EXTRA_ACTIONS_PATH to esp-who/tools (e.g. export IDF_EXTRA_ACTIONS_PATH=~/.../esp-who/tools).
# EXTRA_COMPONENT_DIRS are taken from sibling ../components relative to that path.
if("$ENV{IDF_EXTRA_ACTIONS_PATH}" STREQUAL "")
    message(FATAL_ERROR "IDF_EXTRA_ACTIONS_PATH is not set. Point it at esp-who/tools, e.g.\n"
        "  export IDF_EXTRA_ACTIONS_PATH=~/Documents/Espressif/github/esp-who/tools")
endif()
get_filename_component(ESP_WHO_COMPONENTS_DIR "$ENV{IDF_EXTRA_ACTIONS_PATH}/../components" ABSOLUTE)
if(NOT EXISTS "${ESP_WHO_COMPONENTS_DIR}/who_task/CMakeLists.txt")
    message(FATAL_ERROR "ESP-WHO components not found under:\n  ${ESP_WHO_COMPONENTS_DIR}\n"
        "Check IDF_EXTRA_ACTIONS_PATH points at esp-who/tools (not the repo root). Current value:\n"
        "  $ENV{IDF_EXTRA_ACTIONS_PATH}")
endif()

set(EXTRA_COMPONENT_DIRS ${ESP_WHO_COMPONENTS_DIR}/who_task
                         ${ESP_WHO_COMPONENTS_DIR}/who_peripherals/who_usb
                         ${ESP_WHO_COMPONENTS_DIR}/who_peripherals/who_cam
                         ${ESP_WHO_COMPONENTS_DIR}/who_peripherals/who_lcd
                         ${ESP_WHO_COMPONENTS_DIR}/who_peripherals/who_spiflash_fatfs
                         ${ESP_WHO_COMPONENTS_DIR}/who_frame_cap
                         ${ESP_WHO_COMPONENTS_DIR}/who_frame_lcd_disp
                         ${ESP_WHO_COMPONENTS_DIR}/who_detect
                         ${ESP_WHO_COMPONENTS_DIR}/who_recognition
                         ${ESP_WHO_COMPONENTS_DIR}/who_app/who_recognition_app)

add_compile_options(-fdiagnostics-color=always)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# ESP-WHO's idf.py extension only passes -DBSP= when project_dir basename is exactly "human_face_recognition".
# For copies under other folder names (e.g. workshops), derive BSP from sdkconfig.bsp.<board> defaults.
if(NOT DEFINED BSP AND DEFINED SDKCONFIG_DEFAULTS)
    string(REGEX MATCH "^sdkconfig\\.bsp\\.([A-Za-z0-9_]+)$" _sdk_bsp_defaults "${SDKCONFIG_DEFAULTS}")
    if(_sdk_bsp_defaults)
        set(BSP "${CMAKE_MATCH_1}")
    endif()
    unset(_sdk_bsp_defaults)
endif()
if(NOT DEFINED BSP)
    message(FATAL_ERROR
        "BSP is not defined. Use -DSDKCONFIG_DEFAULTS=sdkconfig.bsp.<board> when configuring, rename the folder to \"human_face_recognition\", "
        "or export IDF_EXTRA_ACTIONS_PATH to esp-who/tools so the example idf.py extension can infer BSP.")
endif()
idf_build_set_property(DEPENDENCIES_LOCK dependencies.lock.${BSP})
project(human_face_recognition)
