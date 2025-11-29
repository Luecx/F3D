//
// Created by finn on 5/22/24.
//

#include "vbo_data.h"

#include "../core/glerror.h"

VBOData::VBOData() {
    glGenBuffers(1, &data_id);
    GL_ERROR_CHECK();
}

VBOData::~VBOData() {
    if (data_id != 0)
        glDeleteBuffers(1, &data_id);
}

void VBOData::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, data_id);
    GL_ERROR_CHECK();
}

void VBOData::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    GL_ERROR_CHECK();
}

void VBOData::store_data(int attributeNumber, int dimensions, std::vector<float>& data) {
    bind();
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), &data.front(), GL_STATIC_DRAW);
    GL_ERROR_CHECK();
    glVertexAttribPointer(attributeNumber, dimensions, GL_FLOAT, GL_FALSE, dimensions * sizeof(float), nullptr);
    GL_ERROR_CHECK();
    unbind();
}

void VBOData::store_data(int attributeNumber, int dimensions, std::vector<int>& data) {
    bind();
    glBufferData(GL_ARRAY_BUFFER, sizeof(int) * data.size(), &data.front(), GL_STATIC_DRAW);
    GL_ERROR_CHECK();
    glVertexAttribIPointer(attributeNumber, dimensions, GL_INT, 0, nullptr);
    GL_ERROR_CHECK();
    unbind();
}

void VBOData::store_indices(const std::vector<uint32_t>& indices) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data_id);
    GL_ERROR_CHECK();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
    GL_ERROR_CHECK();
    unbind();
}
