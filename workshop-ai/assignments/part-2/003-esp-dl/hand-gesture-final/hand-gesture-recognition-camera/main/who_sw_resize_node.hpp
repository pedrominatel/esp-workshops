#pragma once
#include "dl_image.hpp"
#include "who_frame_cap_node.hpp"

namespace who {
namespace frame_cap {

class WhoSwResizeNode : public WhoFrameCapNode {
public:
    WhoSwResizeNode(const std::string &name,
                    uint16_t dst_w,
                    uint16_t dst_h,
                    dl::image::pix_type_t dst_pix_type,
                    uint8_t ringbuf_len,
                    bool out_queue_overwrite = true);
    ~WhoSwResizeNode();
    uint16_t get_fb_width() override { return m_dst_w; }
    uint16_t get_fb_height() override { return m_dst_h; }
    std::string get_type() override { return "SwResizeNode"; }

private:
    void cleanup() override;
    who::cam::cam_fb_t *process(who::cam::cam_fb_t *fb) override;
    void update_ringbuf(who::cam::cam_fb_t *fb) override;
    dl::image::img_t get_dst_img();

    uint16_t m_dst_w;
    uint16_t m_dst_h;
    dl::image::pix_type_t m_dst_pix_type;
    dl::image::ImageTransformer m_transformer;
    std::vector<dl::image::img_t> m_dst_imgs;
    int m_img_idx;
};

} // namespace frame_cap
} // namespace who
