#include "Window.h"

#include <iostream>
#include <cassert>

#include "Renderer.h"


Window::Window(Window &&w) noexcept:
    m_glfwWindow(w.m_glfwWindow),
    m_width(w.m_width), m_height(w.m_height),
    m_title(std::move(w.m_title))
{
    glfwSetWindowUserPointer(m_glfwWindow, this);

    w.m_glfwWindow = nullptr;
    w.m_width = 0;
    w.m_height = 0;
}

Window & Window::operator=(Window &&w) noexcept {
    if (this != &w) {
        if (m_glfwWindow) glfwDestroyWindow(m_glfwWindow);
        m_glfwWindow = w.m_glfwWindow;
        m_width = w.m_width;
        m_height =  w.m_height;
        m_title = std::move(w.m_title);
        glfwSetWindowUserPointer(m_glfwWindow, this);

        w.m_glfwWindow = nullptr;
        w.m_width = 0;
        w.m_height = 0;
    }
    return *this;
}

Window::~Window() {
    if (m_glfwWindow) glfwDestroyWindow(m_glfwWindow);
}

void Window::initialize() {
}

bool Window::shouldClose() {
    return glfwWindowShouldClose(m_glfwWindow);
}

bool Window::isMinimized() const {
    return m_minimized;
}

void Window::close() {
    glfwSetWindowShouldClose(m_glfwWindow, GLFW_TRUE);
}

void Window::swapBuffers() const {
    glfwSwapBuffers(m_glfwWindow);
}

void Window::InitializeBackend() {
    // initialize glfw and give window hints for the version and the opengl profile we want to use
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

}

void Window::CleanupBackend() {
    glfwTerminate();
}

void Window::SetRenderContext(const Window &window) {
    glfwMakeContextCurrent(window.m_glfwWindow);
}

void Window::SetSwapInterval(bool vsync) {
    glfwSwapInterval(vsync);
}

CinGLProc (*Window::GetGLProcAddrFunction())(const char *procname) {
    return glfwGetProcAddress;
}

void Window::glfwWindowResizeCallback(GLFWwindow *window, int width, int height) {

    auto* resizedWindow = getWindowFromGlfwWindow(window);
    resizedWindow->OnWindowResized.broadcast(width, height);
}

void Window::glfwWindowIconifyCallback(GLFWwindow *window, int iconified) {

    auto* windowUser = getWindowFromGlfwWindow(window);

    if (iconified == GLFW_TRUE) {
        windowUser->m_minimized = true;
        windowUser->OnWindowMinimized.broadcast(true);
    }
    else {
        windowUser->m_minimized = false;
        windowUser->OnWindowMinimized.broadcast(false);
    }
}

void Window::glfwWindowFocusCallback(GLFWwindow *window, int focused) {
    auto* winUser = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (focused == GLFW_TRUE) {
        winUser->OnWindowFocused.broadcast(true);
    }
    else {
        winUser->OnWindowFocused.broadcast(false);
    }
}

void Window::glfwCursorEnterCallback(GLFWwindow *window, int entered) {
    auto* winUser = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (entered == GLFW_TRUE) {
        winUser->OnCursorEnter.broadcast(true);
    }
    else {
        winUser->OnCursorEnter.broadcast(false);
    }
}

void Window::glfwWindowCloseCallback(GLFWwindow *window) {
    auto* winUser = static_cast<Window*>(glfwGetWindowUserPointer(window));

    winUser->OnWindowClosed.broadcast();
}

Window * Window::getWindowFromGlfwWindow(GLFWwindow *win) {

    auto* outWindow = static_cast<Window*>(glfwGetWindowUserPointer(win));
    assert(outWindow && "WINDOW: GLFW window contains an invalid window user.\n");

    return outWindow;
}

std::unique_ptr<Window> Window::Create(int w, int h, const std::string &title) {

    GLFWwindow* outGlfwWindow = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);

    if (!outGlfwWindow) {
        std::cerr << "WINDOW: Failed to create glfw window instance.\n";
        return nullptr;
    }

    auto outWindow = std::make_unique<Window>();
    outWindow->m_glfwWindow = outGlfwWindow;
    outWindow->m_width = w;
    outWindow->m_height = h;
    outWindow->m_title = title;

    glfwSetFramebufferSizeCallback(outWindow->m_glfwWindow, glfwWindowResizeCallback);
    glfwSetWindowIconifyCallback(outWindow->m_glfwWindow, glfwWindowIconifyCallback);
    glfwSetWindowFocusCallback(outWindow->m_glfwWindow, glfwWindowFocusCallback);
    glfwSetCursorEnterCallback(outWindow->m_glfwWindow, glfwCursorEnterCallback);
    glfwSetWindowCloseCallback(outWindow->m_glfwWindow, glfwWindowCloseCallback);

    // Technically just gets destroyed but move operations will change user pointer properly
    glfwSetWindowUserPointer(outWindow->m_glfwWindow, outWindow.get());

    return std::move(outWindow);
}
