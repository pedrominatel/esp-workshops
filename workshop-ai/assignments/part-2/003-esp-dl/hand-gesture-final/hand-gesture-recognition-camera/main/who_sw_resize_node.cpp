#include "who_sw_resize_node.hpp"
#include "esp_log.h"

static const char *TAG = "WhoSwResizeNode";

namespace who {
namespace frame_cap {

WhoSwResizeNode::WhoSwResizeNode(const std::string &name,
                                 uint16_t dst_w,
                                 uint16_t dst_h,
                                 dl::image::pix_type_t dst_pix_type,
                                 uint8_t ringbuf_len,
                                 bool out_queue_overwrite) :
    WhoFrameCapNode(name, ringbuf_len, out_queue_overwrite),
    m_dst_w(dst_w),
    m_dst_h(dst_h),
    m_dst_pix_type(dst_pix_type),
    m_dst_imgs(ringbuf_len + 1),
    m_img_idx(0)
{
    for (auto &img : m_dst_imgs) {
        img.width = dst_w;
        img.height = dst_h;
        img.pix_type = dst_pix_type;
        img.data = heap_caps_malloc(dl::image::get_img_byte_size(img), MALLOC_CAP_SPIRAM);
        ESP_ERROR_CHECK(img.data ? ESP_OK : ESP_ERR_NO_MEM);
    }
}

WhoSwResizeNode::~WhoSwResizeNode()
{
    for (auto &img : m_dst_imgs) {
        heap_caps_free(img.data);
    }
}

void WhoSwResizeNode::cleanup()
{
    while (uxQueueMessagesWaiting(m_in_queue) > 0) {
        cam::cam_fb_t *tmp = nullptr;
        xQueueReceive(m_in_queue, &tmp, 0);
    }
    xSemaphoreTake(m_mutex, portMAX_DELAY);
    while (!m_cam_fbs.empty()) {
        delete m_cam_fbs.pop();
    }
    xSemaphoreGive(m_mutex);
}

cam::cam_fb_t *WhoSwResizeNode::process(cam::cam_fb_t *fb)
{
    auto timestamp = fb->timestamp;
    auto dst_img = get_dst_img();
    dl::image::img_t src_img = static_cast<dl::image::img_t>(*fb);
#if CONFIG_IDF_TARGET_ESP32P4
    uint32_t caps = 0;
#else
    uint32_t caps = dl::image::DL_IMAGE_CAP_RGB565_BIG_ENDIAN;
#endif
    if (m_transformer.set_src_img(src_img).set_dst_img(dst_img).set_caps(caps).transform() != ESP_OK) {
        ESP_LOGE(TAG, "%s: resize failed", get_name().c_str());
        return nullptr;
    }
    return new cam::cam_fb_t(dst_img, timestamp);
}

void WhoSwResizeNode::update_ringbuf(cam::cam_fb_t *fb)
{
    if (m_cam_fbs.full()) {
        delete m_cam_fbs.pop();
    }
    m_cam_fbs.push(fb);
}

dl::image::img_t WhoSwResizeNode::get_dst_img()
{
    auto &img = m_dst_imgs[m_img_idx];
    m_img_idx = (m_img_idx + 1) % static_cast<int>(m_dst_imgs.size());
    return img;
}

} // namespace frame_cap
} // namespace who
