#pragma once

#include <memory>

#include "GLFW/glfw3.h"
#include "Delegate.h"

#include <optional>
#include <string>

typedef void(*CinGLProc)();


class Window {
public:
    Window() = default;
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    Window(Window&& w) noexcept;
    Window& operator=(Window&& w) noexcept;
    ~Window();


    void initialize();
    bool shouldClose();
    bool isMinimized() const;
    void close();

    void swapBuffers() const;

    Delegate<> OnWindowClosed;

    Delegate<int, int> OnWindowResized{};

    // False if window was restored
    Delegate<uint8_t> OnWindowMinimized{};

    // False if window lost focus
    Delegate<uint8_t> OnWindowFocused{};

    // False if cursor exit
    Delegate<uint8_t> OnCursorEnter{};



    static void InitializeBackend();
    static void CleanupBackend();

    static std::unique_ptr<Window> Create(int w, int h, const std::string &title);
    static void SetRenderContext(const Window& window);
    // TODO: Maybe go in renderer instead?
    static void SetSwapInterval(bool vsync);
    static CinGLProc (*GetGLProcAddrFunction())(const char *procname);

    const void* getNativeWindow() const {return static_cast<void*>(m_glfwWindow);}
    std::pair<int, int> getWindowSize() const {return {m_width, m_height};}
    std::string_view getTitle() const { return m_title;};


    // Helpers since glfw handles all windows for noww
    GLFWwindow* getGlfwWindow() const {return m_glfwWindow;}

private:
    GLFWwindow* m_glfwWindow{nullptr};
    int m_width{}, m_height{};
    std::string m_title{};

    bool m_minimized{};

    static void glfwWindowResizeCallback(GLFWwindow* window, int width, int height);
    static void glfwWindowIconifyCallback(GLFWwindow* window, int iconified);
    static void glfwWindowFocusCallback(GLFWwindow * window, int focused);
    static void glfwCursorEnterCallback(GLFWwindow * window, int entered);
    static void glfwWindowCloseCallback(GLFWwindow * window);

    static Window* getWindowFromGlfwWindow(GLFWwindow* win);
};


