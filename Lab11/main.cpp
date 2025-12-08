#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <vector>
#include <cmath>

GLuint compileShader(GLenum type, const char* src) {
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint success;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(sh, 512, nullptr, log);
        std::cerr << "Shader compile error:\n" << log << std::endl;
    }
    return sh;
}

GLuint createProgram(const char* vs, const char* fs) {
    GLuint v = compileShader(GL_VERTEX_SHADER, vs);
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, v);
    glAttachShader(prog, f);
    glLinkProgram(prog);
    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(prog, 512, nullptr, log);
        std::cerr << "Program link error:\n" << log << std::endl;
    }
    glDeleteShader(v);
    glDeleteShader(f);
    return prog;
}

const char* vs_flat = R"(
#version 330 core
layout(location=0) in vec2 position;
void main(){
    gl_Position = vec4(position, 0.0, 1.0);
}
)";

const char* fs_flat_const = R"(
#version 330 core
out vec4 fragColor;
void main(){
    fragColor = vec4(0.9, 0.2, 0.2, 1.0);
}
)";

const char* fs_flat_uniform = R"(
#version 330 core
out vec4 fragColor;
uniform vec4 uColor;
void main(){
    fragColor = uColor;
}
)";

const char* vs_grad = R"(
#version 330 core
layout(location=0) in vec2 position;
layout(location=1) in vec3 color;
out vec3 fragColor;
void main(){
    gl_Position = vec4(position, 0.0, 1.0);
    fragColor = color;
}
)";

const char* fs_grad = R"(
#version 330 core
in vec3 fragColor;
out vec4 outColor;
void main(){
    outColor = vec4(fragColor, 1.0);
}
)";

int main() {
    sf::Window window(sf::VideoMode({ 800, 600 }), "OpenGL - 3 Figures, 3 Shading Types");
    if (!window.setActive(true)) {
        std::cerr << "Failed to activate OpenGL context!" << std::endl;
        return -1;
    }
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW Error: " << glewGetErrorString(err) << std::endl;
        return -1;
    }
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Controls:\n";
    std::cout << "1: Quadrilateral\n2: Fan\n3: Pentagon\n";
    std::cout << "F1: Flat (constant - RED)\nF2: Flat (uniform - different colors)\nF3: Gradient\n";
    std::cout << "ESC: Exit\n";
    
    std::vector<float> quad = {
        -0.6f, -0.4f, 0.6f, -0.4f, 0.6f, 0.4f,
        -0.6f, -0.4f, 0.6f, 0.4f, -0.6f, 0.4f
    };
    
    std::vector<float> fan = {
        0.0f, 0.0f, 0.7f, 0.0f, 0.5f, 0.5f,
        0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.7f,
        0.0f, 0.0f, 0.0f, 0.7f, -0.5f, 0.5f,
        0.0f, 0.0f, -0.5f, 0.5f, -0.7f, 0.0f,
        0.0f, 0.0f, -0.7f, 0.0f, 0.7f, 0.0f
    };
    
    std::vector<float> pentagon = {
        0.0f, 0.5f, -0.475f, 0.155f, -0.294f, -0.405f,
        0.0f, 0.5f, -0.294f, -0.405f, 0.294f, -0.405f,
        0.0f, 0.5f, 0.294f, -0.405f, 0.475f, 0.155f
    };
    
    std::vector<float> quadColors = {
        1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f
    };
    
    std::vector<float> fanColors = {
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f
    };
    
    std::vector<float> pentColors = {
        1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f
    };
    
    GLuint progFlatConst = createProgram(vs_flat, fs_flat_const);
    GLuint progFlatUniform = createProgram(vs_flat, fs_flat_uniform);
    GLuint progGrad = createProgram(vs_grad, fs_grad);
    
    enum ViewMode { VIEW_QUAD = 1, VIEW_FAN, VIEW_PENT };
    enum ShaderType { SHADER_FLAT_CONST = 0, SHADER_FLAT_UNIFORM, SHADER_GRADIENT };
    
    ViewMode currentView = VIEW_QUAD;
    ShaderType currentShader = SHADER_FLAT_CONST;
    
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            else if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }
                else if (keyEvent->code == sf::Keyboard::Key::Num1) {
                    currentView = VIEW_QUAD;
                }
                else if (keyEvent->code == sf::Keyboard::Key::Num2) {
                    currentView = VIEW_FAN;
                }
                else if (keyEvent->code == sf::Keyboard::Key::Num3) {
                    currentView = VIEW_PENT;
                }
                else if (keyEvent->code == sf::Keyboard::Key::F1) {
                    currentShader = SHADER_FLAT_CONST;
                }
                else if (keyEvent->code == sf::Keyboard::Key::F2) {
                    currentShader = SHADER_FLAT_UNIFORM;
                }
                else if (keyEvent->code == sf::Keyboard::Key::F3) {
                    currentShader = SHADER_GRADIENT;
                }
            }
        }
        if (!window.setActive(true)) {
            break;
        }
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        std::vector<float>* vertices = nullptr;
        std::vector<float>* colors = nullptr;
        int vertexCount = 0;
        
        if (currentView == VIEW_QUAD) {
            vertices = &quad;
            colors = &quadColors;
            vertexCount = 6;
        }
        else if (currentView == VIEW_FAN) {
            vertices = &fan;
            colors = &fanColors;
            vertexCount = 15;
        }
        else if (currentView == VIEW_PENT) {
            vertices = &pentagon;
            colors = &pentColors;
            vertexCount = 9;
        }
        
        if (vertices) {
            if (currentShader == SHADER_FLAT_CONST) {
                glUseProgram(progFlatConst);
                GLuint vao, vbo;
                glGenVertexArrays(1, &vao);
                glGenBuffers(1, &vbo);
                glBindVertexArray(vao);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, vertices->size() * sizeof(float), vertices->data(), GL_STATIC_DRAW);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
                glDrawArrays(GL_TRIANGLES, 0, vertexCount);
                glDeleteBuffers(1, &vbo);
                glDeleteVertexArrays(1, &vao);
            }
            else if (currentShader == SHADER_FLAT_UNIFORM) {
                glUseProgram(progFlatUniform);
                GLuint colorLoc = glGetUniformLocation(progFlatUniform, "uColor");
                GLuint vao, vbo;
                glGenVertexArrays(1, &vao);
                glGenBuffers(1, &vbo);
                glBindVertexArray(vao);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, vertices->size() * sizeof(float), vertices->data(), GL_STATIC_DRAW);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
                if (currentView == VIEW_QUAD) {
                    glUniform4f(colorLoc, 0.1f, 0.7f, 0.9f, 1.0f);
                }
                else if (currentView == VIEW_FAN) {
                    glUniform4f(colorLoc, 0.8f, 0.4f, 0.1f, 1.0f);
                }
                else if (currentView == VIEW_PENT) {
                    glUniform4f(colorLoc, 0.3f, 0.1f, 0.8f, 1.0f);
                }
                glDrawArrays(GL_TRIANGLES, 0, vertexCount);
                glDeleteBuffers(1, &vbo);
                glDeleteVertexArrays(1, &vao);
            }
            else if (currentShader == SHADER_GRADIENT) {
                glUseProgram(progGrad);
                GLuint vao, vboPos, vboCol;
                glGenVertexArrays(1, &vao);
                glGenBuffers(1, &vboPos);
                glGenBuffers(1, &vboCol);
                glBindVertexArray(vao);
                glBindBuffer(GL_ARRAY_BUFFER, vboPos);
                glBufferData(GL_ARRAY_BUFFER, vertices->size() * sizeof(float), vertices->data(), GL_STATIC_DRAW);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, vboCol);
                glBufferData(GL_ARRAY_BUFFER, colors->size() * sizeof(float), colors->data(), GL_STATIC_DRAW);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(1);
                glDrawArrays(GL_TRIANGLES, 0, vertexCount);
                glDeleteBuffers(1, &vboPos);
                glDeleteBuffers(1, &vboCol);
                glDeleteVertexArrays(1, &vao);
            }
        }
        window.display();
    }
    glDeleteProgram(progFlatConst);
    glDeleteProgram(progFlatUniform);
    glDeleteProgram(progGrad);
    return 0;
}
