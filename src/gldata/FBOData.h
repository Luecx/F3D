//
// Created by finn on 5/30/24.
//

#ifndef F3D_FBODATA_H
#define F3D_FBODATA_H


#include "GLData.h"
#include "TextureData.h"
#include <vector>

#include <memory>

class FBOData : public GLData {

    std::vector<TextureDataPtr> attachments;

    public:
    FBOData();
    ~FBOData();

    public:
    void bind() override;
    void unbind() override;

    void create_depth_attachment(int width, int height);
    void create_color_attachment(int width, int height, int attachmentIndex = 0);

    bool check_status();

};

#endif    // F3D_FBODATA_H
