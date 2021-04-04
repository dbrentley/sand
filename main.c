#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shader.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define W_WIDTH 1280
#define W_HEIGHT 1280
#define MAX_PIXELS 10000000
#define VERTEX_ELEMENTS 20
#define VERTEX_STRIDE 5

bool should_close = false;
double mouse_x;
double mouse_y;

typedef struct {
    float x;
    float y;
} pos_t;

typedef struct {
    float r;
    float g;
    float b;
} color_t;

typedef struct {
    pos_t pos;
    color_t rgb;
    int index;
} pixel_t;

void window_close_callback(GLFWwindow *w) {
    should_close = true;
}

void error_callback(int error, const char *description) {
    printf("Error: %s\n", description);
}

void cursor_position_callback(GLFWwindow *w, double x_pos,
                              double y_pos) {
    mouse_x = x_pos;
    mouse_y = y_pos;
}

void scroll_callback(GLFWwindow *w, double x_offset, double y_offset) {
}

void mouse_button_callback(GLFWwindow *w, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {}
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {}
}

void keyboard_event(GLFWwindow *w, int key, int scancode, int action,
                    int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        should_close = true;
    }
}

void ffree(void *obj, char *v) {
    if (obj != NULL) {
        free(obj);
        obj = NULL;
    }
}

void checkm(void *obj) {
    if (obj == NULL) {
        printf("Could not allocate memory for object\n");
        exit(-1);
    }
}

void pixel_add(float x, float y) {

}

int main() {
    if (!glfwInit()) {
        printf("Could not initialize GLFW\n");
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow *window = glfwCreateWindow(W_WIDTH, W_HEIGHT, "sand", NULL,
                                          NULL);
    if (!window) {
        glfwTerminate();
        printf("Could not create window\n");
        return -1;
    }
    // keyboard
    glfwSetKeyCallback(window, keyboard_event);
    //mouse
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // window
    glfwSetErrorCallback(error_callback);
    glfwSetWindowCloseCallback(window, window_close_callback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);// v-sync
    glewExperimental = GL_TRUE;
    glewInit();
    glDisable(GL_DEPTH_TEST);
    // shader
    char *vs = "#version 330 core\n"
               "layout (location = 0) in vec2 in_Position;\n"
               "layout (location = 1) in vec3 in_Color;\n"
               "out vec3 vertexColor;\n"
               "void main()\n"
               "{\n"
               "    gl_Position = vec4(in_Position, 1.0, 1.0);\n"
               "    vertexColor = in_Color;\n"
               "}";
    char *fs = "#version 330 core\n"
               "out vec4 FragColor;\n"
               "in vec3 vertexColor;\n"
               "void main()\n"
               "{\n"
               "    FragColor = vec4(vertexColor, 1.0);\n"
               "}";
    GLuint program = shader_program_create_s(vs, fs);
    shader_program_bind_attribute_location(program, 0, "in_Position");
    shader_program_bind_attribute_location(program, 1, "in_Color");
    shader_program_link(program);
    // gl
    GLuint vao, vbo, ebo = 0;
    float *vertex_buffer;
    uint32_t *element_buffer;
    // vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // vbo
    vertex_buffer = malloc(MAX_PIXELS * sizeof(float) * VERTEX_ELEMENTS);
    checkm(vertex_buffer);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 MAX_PIXELS * sizeof(float) * VERTEX_ELEMENTS,
                 NULL, GL_DYNAMIC_DRAW);
    // ebo
    element_buffer = malloc(MAX_PIXELS * sizeof(uint32_t) * 6);
    checkm(element_buffer);
    int p = 0;
    for (int e = 0; e < MAX_PIXELS - 6; e += 6) {
        element_buffer[e] = 0 + p;
        element_buffer[e + 1] = 1 + p;
        element_buffer[e + 2] = 2 + p;
        element_buffer[e + 3] = 2 + p;
        element_buffer[e + 4] = 3 + p;
        element_buffer[e + 5] = 0 + p;
        p += 4;
    }
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_PIXELS * sizeof(uint32_t),
                 element_buffer, GL_STATIC_DRAW);
    // position attribute pointer
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          VERTEX_STRIDE * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    // uv attribute pointer
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          VERTEX_STRIDE * sizeof(float),
                          (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // pixels
    pixel_t **pixels = malloc(MAX_PIXELS * sizeof(pixel_t *));
    checkm(pixels);
    for (int x = 0; x < MAX_PIXELS; x++) {
        pixels[x] = malloc(sizeof(pixel_t));
        checkm(pixels[x]);
        pixels[x]->index = -1;
    }

    // game loop
    double delta = 0;
    double start_time = glfwGetTime();
    double previous_time = glfwGetTime();
    int frame_count = 0;
    while (!should_close) {
        start_time = glfwGetTime();
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.169f, 0.169f, 0.169f, 0.1f);
        glUseProgram(program);

        for (int x = 0; x < MAX_PIXELS; x++) {
            if (pixels[x]->index) {

            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        delta = glfwGetTime() - start_time;
        frame_count++;
        if (start_time - previous_time >= 1.0) {
            printf("frame: %.2f, fps: %d\n", delta * 1000, frame_count);
            previous_time = start_time;
            frame_count = 0;
        }
    }

    glfwTerminate();
    return 0;
}
