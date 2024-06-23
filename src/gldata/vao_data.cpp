//
// Created by finn on 5/22/24.
//

#include "vao_data.h"

#include "../core/glerror.h"

#include <iostream>

VAOData::VAOData() {
    glGenVertexArrays(1, &data_id);
    GL_ERROR_CHECK();
    std::cout << "VAOData created: " << data_id << std::endl;
}

VAOData::~VAOData() {
    unbind();
    if (data_id != 0) {
        glDeleteVertexArrays(1, &data_id);
        GL_ERROR_CHECK();
    }
}

void VAOData::bind() {
    glBindVertexArray(data_id);
    GL_ERROR_CHECK();
}
void VAOData::unbind() {
    glBindVertexArray(0);
    GL_ERROR_CHECK();
}

void VAOData::addVBO(const VBODataPtr& vbo) {
    vbos.push_back(vbo);
}
