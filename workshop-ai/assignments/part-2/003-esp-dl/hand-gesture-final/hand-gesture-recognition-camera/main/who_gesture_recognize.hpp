#pragma once
#include "hand_detect.hpp"
#include "hand_gesture_recognition.hpp"
#include "who_frame_cap.hpp"

namespace who {
namespace gesture {

class WhoGestureRecognize : public task::WhoTask {
public:
    static inline constexpr EventBits_t NEW_FRAME = frame_cap::WhoFrameCapNode::NEW_FRAME;

    WhoGestureRecognize(const std::string &name,
                        frame_cap::WhoFrameCapNode *frame_cap_node,
                        HandDetect *hand_detect,
                        HandGestureRecognizer *recognizer);
    ~WhoGestureRecognize();

    static void preload_models(HandDetect **hand_detect, HandGestureRecognizer **recognizer);
    void set_fps(float fps);
    bool run(const configSTACK_DEPTH_TYPE uxStackDepth, UBaseType_t uxPriority, const BaseType_t xCoreID) override;
    bool stop_async() override;
    bool pause_async() override;

private:
    void task() override;

    frame_cap::WhoFrameCapNode *m_frame_cap_node;
    HandDetect *m_hand_detect;
    HandGestureRecognizer *m_recognizer;
    void *m_copy_buf;
    TickType_t m_interval;
};

} // namespace gesture
} // namespace who
