#pragma once

#include "../Definition.h"
#include "Color.h"
#include "Camera.h"
#include "../PhysicsRules.h"

class Renderer {
public:
    static Renderer& GetInstance();

    bool IsRendering() const;

    void InstantiateWindow(const std::string& title, const Integer& width = 960, const Integer& height = 680);

    void LoadScreen() const;

    void ClearScreen() const;

    void SetBackground(const Color& color);

    inline const Integer& GetScreenWidth() const { return m_screenWidth; }
    inline const Integer& GetScreenHeight() const { return m_screenHeight; }
    inline void SetMouseCallback(GLFWmousebuttonfun callback) { glfwSetMouseButtonCallback(m_window, callback); }
    inline void SetKeyboardCallback(GLFWkeyfun callback) { glfwSetKeyCallback(m_window, callback); }

    template <typename T>
	inline void SetUserPointer(T& obj) { glfwSetWindowUserPointer(m_window, &obj); }

    void RenderPoints(const std::vector<Vector3>& points, const Matrix4& projMatrix = Matrix4::Identity(), const Scalar& pointSize = 5.0, const Color& color = Color::Gray()) const;

    void RenderLines(const std::vector<Vector3>& points, const std::vector<Integer>& indices, const Matrix4& projMatrix = Matrix4::Identity(), const Scalar& lineWidth = 5.0, const Color& color = Color::Gray()) const;

    void RenderParticles(const std::vector<Particle<3>>& points, const Matrix4& projMatrix = Matrix4::Identity(), const Scalar& pointSize = 5.0, const Color& color = Color::Gray()) const;

private:
    static std::string ReadShaderSource(const char* filePath);

    static void PrintShaderLog(GLuint shader);

    static void PrintProgramLog(GLuint program);

    static bool CheckOpenGLError();

    static GLuint CreateShader(const char* filePath, GLenum shaderType);

    static GLuint CreateShaderProgram(const char* vp, const char* fp);

private:
    Renderer(Integer screenWidth = 800, Integer screenHeight = 600, const std::string& screenTitle = "DEMO Screen");
    ~Renderer();

    Renderer(const Renderer& rend) = delete;
    const Renderer& operator=(const Renderer& rend) = delete;

private:
    Integer m_screenWidth;
    Integer m_screenHeight;
    GLFWwindow* m_window;
    Color m_backgroundColor;
};