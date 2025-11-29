#pragma once

#include "resource_data.h"

#include "../gldata/texture_data.h"
#include "image_data.h"

class TextureResource : public ResourceData {
  public:
    TextureResource(const std::string& key, std::shared_ptr<ImageData> image, TextureSpecification specification);

    TextureData* texture() const { return gpu_texture_.get(); }
    const TextureSpecification& specification() const { return spec_; }

  protected:
    bool load_to_ram() override;
    void unload_from_ram() override;
    bool load_to_gpu() override;
    void unload_from_gpu() override;

  private:
    std::shared_ptr<ImageData> image_;
    TextureSpecification spec_;
    TextureData::UPtr gpu_texture_;
};
