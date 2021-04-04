#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "linmath.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define W_WIDTH 1920
#define W_HEIGHT 1080
#define MAX_PIXELS 10000
#define VERTEX_ELEMENTS 20
#define VERTEX_STRIDE 5

typedef struct pixel_t pixel_t;

typedef void (*update_f)(pixel_t *);

typedef enum {
    SAND
} pixel_type_e;

typedef struct {
    float x;
    float y;
} pos_t;

typedef struct {
    float r;
    float g;
    float b;
} rgb_t;

struct pixel_t {
    pos_t pos;
    rgb_t rgb;
    int index;
    float mass;
    float velocity;
    float life_time;
    update_f update;
    pixel_type_e type;

};

typedef struct {
    pos_t pos;
    rgb_t rgb;
} pixel_vertex_t;

bool should_close = false;
double mouse_x;
double mouse_y;
float zoom;
float *vertex_buffer;
float gravity;
int pixel_count;
int w_width, w_height;
pixel_t **pixels;
mat4x4 mvp;

void window_close_callback(GLFWwindow *w) {
    should_close = true;
}

void set_aspect(int width, int height) {
    float aspect = (float) width / (float) height;
    mat4x4 m, p;
    mat4x4_identity(m);
    mat4x4_ortho(p, -aspect * zoom, aspect * zoom, zoom, -zoom, 1, -1);
    mat4x4_translate_in_place(p, -aspect * zoom, -zoom, -1);
    mat4x4_mul(mvp, p, m);
}

void resize_callback(GLFWwindow *w, int width, int height) {
    w_width = width;
    w_height = height;
    set_aspect(w_width, w_height);
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

void ffree(void *obj) {
    if (obj != NULL) {
        free(obj);
        obj = NULL;
    }
}

void update(pixel_t *pixel) {
    for (int x = 0; x < w_width; x++) {
        for (int y = 0; y < w_height; y++) {

        }
    }
    switch (pixel->type) {
        case SAND:

            break;
        default:
            break;
    }
}

void checkm(void *obj) {
    if (obj == NULL) {
        printf("Could not allocate memory for object\n");
        exit(-1);
    }
}

void repack() {
    float *pixel_buffer = malloc(pixel_count * VERTEX_ELEMENTS * sizeof(float));
    for (int i = 0; i < MAX_PIXELS; i++) {
        if (pixels[i]->index != -1) {
            int offset = pixels[i]->index * VERTEX_ELEMENTS;
            memcpy(pixel_buffer + (i * VERTEX_ELEMENTS), vertex_buffer + offset,
                   VERTEX_ELEMENTS * sizeof(float));
        }
    }
    memcpy(vertex_buffer, pixel_buffer,
           pixel_count * VERTEX_ELEMENTS * sizeof(float));
    ffree(pixel_buffer);
}

void pixel_add(float x, float y, pixel_type_e type) {
    int i;
    for (i = 0; i < MAX_PIXELS; i++) {
        if (pixels[i]->index == -1) { break; }
    }

    pos_t pos;
    pos.x = x;
    pos.y = y;

    rgb_t rgb;

    switch (type) {
        case SAND:
            rgb.r = 1.0f;
            rgb.g = 0.89f;
            rgb.b = 0.623f;
            break;
        default:
            break;
    }

    pixels[i]->index = i;
    pixels[i]->pos = pos;
    pixels[i]->rgb = rgb;
    pixels[i]->type = type;

    pixel_vertex_t p[4];
    float scale = 0.5f;
    x += scale;
    y += scale;
    // ll
    p[0].pos.x = x - scale;
    p[0].pos.y = y - scale;
    // lr
    p[1].pos.x = x + scale;
    p[1].pos.y = y - scale;
    // ul
    p[2].pos.x = x + scale;
    p[2].pos.y = y + scale;
    // ul
    p[3].pos.x = x - scale;
    p[3].pos.y = y + scale;
    // color
    for (int n = 0; n < 4; n++) {
        p[n].rgb.r = rgb.r;
        p[n].rgb.g = rgb.g;
        p[n].rgb.b = rgb.b;
    }

    int offset = i * VERTEX_ELEMENTS;
    memcpy(vertex_buffer + offset, p, VERTEX_ELEMENTS * sizeof(float));
    pixel_count++;
    repack();
}

void pixel_destroy(pixel_t *pixel) {
    if (pixel == NULL || pixel->index == -1) {
        return;
    }
    pixel->index = -1;
    pixel_count--;
    repack();
}


int main() {
    pixel_count = 0;
    zoom = 100.0f;
    gravity = 1.0f;
    w_width = W_WIDTH;
    w_height = W_HEIGHT;

    if (!glfwInit()) {
        printf("Could not initialize GLFW\n");
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow *window = glfwCreateWindow(w_width, w_height, "sand", NULL,
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
    glfwSetFramebufferSizeCallback(window, resize_callback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);// v-sync
    glewExperimental = GL_TRUE;
    glewInit();
    glDisable(GL_DEPTH_TEST);
    set_aspect(w_width, w_height);
    // shader
    char *vs = "#version 330 core\n"
               "layout (location = 0) in vec2 in_Position;\n"
               "layout (location = 1) in vec3 in_Color;\n"
               "uniform mat4 mvp;\n"
               "out vec3 vertexColor;\n"
               "void main()\n"
               "{\n"
               "    gl_Position = mvp * vec4(in_Position, 0.0f, 1.0f);\n"
               "    vertexColor = in_Color;\n"
               "}\n\0";
    char *fs = "#version 330 core\n"
               "out vec4 FragColor;\n"
               "in vec3 vertexColor;\n"
               "void main()\n"
               "{\n"
               "    FragColor = vec4(vertexColor, 1.0f);\n"
               "}\n\0";
    GLuint program = shader_program_create_s(vs, fs);
    shader_program_bind_attribute_location(program, 0, "in_Position");
    shader_program_bind_attribute_location(program, 1, "in_Color");
    shader_program_link(program);
    GLint mvp_uniform = shader_program_get_uniform_location(program, "mvp");
    // gl
    GLuint vao, vbo, ebo = 0;
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
    GLuint position_size = 2;
    glVertexAttribPointer(0, position_size, GL_FLOAT, GL_FALSE,
                          VERTEX_STRIDE * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    // rgb attribute pointer
    GLuint rgb_size = 3;
    glVertexAttribPointer(1, rgb_size, GL_FLOAT, GL_FALSE,
                          VERTEX_STRIDE * sizeof(float),
                          (void *) (position_size * sizeof(float)));
    glEnableVertexAttribArray(1);
    // pixels
    pixels = malloc(MAX_PIXELS * sizeof(pixel_t *));
    checkm(pixels);
    for (int x = 0; x < MAX_PIXELS; x++) {
        pixels[x] = malloc(sizeof(pixel_t));
        checkm(pixels[x]);
        pixels[x]->index = -1;
        pixels[x]->update = update;
    }

    // game loop
    double delta;
    double start_time;
    double previous_time = glfwGetTime();
    int frame_count = 0;
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    pixel_add(0.0f, 0.0f, SAND);
    //pixel_add(1.0f, 1.0f, SAND);
    pixel_add(1.0f, 1.0f, SAND);

    while (!should_close) {
        start_time = glfwGetTime();
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.169f, 0.169f, 0.169f, 1.0f);
        glUseProgram(program);
        glUniformMatrix4fv(mvp_uniform, 1, GL_FALSE, (const GLfloat *) mvp);
        set_aspect(w_width, w_height);

        //repack();
        for (int x = 0; x < pixel_count; x++) {
            if (pixels[x]->index != -1) {
                pixels[x]->update(pixels[x]);
            }
        }

        glBufferSubData(GL_ARRAY_BUFFER, 0,
                        pixel_count * sizeof(float) * VERTEX_ELEMENTS,
                        vertex_buffer);
        glDrawElements(GL_TRIANGLES, pixel_count * 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
        delta = glfwGetTime() - start_time;
        frame_count++;
        if (start_time - previous_time >= 1.0) {
            printf("frame: %.2f, fps: %d, pixels: %d\n", delta * 1000,
                   frame_count, pixel_count);
            previous_time = start_time;
            frame_count = 0;
        }
    }

    glfwTerminate();
    return 0;
}
