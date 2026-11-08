#include "Input.h"

#include <cassert>
#include <iostream>

Input* Input::instance = nullptr;

Delegate<Key, KeyMod> Input::OnKeyPressed{};
Delegate<Key, KeyMod> Input::OnKeyReleased{};

Delegate<MouseButton, KeyMod> Input::OnMouseButtonPressed{};
Delegate<MouseButton, KeyMod> Input::OnMouseButtonReleased{};

Delegate<double, double> Input::OnCursorMoved{};
Delegate<double, double> Input::OnScroll{};

Delegate<unsigned> Input::CharDelegate{};

Input::Input(Window *captureWindow) {
    assert(instance == nullptr && "INPUT: Input instance already exists, cannot create another\n");
    assert(captureWindow && "INPUT: Input construction requires a valid window to capture input\n");

    instance = this;
    m_capturingWindow = captureWindow;

    auto* glfwWindow = m_capturingWindow->getGlfwWindow();
    // GLFW callback binding
    glfwSetKeyCallback(m_capturingWindow->getGlfwWindow(), Keyboard::glfwKeyCallback);
    glfwSetMouseButtonCallback(m_capturingWindow->getGlfwWindow(), Mouse::glfwMouseKeyCallback);
    glfwSetCursorPosCallback(m_capturingWindow->getGlfwWindow(), Mouse::glfwCursorPosCallback);
    glfwSetScrollCallback(m_capturingWindow->getGlfwWindow(), Mouse::glfwScrollCallback);

    glfwSetCharCallback(glfwWindow, glfwCharCallback);
    // Not actually needed by imgui or me but keep incase
    //glfwSetMonitorCallback(glfwWindow,);
}

Input* Input::GetInstance() {
    assert(instance && "INPUT: No instance object created.");
    return instance;
}

void Input::update() {
    beginFrame();

    glfwPollEvents();
}

bool Input::Keyboard::IsKeyDown(Key k) const{
    return glfwGetKey(GetInstance()->m_capturingWindow->getGlfwWindow(), static_cast<int>(k)) == GLFW_PRESS;
}

bool Input::Keyboard::IsModDown(const KeyMod &mod) const{
    return GetInstance()->m_keyboard.activeMods.has(mod);
}

bool Input::Keyboard::WasKeyPressed(Key k) const{
    return GetInstance()->m_keyboard.pressedKeys.contains(k);
}

bool Input::Keyboard::WasKeyReleased(Key k) const{
    return GetInstance()->m_keyboard.releasedKeys.contains(k);
}


void Input::Keyboard::glfwKeyCallback(GLFWwindow *win, int key, int scancode, int action, int mods) {
    // TODO: Actual keycodes + glfw translation
    auto* inst = GetInstance();
    switch (action) {
        case GLFW_PRESS:
           inst->m_keyboard.pressedKeys.insert(static_cast<Key>(key));
            // Modifier keys
            if      (key == GLFW_KEY_LEFT_SHIFT   || key == GLFW_KEY_RIGHT_SHIFT) inst->m_keyboard.activeMods.addMod(KeyMod::Shift);
            else if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) inst->m_keyboard.activeMods.addMod(KeyMod::Control);
            else if (key == GLFW_KEY_LEFT_ALT     || key == GLFW_KEY_RIGHT_ALT) inst->m_keyboard.activeMods.addMod(KeyMod::Alt);
            else if (key == GLFW_KEY_LEFT_SUPER   || key == GLFW_KEY_RIGHT_SUPER) inst->m_keyboard.activeMods.addMod(KeyMod::Super);
            else if (key == GLFW_KEY_CAPS_LOCK) inst->m_keyboard.activeMods.addMod(KeyMod::CapsLock);
            else if (key == GLFW_KEY_NUM_LOCK) inst->m_keyboard.activeMods.addMod(KeyMod::NumLock);

            OnKeyPressed.broadcast(static_cast<Key>(key), inst->m_keyboard.activeMods);
            break;
        case GLFW_RELEASE:
            inst->m_keyboard.releasedKeys.insert(static_cast<Key>(key));

            // Modifier keys
            if      (key == GLFW_KEY_LEFT_SHIFT   || key == GLFW_KEY_RIGHT_SHIFT) inst->m_keyboard.activeMods.removeMod(KeyMod::Shift);
            else if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) inst->m_keyboard.activeMods.removeMod(KeyMod::Control);
            else if (key == GLFW_KEY_LEFT_ALT     || key == GLFW_KEY_RIGHT_ALT) inst->m_keyboard.activeMods.removeMod(KeyMod::Alt);
            else if (key == GLFW_KEY_LEFT_SUPER   || key == GLFW_KEY_RIGHT_SUPER) inst->m_keyboard.activeMods.removeMod(KeyMod::Super);
            else if (key == GLFW_KEY_CAPS_LOCK) inst->m_keyboard.activeMods.removeMod(KeyMod::CapsLock);
            else if (key == GLFW_KEY_NUM_LOCK) inst->m_keyboard.activeMods.removeMod(KeyMod::NumLock);

            OnKeyReleased.broadcast(static_cast<Key>(key), inst->m_keyboard.activeMods);
            break;
        default:
            break;
    }

}

double Input::GetMouseXDelta() {
    return GetInstance()->m_mouse.GetXDelta();
}

double Input::GetMouseYDelta() {
    return GetInstance()->m_mouse.GetYDelta();
}

double Input::GetMouseScrollDelta() {
    return GetInstance()->m_mouse.GetScrollDelta();
}

bool Input::IsMouseButtonDown(MouseButton b) {
    return GetInstance()->m_mouse.IsButtonDown(b);
}

bool Input::WasMouseButtonPressed(MouseButton b) {
    return GetInstance()->m_mouse.WasButtonPressed(b);
}

bool Input::WasMouseButtonReleased(MouseButton b) {
    return GetInstance()->m_mouse.WasButtonReleased(b);
}

std::pair<float, float> Input::GetCursorPosition() {
    return GetInstance()->m_mouse.GetCursorPosition();
}

void Input::beginFrame() {
    m_keyboard.pressedKeys.clear();
    m_keyboard.releasedKeys.clear();

    m_mouse.pressedButtons.clear();
    m_mouse.releasedButtons.clear();
    m_mouse.xOffset = 0;
    m_mouse.yOffset = 0;
    m_mouse.scrollOffset = 0;
}

void Input::setCaptureWindow(Window *newWindow) {
    if (!newWindow) std::cerr << "INPUT: New capture window requires a valid window pointer.\n";

    // Unbind old window callbacks
    glfwSetKeyCallback(m_capturingWindow->getGlfwWindow(), nullptr);
    glfwSetMouseButtonCallback(m_capturingWindow->getGlfwWindow(), nullptr);
    glfwSetCursorPosCallback(m_capturingWindow->getGlfwWindow(), nullptr);
    glfwSetScrollCallback(m_capturingWindow->getGlfwWindow(), nullptr);

    m_capturingWindow = newWindow;

    // Bind new window callbacks
    glfwSetKeyCallback(m_capturingWindow->getGlfwWindow(), Keyboard::glfwKeyCallback);
    glfwSetMouseButtonCallback(m_capturingWindow->getGlfwWindow(), Mouse::glfwMouseKeyCallback);
    glfwSetCursorPosCallback(m_capturingWindow->getGlfwWindow(), Mouse::glfwCursorPosCallback);
    glfwSetScrollCallback(m_capturingWindow->getGlfwWindow(), Mouse::glfwScrollCallback);


}

bool Input::IsKeyDown(Key k) {
    return GetInstance()->m_keyboard.IsKeyDown(k);
}

bool Input::IsModDown(const KeyMod &mod) {
    return GetInstance()->m_keyboard.IsModDown(mod);
}

bool Input::WasKeyPressed(Key k) {
    return GetInstance()->m_keyboard.WasKeyPressed(k);
}

bool Input::WasKeyReleased(Key k) {
    return GetInstance()->m_keyboard.WasKeyReleased(k);
}

double Input::Mouse::GetXDelta() const {
    return xOffset;
}

double Input::Mouse::GetYDelta() const {
    return yOffset;
}

double Input::Mouse::GetScrollDelta() const {
    return scrollOffset;
}

std::pair<double, double> Input::Mouse::GetCursorPosition() const {
    double outX, outY;
    glfwGetCursorPos(GetInstance()->m_capturingWindow->getGlfwWindow(), &outX, &outY);
    return {outX, outY};
}

void Input::Mouse::glfwMouseKeyCallback(GLFWwindow *win, int button, int action, int mods) {
    switch (action) {
        case GLFW_PRESS:
            GetInstance()->m_mouse.pressedButtons.insert(static_cast<MouseButton>(button));
            OnMouseButtonPressed.broadcast(static_cast<MouseButton>(button), GetInstance()->m_keyboard.activeMods);
            break;

        case GLFW_RELEASE:
            GetInstance()->m_mouse.releasedButtons.insert(static_cast<MouseButton>(button));
            OnMouseButtonReleased.broadcast(static_cast<MouseButton>(button), GetInstance()->m_keyboard.activeMods);
             break;
            default:
            break;
    }
}

void Input::Mouse::glfwCursorPosCallback(GLFWwindow *win, double xPos, double yPos) {

    // Broadcast absolute position regardless of mouse-delta tracking (UI needs it)
    OnCursorMoved.broadcast(xPos, yPos);

    auto* inst = GetInstance();
    if (!inst->m_mouse.enabled) return;
    if (inst->m_mouse.firstInput) {

        inst->m_mouse.lastXPos = xPos;
        inst->m_mouse.lastYPos = yPos;
        inst->m_mouse.firstInput = false;
        return;
    }

    inst->m_mouse.xOffset += xPos - inst->m_mouse.lastXPos;
    inst->m_mouse.yOffset += inst->m_mouse.lastYPos - yPos;

    inst->m_mouse.lastXPos = xPos;
    inst->m_mouse.lastYPos = yPos;
}

void Input::Mouse::glfwScrollCallback(GLFWwindow *win, double xOffset, double yOffset) {
    GetInstance()->m_mouse.scrollOffset += yOffset;
    OnScroll.broadcast(xOffset, yOffset);
}

bool Input::Mouse::IsButtonDown(MouseButton b) const {
    return glfwGetMouseButton(GetInstance()->m_capturingWindow->getGlfwWindow(), static_cast<int>(b));
}

bool Input::Mouse::WasButtonPressed(MouseButton b) const {
    return pressedButtons.contains(b);
}

bool Input::Mouse::WasButtonReleased(MouseButton b) const {
    return releasedButtons.contains(b);
}

void Input::glfwCharCallback(GLFWwindow *window, unsigned codepoint) {

    CharDelegate.broadcast(codepoint);
}



