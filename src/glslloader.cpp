#include "glslloader.h"

// Public
GLuint glsl::createShaderProgram(const char *vert_filename, const char *frag_filename)
{
    // Read vertex and fragment shaders from file
    char *vert_source, *frag_source;
    int32_t vert_length = readFile(vert_filename, &vert_source);
    int32_t frag_length = readFile(frag_filename, &frag_source);
    if (vert_length < 0 || frag_length < 0)
    {
        return 0;
    }

    // Compile vetex shader
    GLuint vertex_shader = compileShader(vert_source, vert_length, GL_VERTEX_SHADER);
    // Compile fragment shader
    GLuint fragment_shader = compileShader(frag_source, frag_length, GL_FRAGMENT_SHADER);

    // Create GPU program from the compiled vertex and fragment shaders
    GLuint shaders[2] = {vertex_shader, fragment_shader};
    GLuint program = attachShaders(shaders, 2);

    return program;
}

void glsl::linkShaderProgram(GLuint program)
{
    // Link GPU program
    GLint status;
    glLinkProgram(program);

    // Check to see if it linked successfully
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == 0)
    {
        GLint log_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        char *info = new char[log_length + 1];
        glGetProgramInfoLog(program, log_length, NULL, info);
        fprintf(stderr, "Error: failed to link shader program\n");
        fprintf(stderr, "%s\n", info);
        delete[] info;
    }
}

void glsl::getShaderProgramUniforms(GLuint program, std::map<std::string,GLint>& uniforms)
{
    // Get handles to uniform variables defined in the shaders
    GLint num_uniforms;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &num_uniforms);
    int i;
    GLchar uniform_name[65];
    GLsizei max_name_length = 64;
    GLsizei name_length;
    GLint size;
    GLenum type;
    for (i = 0; i < num_uniforms; i++)
    {
        glGetActiveUniform(program, i, max_name_length, &name_length, &size, &type, uniform_name);
        uniforms[uniform_name] = glGetUniformLocation(program, uniform_name);
    }
}


// Private
GLint glsl::compileShader(char *source, int32_t length, GLenum type)
{
    // Create a shader object
    GLint status;
    GLuint shader = glCreateShader(type);

    // Send the source to the shader object
    const char *src_bytes = const_cast<const char*>(source);
    const GLint len = length;
    glShaderSource(shader, 1, &src_bytes, &len);

    // Compile the shader program
    glCompileShader(shader);

    // Check to see if it compiled successfully
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        char *info = new char[log_length + 1];
        glGetShaderInfoLog(shader, log_length, NULL, info);
        std::string shader_type = shaderTypeToString(type);
        fprintf(stderr, "Error: failed to compile %s shader:\n", shader_type.c_str());
        fprintf(stderr, "%s\n", info);
        delete[] info;

        return -1;
    }

    return shader;
}

GLuint glsl::attachShaders(GLuint shaders[], uint16_t num_shaders)
{
    // Create a GPU program
    GLuint program = glCreateProgram();

    // Attach all shaders to that program
    int i;
    for (i = 0; i < num_shaders; i++)
    {
        glAttachShader(program, shaders[i]);
    }

    return program;
}

std::string glsl::shaderTypeToString(GLenum type)
{
    std::string shader_type;
    switch (type)
    {
        case GL_VERTEX_SHADER:
            shader_type = "vertex";
            break;
        case GL_GEOMETRY_SHADER:
            shader_type = "geometry";
            break;
        case GL_FRAGMENT_SHADER:
            shader_type = "fragment";
            break;
    }
    return shader_type;
}

int32_t glsl::readFile(const char* filename, char** data_ptr)
{
    FILE *fp;
    int err = 0;
#ifdef _WIN32
    err = fopen_s(&fp, filename, "rb");
#else
    fp = fopen(filename, "rb");
#endif
    if (err != 0 || fp == NULL)
    {
        fprintf(stderr, "Error: cannot open %s\n", filename);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    int32_t fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    *data_ptr = (char*)malloc(fsize);
    size_t read = fread(*data_ptr, fsize, 1, fp);
    if (read != 1)
    {
        fprintf(stderr, "Error: cannot read %s\n", filename);
        return -1;
    }

    fclose(fp);

    return fsize;
}

