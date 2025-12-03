#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>

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

const char* vs_flat_const = R"(
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
    fragColor = vec4(0.1, 0.7, 0.9, 1.0); // КОНСТАНТА
}
)";

const char* vs_flat_uniform = R"(
#version 330 core
layout(location=0) in vec2 position;

void main(){
    gl_Position = vec4(position, 0.0, 1.0);
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
layout(location=1) in vec3 vColor;

out vec3 color;

void main(){
    gl_Position = vec4(position, 0.0, 1.0);
    color = vColor;
}
)";

const char* fs_grad = R"(
#version 330 core
in vec3 color;
out vec4 fragColor;

void main(){
    fragColor = vec4(color, 1.0);
}
)";

std::vector<float> makePolygon(int n) {
    std::vector<float> pts;
    float r = 0.5f;

    for (int i = 0; i < n; i++) {
        float a = (2 * M_PI * i) / n;
        pts.push_back(r * cos(a));
        pts.push_back(r * sin(a));
    }
    return pts;
}

void drawFigureWithSeparateColors(GLuint program, const std::vector<float>& positions,
    const std::vector<float>& colors) {

    GLuint vao, vboPositions, vboColors;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboPositions);
    glGenBuffers(1, &vboColors);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vboColors);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLE_FAN, 0, positions.size() / 2);

    glDeleteBuffers(1, &vboPositions);
    glDeleteBuffers(1, &vboColors);
    glDeleteVertexArrays(1, &vao);
}

void drawFigureWithoutColors(GLuint program, const std::vector<float>& positions,
    const std::array<float, 4>& uniformColor = { 1.0f, 1.0f, 1.0f, 1.0f }) {

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    GLuint colorLoc = glGetUniformLocation(program, "uColor");
    if (colorLoc != -1) {
        glUniform4f(colorLoc, uniformColor[0], uniformColor[1], uniformColor[2], uniformColor[3]);
    }

    glDrawArrays(GL_TRIANGLE_FAN, 0, positions.size() / 2);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

int main() {
    sf::Window window(sf::VideoMode({ 800, 600 }), "SFML + OpenGL - Figure Control (1-3: view single, 4: view all, F1-F3: shading)");
    window.setActive(true);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW Error: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "=== FIGURE VIEWING MODE ===\n";
    std::cout << "View modes:\n";
    std::cout << "  1 - Show Quadrilateral only\n";
    std::cout << "  2 - Show Fan only\n";
    std::cout << "  3 - Show Pentagon only\n";
    std::cout << "  4 - Show All figures\n";
    std::cout << "\nShading modes:\n";
    std::cout << "  F1 - Flat shading (constant in shader)\n";
    std::cout << "  F2 - Flat shading (uniform from program)\n";
    std::cout << "  F3 - Gradient shading\n";
    std::cout << "\nEscape - Exit\n";

    std::vector<float> quad = {
        -0.6f, -0.4f,
         0.6f, -0.4f,
         0.6f,  0.4f,
        -0.6f,  0.4f
    };

    std::vector<float> fan = {
        0.f, 0.f,
        0.7f, 0.f,
        0.5f, 0.5f,
        0.f, 0.7f,
        -0.5f, 0.5f,
        -0.7f, 0.f
    };

    std::vector<float> pent = makePolygon(5);

    GLuint progFlatConst = createProgram(vs_flat_const, fs_flat_const);
    GLuint progFlatUniform = createProgram(vs_flat_uniform, fs_flat_uniform);
    GLuint progGrad = createProgram(vs_grad, fs_grad);

    std::vector<float> quadColors = {
        1.0f, 0.0f, 0.0f,  // Красный
        0.0f, 1.0f, 0.0f,  // Зеленый
        0.0f, 0.0f, 1.0f,  // Синий
        1.0f, 1.0f, 0.0f   // Желтый
    };

    std::vector<float> fanColors = {
        1.0f, 1.0f, 1.0f,  // Центр - белый
        1.0f, 0.0f, 0.0f,  // Красный
        1.0f, 0.5f, 0.0f,  // Оранжевый
        1.0f, 1.0f, 0.0f,  // Желтый
        0.0f, 1.0f, 0.0f,  // Зеленый
        0.0f, 0.0f, 1.0f   // Синий
    };

    std::vector<float> pentColors;
    for (int i = 0; i < pent.size() / 2; i++) {
        pentColors.push_back((float)i / 5.f);
        pentColors.push_back(1.f - (float)i / 5.f);
        pentColors.push_back(0.3f + 0.1f * i);
    }

    enum ViewMode { VIEW_ALL = 0, VIEW_QUAD, VIEW_FAN, VIEW_PENT };
    enum ShaderType { SHADER_FLAT_CONST = 0, SHADER_FLAT_UNIFORM, SHADER_GRADIENT };

    ViewMode currentView = VIEW_ALL;
    ShaderType currentShader = SHADER_FLAT_CONST;
    bool showFigures = true;

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
                    std::cout << "View: Quadrilateral only\n";
                }
                else if (keyEvent->code == sf::Keyboard::Key::Num2) {
                    currentView = VIEW_FAN;
                    std::cout << "View: Fan only\n";
                }
                else if (keyEvent->code == sf::Keyboard::Key::Num3) {
                    currentView = VIEW_PENT;
                    std::cout << "View: Pentagon only\n";
                }
                else if (keyEvent->code == sf::Keyboard::Key::Num4) {
                    currentView = VIEW_ALL;
                    std::cout << "View: All figures\n";
                }
                else if (keyEvent->code == sf::Keyboard::Key::F1) {
                    currentShader = SHADER_FLAT_CONST;
                    std::cout << "Shading: Flat (constant in shader)\n";
                }
                else if (keyEvent->code == sf::Keyboard::Key::F2) {
                    currentShader = SHADER_FLAT_UNIFORM;
                    std::cout << "Shading: Flat (uniform from program)\n";
                }
                else if (keyEvent->code == sf::Keyboard::Key::F3) {
                    currentShader = SHADER_GRADIENT;
                    std::cout << "Shading: Gradient\n";
                }
            }
        }

        window.setActive(true);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (showFigures) {

            if (currentView == VIEW_ALL || currentView == VIEW_QUAD) {
                std::array<float, 4> quadUniformColor = { 0.1f, 0.7f, 0.9f, 1.0f }; // Как в исходном примере

                if (currentShader == SHADER_FLAT_CONST) {
                    glUseProgram(progFlatConst);
                    drawFigureWithoutColors(progFlatConst, quad);
                }
                else if (currentShader == SHADER_FLAT_UNIFORM) {
                    glUseProgram(progFlatUniform);
                    drawFigureWithoutColors(progFlatUniform, quad, quadUniformColor);
                }
                else if (currentShader == SHADER_GRADIENT) {
                    glUseProgram(progGrad);
                    drawFigureWithSeparateColors(progGrad, quad, quadColors);
                }
            }

            if (currentView == VIEW_ALL || currentView == VIEW_FAN) {
                std::array<float, 4> fanUniformColor = { 1.0f, 0.3f, 0.2f, 1.0f }; // Как в исходном примере

                if (currentShader == SHADER_FLAT_CONST) {
                    glUseProgram(progFlatConst);
                    drawFigureWithoutColors(progFlatConst, fan);
                }
                else if (currentShader == SHADER_FLAT_UNIFORM) {
                    glUseProgram(progFlatUniform);
                    drawFigureWithoutColors(progFlatUniform, fan, fanUniformColor);
                }
                else if (currentShader == SHADER_GRADIENT) {
                    glUseProgram(progGrad);
                    drawFigureWithSeparateColors(progGrad, fan, fanColors);
                }
            }

            if (currentView == VIEW_ALL || currentView == VIEW_PENT) {
                std::array<float, 4> pentUniformColor = { 0.0f, 0.0f, 1.0f, 1.0f }; // Синий для пятиугольника

                if (currentShader == SHADER_FLAT_CONST) {
                    glUseProgram(progFlatConst);
                    drawFigureWithoutColors(progFlatConst, pent);
                }
                else if (currentShader == SHADER_FLAT_UNIFORM) {
                    glUseProgram(progFlatUniform);
                    drawFigureWithoutColors(progFlatUniform, pent, pentUniformColor);
                }
                else if (currentShader == SHADER_GRADIENT) {
                    glUseProgram(progGrad);
                    drawFigureWithSeparateColors(progGrad, pent, pentColors);
                }
            }
        }

        window.display();
    }

    glDeleteProgram(progFlatConst);
    glDeleteProgram(progFlatUniform);
    glDeleteProgram(progGrad);

    return 0;
}
