#include "ShaderProgram.h"
#include "../core/glerror.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

ShaderProgram::~ShaderProgram() {
    if (!created)
        return;

    stop();
    glDetachShader(program_id, vertex_shader_id);
    glDetachShader(program_id, fragment_shader_id);
    glDetachShader(program_id, geometry_shader_id);
    glDetachShader(program_id, tess_control_shader_id);
    glDetachShader(program_id, tess_eval_shader_id);

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);
    glDeleteShader(geometry_shader_id);
    glDeleteShader(tess_control_shader_id);
    glDeleteShader(tess_eval_shader_id);

    glDeleteProgram(program_id);
}

ShaderProgram& ShaderProgram::vertex_file(const string& file) {
    vertex_file_ = file;
    return *this;
}

ShaderProgram& ShaderProgram::fragment_file(const string& file) {
    fragment_file_ = file;
    return *this;
}

ShaderProgram& ShaderProgram::geometry_file(const string& file) {
    geometry_file_ = file;
    return *this;
}

ShaderProgram& ShaderProgram::tess_control_file(const string& file) {
    tess_control_file_ = file;
    return *this;
}

ShaderProgram& ShaderProgram::tess_eval_file(const string& file) {
    tess_eval_file_ = file;
    return *this;
}

ShaderProgram& ShaderProgram::compile() {
    if (created)
        return *this;
    created = true;

    vertex_shader_id = load_shader(vertex_file_, GL_VERTEX_SHADER);
    fragment_shader_id = load_shader(fragment_file_, GL_FRAGMENT_SHADER);
    geometry_shader_id = geometry_file_.empty() ? 0 : load_shader(geometry_file_, GL_GEOMETRY_SHADER);
    tess_control_shader_id = tess_control_file_.empty() ? 0 : load_shader(tess_control_file_, GL_TESS_CONTROL_SHADER);
    tess_eval_shader_id = tess_eval_file_.empty() ? 0 : load_shader(tess_eval_file_, GL_TESS_EVALUATION_SHADER);

    program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    if (geometry_shader_id)
        glAttachShader(program_id, geometry_shader_id);
    if (tess_control_shader_id)
        glAttachShader(program_id, tess_control_shader_id);
    if (tess_eval_shader_id)
        glAttachShader(program_id, tess_eval_shader_id);

    bind_attributes();
    glLinkProgram(program_id);
    glValidateProgram(program_id);

    get_all_uniform_locations();

    start();
    connect_all_texture_units();
    stop();

    printf("%-13s %-100s %-20s\n", "Linking", "", warnings ? "Status = WARNINGS" : "Status = SUCCESSFUL");
    fflush(stdout);
    return *this;
}

bool ShaderProgram::is_created() const { return created; }

void ShaderProgram::start() {
    if (created)
        glUseProgram(program_id);
}
void ShaderProgram::stop() { glUseProgram(0); }

int ShaderProgram::load_shader(const string& file, GLenum type) {
    unordered_set<string> visited;
    string source = preprocess_shader(file, visited);
    if (source.empty()) {
        printf("%-13s %-100s %-20s\n", "Compile", ("<" + file + ">").c_str(), "Status = NOT EXISTING SOURCE");
        fflush(stdout);
        return 0;
    }

    const char* src = source.c_str();
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    GLint result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        printf("%-13s %-100s %-20s\n", "Compile", ("<" + file + ">").c_str(), "Status = INCOMPLETE");
        GLint log_len = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_len);
        auto* log = new char[log_len + 1];
        glGetShaderInfoLog(id, log_len, nullptr, log);
        cerr << log << endl;
        delete[] log;
        return 0;
    }

    printf("%-13s %-100s %-20s\n", "Compile", ("<" + file + ">").c_str(), "Status = SUCCESSFUL");
    fflush(stdout);
    return id;
}

string ShaderProgram::preprocess_shader(const string& path, unordered_set<string>& visited) {
    if (visited.count(path))
        return ""; // prevent include cycles
    visited.insert(path);

    ifstream file(path);
    if (!file.is_open())
        return "";

    stringstream result;
    string line;
    // Get the directory of the current file.
    fs::path current_path(path);
    fs::path base_dir = current_path.parent_path();

    while (getline(file, line)) {
        if (line.find("#include") == 0) {
            size_t start = line.find_first_of("\"<") + 1;
            size_t end = line.find_last_of("\">");
            string include_file = line.substr(start, end - start);
            fs::path full_path = base_dir / include_file;
            ifstream inc(full_path);
            if (inc.good()) {
                result << "// Begin include: " << include_file << "\n";
                result << preprocess_shader(full_path.string(), visited);
                result << "// End include: " << include_file << "\n";
            } else {
                cerr << "Include file not found: " << full_path << endl;
            }
        } else {
            result << line << '\n';
        }
    }
    return result.str();
}

// -------- Uniform utilities --------

int ShaderProgram::get_uniform_location(const string& name) {
    int loc = glGetUniformLocation(program_id, name.c_str());
    if (loc == -1) {
        printf("%-13s %-30s %-20s\n", "Warning", name.c_str(), " is unused or missing!");
        warnings = true;
    }
    return loc;
}

void ShaderProgram::load_float(int location, float value) { glUniform1f(location, value); }
void ShaderProgram::load_int(int location, int value) { glUniform1i(location, value); }
void ShaderProgram::load_bool(int location, bool value) { glUniform1f(location, value ? 1.0f : 0.0f); }
void ShaderProgram::load_vector(int location, Vec2f& vec) { glUniform2f(location, vec[0], vec[1]); }
void ShaderProgram::load_vector(int location, Vec3f& vec) { glUniform3f(location, vec[0], vec[1], vec[2]); }
void ShaderProgram::load_vector(int location, Vec4f& vec) { glUniform4f(location, vec[0], vec[1], vec[2], vec[3]); }
void ShaderProgram::load_vector(int location, float x, float y, float z) { glUniform3f(location, x, y, z); }
void ShaderProgram::load_vector(int location, float x, float y, float z, float w) { glUniform4f(location, x, y, z, w); }
void ShaderProgram::load_matrix(int location, Mat4f& matrix) {
    float values[16];
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            values[col * 4 + row] = matrix(row, col); // convert row-major storage to column-major upload order
        }
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, values);
}
