#pragma once
#include "who_frame_cap.hpp"

#if CONFIG_IDF_TARGET_ESP32S3
inline constexpr uint16_t PROCESS_FRAME_W = 480;
inline constexpr uint16_t PROCESS_FRAME_H = 480;
inline constexpr const char *PROCESS_FRAME_NODE = "FrameCap480";

who::frame_cap::WhoFrameCap *get_dvp_frame_cap_pipeline();
who::frame_cap::WhoFrameCapNode *get_processing_frame_node(who::frame_cap::WhoFrameCap *frame_cap);
#elif CONFIG_IDF_TARGET_ESP32P4
who::frame_cap::WhoFrameCap *get_mipi_csi_frame_cap_pipeline();
who::frame_cap::WhoFrameCap *get_uvc_frame_cap_pipeline();
#endif
