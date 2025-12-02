#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <vector>
#include <cmath>

// ----------  омпил€ци€ шейдера ----------
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

// ---------- —оздание программы ----------
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

// -------------------------------------------------------------
// 1) Ўейдер дл€ плоского закрашивани€ (цвет пришит константой)
// -------------------------------------------------------------
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
    fragColor = vec4(0.1, 0.7, 0.9, 1.0); //  ќЌ—“јЌ“ј
}
)";

// -------------------------------------------------------------
// 2) ѕлоское закрашивание через uniform
// -------------------------------------------------------------
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

// -------------------------------------------------------------
// 3) √радиент (между цветов вершин)
// -------------------------------------------------------------
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

// ---------- —оздание правильного N-угольника ----------
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

int main() {
    sf::Window window(sf::VideoMode({ 800, 600 }), "SFML + OpenGL");
    window.setActive(true);

    glewInit();

    // ---- ‘игуры ----
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

    // ---- Ўейдеры ----
    GLuint progFlatConst = createProgram(vs_flat_const, fs_flat_const);
    GLuint progFlatUniform = createProgram(vs_flat_uniform, fs_flat_uniform);
    GLuint progGrad = createProgram(vs_grad, fs_grad);

    // ---- √лавный цикл ----
    while (window.isOpen()) {
        //sf::Event e;
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { window.close(); }
        }
        glClearColor(0.1f, 0.1f, 0.15f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        // -------------------------------
        // 1) „етырЄхугольник Ч плоский (CONST)
        // -------------------------------
        glUseProgram(progFlatConst);

        GLuint vao1, vbo1;
        glGenVertexArrays(1, &vao1);
        glGenBuffers(1, &vbo1);

        glBindVertexArray(vao1);
        glBindBuffer(GL_ARRAY_BUFFER, vbo1);
        glBufferData(GL_ARRAY_BUFFER, quad.size() * sizeof(float), quad.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDeleteBuffers(1, &vbo1);
        glDeleteVertexArrays(1, &vao1);

        // -------------------------------
        // 2) ¬еер Ч плоский через UNIFORM
        // -------------------------------
        glUseProgram(progFlatUniform);

        GLuint colorLoc = glGetUniformLocation(progFlatUniform, "uColor");
        glUniform4f(colorLoc, 1.0, 0.3, 0.2, 1.0);

        GLuint vao2, vbo2;
        glGenVertexArrays(1, &vao2);
        glGenBuffers(1, &vbo2);

        glBindVertexArray(vao2);
        glBindBuffer(GL_ARRAY_BUFFER, vbo2);
        glBufferData(GL_ARRAY_BUFFER, fan.size() * sizeof(float), fan.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glDrawArrays(GL_TRIANGLE_FAN, 0, fan.size() / 2);

        glDeleteBuffers(1, &vbo2);
        glDeleteVertexArrays(1, &vao2);

        // -------------------------------
        // 3) ѕ€тиугольник Ч градиент
        // -------------------------------
        glUseProgram(progGrad);

        // создаЄм массив позиций + цветов
        std::vector<float> data;
        for (int i = 0; i < pent.size() / 2; i++) {
            float x = pent[i * 2];
            float y = pent[i * 2 + 1];
            data.push_back(x);
            data.push_back(y);

            // цвет вершины Ч по индексу
            data.push_back((float)i / 5.f);
            data.push_back(1.f - (float)i / 5.f);
            data.push_back(0.3f + 0.1f * i);
        }

        GLuint vao3, vbo3;
        glGenVertexArrays(1, &vao3);
        glGenBuffers(1, &vbo3);

        glBindVertexArray(vao3);
        glBindBuffer(GL_ARRAY_BUFFER, vbo3);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLE_FAN, 0, pent.size() / 2);

        glDeleteBuffers(1, &vbo3);
        glDeleteVertexArrays(1, &vao3);


        // -------------------------------
        window.display();
    }

    return 0;
}
