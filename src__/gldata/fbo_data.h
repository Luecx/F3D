//
// Created by finn on 5/30/24.
//

#ifndef F3D_FBODATA_H
#define F3D_FBODATA_H

#include "gl_data.h"
#include "texture_data.h"
#include <vector>

#include <memory>

class FBOData : public GLData {

    std::vector<TextureData::SPtr> attachments;
    TextureData::SPtr depth_attachment;

    // type of fbo
    TextureType type;

  public:
    FBOData(TextureType type = TextureType::TEX_2D);
    ~FBOData();

  public:
    void bind() override;
    void unbind() override;

    TextureData::SPtr create_depth_attachment(int width, int height, const TextureSpecification& specification);
    TextureData::SPtr create_color_attachment(int width,
                                              int height,
                                              const TextureSpecification& specification,
                                              GLenum attachment = GL_COLOR_ATTACHMENT0);
    void attach_texture(GLenum attachment, const TextureData::SPtr& texture);

    TextureData* depth_texture() const { return depth_attachment.get(); }

    bool check_status();

    using SPtr = std::shared_ptr<FBOData>;
    using UPtr = std::unique_ptr<FBOData>;
};

#endif // F3D_FBODATA_H
