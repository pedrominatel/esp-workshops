set(src_dirs        ./)

set(include_dirs    ./)

set(requires    hand_gesture_recognition
                esp-dl
                who_task
                who_frame_cap
    )

if (IDF_TARGET STREQUAL "esp32s3")
    list(APPEND requires esp32_s3_eye_noglib)
elseif (IDF_TARGET STREQUAL "esp32p4")
    list(APPEND requires esp32_p4_function_ev_board)
endif()

idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires})
