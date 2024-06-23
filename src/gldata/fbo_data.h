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

    std::vector<TextureDataPtr> attachments;

    // type of fbo
    TextureType type;

    public:
    FBOData(TextureType type = TextureType::TEX_2D);
    ~FBOData();

    public:
    void bind() override;
    void unbind() override;

    void create_depth_attachment(int width, int height);
    void create_color_attachment(int width, int height);

    bool check_status();

};

using FBODataSPtr = std::shared_ptr<FBOData>;
using FBODataUPtr = std::unique_ptr<FBOData>;

#endif    // F3D_FBODATA_H
