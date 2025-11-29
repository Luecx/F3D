#pragma once

#include <glad/glad.h>
#include "../math/mat.h"

#include <string>
#include <unordered_set>
#include <filesystem>

class ShaderProgram {
  private:
    bool created = false;
    bool warnings = false;

    std::string vertex_file_;
    std::string fragment_file_;
    std::string geometry_file_;
    std::string tess_control_file_;
    std::string tess_eval_file_;

    GLuint program_id = 0;
    GLuint vertex_shader_id = 0;
    GLuint fragment_shader_id = 0;
    GLuint geometry_shader_id = 0;
    GLuint tess_control_shader_id = 0;
    GLuint tess_eval_shader_id = 0;

  public:
    ShaderProgram() = default;
    virtual ~ShaderProgram();

    ShaderProgram& vertex_file(const std::string& file);
    ShaderProgram& fragment_file(const std::string& file);
    ShaderProgram& geometry_file(const std::string& file);
    ShaderProgram& tess_control_file(const std::string& file);
    ShaderProgram& tess_eval_file(const std::string& file);
    ShaderProgram& compile();

    void start();
    void stop();
    bool is_created() const;
    GLuint program_id_handle() const { return program_id; }

  protected:
    virtual void get_all_uniform_locations() {};
    virtual void bind_attributes() {};
    virtual void connect_all_texture_units() {};

    int get_uniform_location(const std::string& uniform_name);
    int load_shader(const std::string& file, GLenum type);
    std::string preprocess_shader(const std::string& file_path, std::unordered_set<std::string>& visited);

  public:
    void load_float(int location, float value);
    void load_int(int location, int value);
    void load_bool(int location, bool value);

    void load_vector(int location, Vec2f& vec);
    void load_vector(int location, Vec3f& vec);
    void load_vector(int location, Vec4f& vec);
    void load_vector(int location, float x, float y, float z);
    void load_vector(int location, float x, float y, float z, float w);

    void load_matrix(int location, Mat4f& matrix);
};
