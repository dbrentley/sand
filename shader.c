//
// Created by dbrent on 2/20/21.
//

#include "shader.h"

#include <stdlib.h>
#include <stdio.h>

char *file_to_buffer(char *file) {
    FILE *f;
    long size;
    char *buffer;

    f = fopen(file, "r");
    if (f == NULL) {
        printf("Failed to open file %s\n", file);
        return NULL;
    }

    fseek(f, 0L, SEEK_END);
    size = ftell(f);
    fseek(f, 0L, SEEK_SET);

    buffer = calloc(size, sizeof(char));
    if (buffer == NULL) {
        printf("Could not create file buffer.\n");
        return NULL;
    }

    fread(buffer, sizeof(char), size, f);
    fclose(f);

    printf("Successfully read file %s\n", file);
    return buffer;
}

GLuint shader_create(GLenum type, char *file) {
    GLuint shader;
    char *info_log;
    const GLchar *shader_source = file_to_buffer(file);
    int max_length, is_compiled;

    shader = glCreateShader(type);
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
    if (is_compiled == GL_FALSE) {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);
        info_log = (char *) malloc(max_length);
        glGetShaderInfoLog(shader, max_length, &max_length, info_log);
        printf("%s\n", info_log);
        free(info_log);
        exit(-1);
    }
    free((void *) shader_source);
    return shader;
}

GLuint shader_create_s(GLenum type, const char *source) {
    GLuint shader;
    char *info_log;
    const GLchar *shader_source = source;
    int max_length, is_compiled;

    shader = glCreateShader(type);
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
    if (is_compiled == GL_FALSE) {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);
        info_log = (char *) malloc(max_length);
        glGetShaderInfoLog(shader, max_length, &max_length, info_log);
        printf("%s\n", info_log);
        free(info_log);
        exit(-1);
    }
    return shader;
}

GLuint shader_program_create(char *vertex_shader_file,
                             char *fragment_shader_file) {
    GLuint program, vertex_shader, fragment_shader;

    vertex_shader = shader_create(GL_VERTEX_SHADER, vertex_shader_file);
    fragment_shader = shader_create(GL_FRAGMENT_SHADER, fragment_shader_file);

    program = glCreateProgram();

    glAttachShader(program, fragment_shader);
    glAttachShader(program, vertex_shader);

    return program;
}

GLuint shader_program_create_s(char *vertex_shader_s, char *fragment_shader_s) {
    GLuint program, vertex_shader, fragment_shader;

    vertex_shader = shader_create_s(GL_VERTEX_SHADER, vertex_shader_s);
    fragment_shader = shader_create_s(GL_FRAGMENT_SHADER, fragment_shader_s);

    program = glCreateProgram();

    glAttachShader(program, fragment_shader);
    glAttachShader(program, vertex_shader);

    return program;
}

void shader_program_bind_attribute_location(GLuint program, GLuint index,
                                            const GLchar *name) {
    glBindAttribLocation(program, index, name);
}

GLint shader_program_get_uniform_location(GLuint program, const GLchar *name) {
    return glGetUniformLocation(program, name);
}

void shader_program_link(GLuint program) {
    // link the program. attribute binding must happen before this
    glLinkProgram(program);
}
