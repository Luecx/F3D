//
// Created by finn on 5/22/24.
//

#ifndef ENGINE3D_VBOID_H
#define ENGINE3D_VBOID_H

#include "GLData.h"

#include <vector>
#include <memory>

class VBOData : public GLData {

    public:
    VBOData();
    ~VBOData();


    public:
    void bind() override;
    void unbind() override;

    void store_data(int attributeNumber, int dimensions, std::vector<float>& data);
    void store_data(int attributeNumber, int dimensions, std::vector<int>& data);
    void store_indices(std::vector<int>& indices);

};

using VBODataPtr = std::shared_ptr<VBOData>;

#endif    // ENGINE3D_VBOID_H
