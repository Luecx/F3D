//
// Created by finn on 5/22/24.
//

#ifndef ENGINE3D_VAOData_H
#define ENGINE3D_VAOData_H

#include "gl_data.h"
#include "vbo_data.h"

#include <memory>
#include <vector>

class VAOData : public GLData {
    std::vector<VBODataPtr> vbos;

    public:
    VAOData();
    ~VAOData();

    public:
    void bind() override;
    void unbind() override;

    void addVBO(const VBODataPtr& vbo);
};

using VAODataPtr = std::shared_ptr<VAOData>;

#endif    // ENGINE3D_VAOData_H
