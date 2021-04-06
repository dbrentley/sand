#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "linmath.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define W_WIDTH 1920
#define W_HEIGHT 1080
#define MAX_PIXELS (1920 * 1080)
#define VERTEX_ELEMENTS 20
#define VERTEX_STRIDE 5

typedef struct pixel_t pixel_t;

typedef void (*update_f)(pixel_t *);

typedef enum {
    SAND, WATER
} pixel_type_e;

typedef enum {
    N, E, S, W, NE, NW, SE, SW
} pixel_direction_e;

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
    float friction;
    float life_time;
    update_f update;
    pixel_type_e type;
    int grid_x;
    int grid_y;
};

typedef struct {
    pos_t pos;
    rgb_t rgb;
} pixel_vertex_t;

bool should_close = false;
bool mouse_left_down = false;
bool mouse_right_down = false;
double mouse_x;
double mouse_y;
float zoom;
float *vertex_buffer;
float gravity;
float scale;
int pixel_count;
int w_width, w_height;
int grid[W_WIDTH * W_HEIGHT];
pixel_t **pixels;
mat4x4 mvp;


void pixel_add(float x, float y, pixel_type_e type);

float float_rand(float min, float max) {
    float s = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + s * (max - min);        /* [min, max] */
}

void window_close_callback(GLFWwindow *w) {
    should_close = true;
}

void set_aspect(int width, int height) {
    float aspect = (float) width / (float) height;
    glViewport(0, 0, W_WIDTH, W_HEIGHT);
    gluOrtho2D(0.0f, (float) W_WIDTH, (float) W_HEIGHT, 0.0f);
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
    //    if (y_offset == -1.0f) {
    //        if (zoom < 100.0f) { zoom += 5.0f; }
    //    } else {
    //        if (zoom > 5.0f) { zoom -= 5.0f; }
    //    }
}

void mouse_button_callback(GLFWwindow *w, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        mouse_right_down = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        mouse_right_down = false;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouse_left_down = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mouse_left_down = false;
    }
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

void grid_init() {
    for (int x = 0; x < W_WIDTH * W_HEIGHT; x++) {
        grid[x] = -1;
    }
}

void update(pixel_t *pixel) {
    bool can_move = false;
    pixel_direction_e dir = S;
    int pixel_x = pixel->grid_x;
    int pixel_y = pixel->grid_y;
    int grid_position = pixel_x + pixel_y * W_WIDTH;
    int mass = (int) pixel->mass;
    int friction = (int) pixel->friction;
    int distance_s = 0;
    int distance_w = 0;
    int distance_e = 0;
    int distance_se = 0;
    int distance_sw = 0;

    int dir_e;
    int dir_se;
    int dir_s;
    int dir_sw;
    int dir_w;

    for (int m = 1; m < mass; m++) {
        dir_s = pixel_x + (pixel_y + m) * W_WIDTH;
        if (grid[dir_s] != -1) {
            break;
        }
        distance_s++;
    }
    for (int m = 1; m < mass; m++) {
        dir_w = (pixel_x - m) + pixel_y * W_WIDTH;
        if (grid[dir_w] != -1) {
            break;
        }
        distance_w++;
    }
    for (int m = 1; m < mass; m++) {
        dir_e = (pixel_x + m) + pixel_y * W_WIDTH;
        if (grid[dir_e] != -1) {
            break;
        }
        distance_e++;
    }
    for (int m = 1; m < friction; m++) {
        dir_sw = (pixel_x - m) + (pixel_y + m) * W_WIDTH;
        if (grid[dir_sw] != -1) {
            break;
        }
        distance_sw++;
    }
    for (int m = 1; m < friction; m++) {
        dir_se = (pixel_x + m) + (pixel_y + m) * W_WIDTH;
        if (grid[dir_se] != -1) {
            break;
        }
        distance_se++;
    }

    dir_s = (pixel_x) + (pixel_y + 1) * W_WIDTH;
    dir_sw = (pixel_x - 1) + (pixel_y + 1) * W_WIDTH;
    dir_se = (pixel_x + 1) + (pixel_y + 1) * W_WIDTH;
    dir_w = (pixel_x - 1) + pixel_y * W_WIDTH;
    dir_e = (pixel_x + 1) + pixel_y * W_WIDTH;

    switch (pixel->type) {
        case SAND:
            if (grid[dir_s] == -1) {
                can_move = true;
                dir = S;
            } else if (grid[dir_sw] == -1) {
                can_move = true;
                dir = SW;
            } else if (grid[dir_se] == -1) {
                can_move = true;
                dir = SE;
            }
            break;
        case WATER:
            if (grid[dir_s] == -1) {
                can_move = true;
                dir = S;
            } else if (grid[dir_sw] == -1) {
                can_move = true;
                dir = SW;
            } else if (grid[dir_se] == -1) {
                can_move = true;
                dir = SE;
            } else if (grid[dir_w] == -1) {
                can_move = true;
                dir = W;
            } else if (grid[dir_e] == -1) {
                can_move = true;
                dir = E;
            }
            break;
    }

    if (!can_move) {
        return;
    }

    switch (dir) {
        case S:
            pixel->pos.y += scale * (float) distance_s;
            pixel->grid_y += (int) (scale * (float) distance_s);
            break;
        case W:
            pixel->pos.x -= scale * (float) distance_w;
            pixel->grid_x -= (int) (scale * (float) distance_w);
            break;
        case E:
            pixel->pos.x += scale * (float) distance_e;
            pixel->grid_x += (int) (scale * (float) distance_e);
            break;
        case SW:
            pixel->pos.x -= scale * (float) distance_sw;
            pixel->pos.y += scale * (float) distance_sw;
            pixel->grid_x -= (int) (scale * (float) distance_sw);
            pixel->grid_y += (int) (scale * (float) distance_sw);
            break;
        case SE:
            pixel->pos.x += scale * (float) distance_se;
            pixel->pos.y += scale * (float) distance_se;
            pixel->grid_x += (int) (scale * (float) distance_se);
            pixel->grid_y += (int) (scale * (float) distance_se);
            break;
        default:
            break;
    }

    if (pixel->pos.y >= W_HEIGHT - 1) {
        pixel->pos.y = W_HEIGHT - 1;
        pixel->grid_y = W_HEIGHT - 1;
    }
    if (pixel->pos.x >= W_WIDTH - 1) {
        pixel->pos.x = W_WIDTH - 1;
        pixel->grid_x = W_WIDTH - 1;
    }

    pixel_x = (int) pixel->grid_x;
    pixel_y = (int) pixel->grid_y;
    int new_position = pixel_x + pixel_y * W_WIDTH;
    grid[new_position] = pixel->index;
    grid[grid_position] = -1;

    float v[8];
    v[0] = pixel->pos.x - scale;
    v[1] = pixel->pos.y - scale;
    v[2] = pixel->pos.x + scale;
    v[3] = pixel->pos.y - scale;
    v[4] = pixel->pos.x + scale;
    v[5] = pixel->pos.y + scale;
    v[6] = pixel->pos.x - scale;
    v[7] = pixel->pos.y + scale;

    int cnt = 0;
    int position_size = 2;
    int offset = pixel->index * VERTEX_ELEMENTS;
    for (int y = 0; y < VERTEX_ELEMENTS; y += VERTEX_STRIDE) {
        float m[position_size];
        for (int x = 0; x < position_size; x++) { m[x] = v[cnt + x]; }
        memcpy(vertex_buffer + offset + y, m, position_size * sizeof(float));
        cnt += position_size;
    }
}

void checkm(void *obj) {
    if (obj == NULL) {
        printf("Could not allocate memory for object\n");
        exit(-1);
    }
}

void repack() {
    return; // not used for now
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
    if (pixel_count >= MAX_PIXELS - 10) {
        return;
    }
    int i;
    for (i = 0; i < MAX_PIXELS; i++) {
        if (pixels[i]->index == -1) { break; }
    }

    if (x < 1) {
        x = 1;
    }
    if (x > W_WIDTH) {
        x = W_WIDTH;
    }
    if (y < 1) {
        y = 1;
    }
    if (y > W_HEIGHT - 1) {
        y = W_HEIGHT - 1;
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
        case WATER:
            rgb.r = 0.0f;
            rgb.g = 0.1f;
            rgb.b = 1.0f;
        default:
            break;
    }

    pixels[i]->index = i;
    pixels[i]->pos = pos;
    pixels[i]->rgb = rgb;
    pixels[i]->type = type;
    pixels[i]->mass = 1.0f;
    pixels[i]->life_time = 0;
    pixels[i]->grid_x = (int) x;
    pixels[i]->grid_y = (int) y;

    switch (type) {
        case SAND:
            pixels[i]->mass = 10;
            pixels[i]->friction = 2.0f;
            break;
        case WATER:
            pixels[i]->mass = 10;
            pixels[i]->friction = 1.0f;
            break;
        default:
            pixels[i]->mass = 1.0f;
            pixels[i]->friction = 1.0f;
            break;
    }

    pixel_vertex_t p[4];
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
    int pixel_x = pixels[i]->grid_x;
    int pixel_y = pixels[i]->grid_y;
    int grid_position = pixel_x + pixel_y * W_WIDTH;
    grid[grid_position] = i;
    pixel_count++;
    repack();
}

void pixel_destroy(pixel_t *pixel) {
    if (pixel == NULL || pixel->index == -1) {
        return;
    }
    pixel->index = -1;
    int x = pixel->grid_x;
    int y = pixel->grid_y;
    int grid_position = x + y * W_WIDTH;
    grid[grid_position] = -1;
    pixel_count--;
    repack();
}

int main() {
    pixel_count = 0;
    zoom = 540.0f;
    scale = 1.0f;
    gravity = 1.0f;
    w_width = W_WIDTH;
    w_height = W_HEIGHT;
    grid_init();

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

    while (!should_close) {
        start_time = glfwGetTime();
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.169f, 0.169f, 0.169f, 1.0f);
        glUseProgram(program);
        glUniformMatrix4fv(mvp_uniform, 1, GL_FALSE, (const GLfloat *) mvp);
        set_aspect(w_width, w_height);

        if (mouse_left_down) {
            float min = -50.0f;
            float max = 50.0f;
            for (int x = 0; x < 50; x++) {
                pixel_add((float) mouse_x + float_rand(min, max),
                          (float) mouse_y + float_rand(min, max), SAND);
            }
        }
        if (mouse_right_down) {
            float min = -50.0f;
            float max = 50.0f;
            for (int x = 0; x < 50; x++) {
                pixel_add((float) mouse_x + float_rand(min, max),
                          (float) mouse_y + float_rand(min, max), WATER);
            }
        }

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
            printf("frame: %.2f, fps: %d, pixels: %d/%d, mouse: %f, %f\n",
                   delta * 1000, frame_count, pixel_count, MAX_PIXELS, mouse_x,
                   mouse_y);
            previous_time = start_time;
            frame_count = 0;
        }
    }

    glfwTerminate();
    return 0;
}
