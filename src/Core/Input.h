#pragma once

#include "KeyCodes.h"
#include "Window.h"

#include <unordered_set>
#include <utility>


class Input {
private:
    static Input* instance;
public:
    Input(Window* captureWindow);

    static Input* GetInstance();

    void update();
    void setCaptureWindow(Window* newWindow);

    static Delegate<Key, KeyMod> OnKeyPressed;
    static Delegate<Key, KeyMod> OnKeyReleased;

    static Delegate<MouseButton, KeyMod> OnMouseButtonPressed;
    static Delegate<MouseButton, KeyMod> OnMouseButtonReleased;

    // Absolute cursor position in window coordinates
    static Delegate<double, double> OnCursorMoved;
    // x/y scroll offsets
    static Delegate<double, double> OnScroll;


private:

    Window* m_capturingWindow{nullptr};

    void beginFrame();

    static bool IsKeyDown(Key k);
    static bool IsModDown(const KeyMod& mod);
    static bool WasKeyPressed(Key k);
    static bool WasKeyReleased(Key k);

    struct Keyboard {
        KeyMod activeMods{};

        bool IsKeyDown(Key k) const;
        bool IsModDown(const KeyMod& mod) const;
        bool WasKeyPressed(Key k) const;
        bool WasKeyReleased(Key k) const;

        std::unordered_set<Key> pressedKeys{};
        std::unordered_set<Key> releasedKeys{};

        // GLFW key callback
        static void glfwKeyCallback(GLFWwindow* win, int key, int scancode, int action, int mods);
    } m_keyboard;

    static double GetMouseXDelta();
    static double GetMouseYDelta();
    static double GetMouseScrollDelta();

    static bool IsMouseButtonDown(MouseButton b);
    static bool WasMouseButtonPressed(MouseButton b);
    static bool WasMouseButtonReleased(MouseButton b);

    static std::pair<float, float> GetCursorPosition();
    struct Mouse {
        bool enabled{true};

        double lastXPos{};
        double lastYPos{};

        double xOffset{};
        double yOffset{};
        double scrollOffset{};

        bool firstInput{true};

        std::unordered_set<MouseButton> pressedButtons{};
        std::unordered_set<MouseButton> releasedButtons{};

        double GetXDelta() const;
        double GetYDelta() const;
        double GetScrollDelta() const;

        std::pair<double, double> GetCursorPosition() const;

        static void glfwMouseKeyCallback(GLFWwindow* win, int button, int action, int mods);
        static void glfwCursorPosCallback(GLFWwindow* win, double xPos, double yPos);
        static void glfwScrollCallback(GLFWwindow* win, double xOffset, double yOffset);

        bool IsButtonDown(MouseButton b) const;
        bool WasButtonPressed(MouseButton b) const;
        bool WasButtonReleased(MouseButton b) const;

    } m_mouse;


    static void glfwCharCallback(GLFWwindow * window, unsigned codepoint);

public:
    static Delegate<unsigned> CharDelegate;

};

