#pragma once

#include <GLFW/glfw3.h>

namespace core
{
    class input
    {
    public:
        static void setWindow(GLFWwindow* window) { _window = window; }

        using priority_t = uint32_t;
        enum class key_action {
            ACTION_RELEASE = GLFW_RELEASE,
            ACTION_PRESS   = GLFW_PRESS,
            ACTION_REPEAT  = GLFW_REPEAT
        };
        enum class key {
        KEY_UNKNOWN      = GLFW_KEY_UNKNOWN,
            KEY_SPACE        = GLFW_KEY_SPACE,
            KEY_APOSTROPHE   = GLFW_KEY_APOSTROPHE,  // '
            KEY_COMMA        = GLFW_KEY_COMMA,       // ,
            KEY_MINUS        = GLFW_KEY_MINUS,       // -
            KEY_PERIOD       = GLFW_KEY_PERIOD,      // .
            KEY_SLASH        = GLFW_KEY_SLASH,       // /

            KEY_0            = GLFW_KEY_0,
            KEY_1            = GLFW_KEY_1,
            KEY_2            = GLFW_KEY_2,
            KEY_3            = GLFW_KEY_3,
            KEY_4            = GLFW_KEY_4,
            KEY_5            = GLFW_KEY_5,
            KEY_6            = GLFW_KEY_6,
            KEY_7            = GLFW_KEY_7,
            KEY_8            = GLFW_KEY_8,
            KEY_9            = GLFW_KEY_9,

            KEY_SEMICOLON    = GLFW_KEY_SEMICOLON,   // ;
            KEY_EQUAL        = GLFW_KEY_EQUAL,       // =

            KEY_A            = GLFW_KEY_A,
            KEY_B            = GLFW_KEY_B,
            KEY_C            = GLFW_KEY_C,
            KEY_D            = GLFW_KEY_D,
            KEY_E            = GLFW_KEY_E,
            KEY_F            = GLFW_KEY_F,
            KEY_G            = GLFW_KEY_G,
            KEY_H            = GLFW_KEY_H,
            KEY_I            = GLFW_KEY_I,
            KEY_J            = GLFW_KEY_J,
            KEY_K            = GLFW_KEY_K,
            KEY_L            = GLFW_KEY_L,
            KEY_M            = GLFW_KEY_M,
            KEY_N            = GLFW_KEY_N,
            KEY_O            = GLFW_KEY_O,
            KEY_P            = GLFW_KEY_P,
            KEY_Q            = GLFW_KEY_Q,
            KEY_R            = GLFW_KEY_R,
            KEY_S            = GLFW_KEY_S,
            KEY_T            = GLFW_KEY_T,
            KEY_U            = GLFW_KEY_U,
            KEY_V            = GLFW_KEY_V,
            KEY_W            = GLFW_KEY_W,
            KEY_X            = GLFW_KEY_X,
            KEY_Y            = GLFW_KEY_Y,
            KEY_Z            = GLFW_KEY_Z,

            KEY_LEFT_BRACKET  = GLFW_KEY_LEFT_BRACKET,   // [
            KEY_BACKSLASH     = GLFW_KEY_BACKSLASH,      // '\'
            KEY_RIGHT_BRACKET = GLFW_KEY_RIGHT_BRACKET,  // ]
            KEY_GRAVE_ACCENT  = GLFW_KEY_GRAVE_ACCENT,   // `

            KEY_ESCAPE        = GLFW_KEY_ESCAPE,
            KEY_ENTER         = GLFW_KEY_ENTER,
            KEY_TAB           = GLFW_KEY_TAB,
            KEY_BACKSPACE     = GLFW_KEY_BACKSPACE,
            KEY_INSERT        = GLFW_KEY_INSERT,
            KEY_DELETE        = GLFW_KEY_DELETE,
            KEY_RIGHT         = GLFW_KEY_RIGHT,
            KEY_LEFT          = GLFW_KEY_LEFT,
            KEY_DOWN          = GLFW_KEY_DOWN,
            KEY_UP            = GLFW_KEY_UP,
            KEY_PAGE_UP       = GLFW_KEY_PAGE_UP,
            KEY_PAGE_DOWN     = GLFW_KEY_PAGE_DOWN,
            KEY_HOME          = GLFW_KEY_HOME,
            KEY_END           = GLFW_KEY_END,

            KEY_CAPS_LOCK     = GLFW_KEY_CAPS_LOCK,
            KEY_SCROLL_LOCK   = GLFW_KEY_SCROLL_LOCK,
            KEY_NUM_LOCK      = GLFW_KEY_NUM_LOCK,
            KEY_PRINT_SCREEN  = GLFW_KEY_PRINT_SCREEN,
            KEY_PAUSE         = GLFW_KEY_PAUSE,

            KEY_F1            = GLFW_KEY_F1,
            KEY_F2            = GLFW_KEY_F2,
            KEY_F3            = GLFW_KEY_F3,
            KEY_F4            = GLFW_KEY_F4,
            KEY_F5            = GLFW_KEY_F5,
            KEY_F6            = GLFW_KEY_F6,
            KEY_F7            = GLFW_KEY_F7,
            KEY_F8            = GLFW_KEY_F8,
            KEY_F9            = GLFW_KEY_F9,
            KEY_F10           = GLFW_KEY_F10,
            KEY_F11           = GLFW_KEY_F11,
            KEY_F12           = GLFW_KEY_F12,

            KEY_LEFT_SHIFT    = GLFW_KEY_LEFT_SHIFT,
            KEY_LEFT_CONTROL  = GLFW_KEY_LEFT_CONTROL,
            KEY_LEFT_ALT      = GLFW_KEY_LEFT_ALT,
            KEY_LEFT_SUPER    = GLFW_KEY_LEFT_SUPER,
            KEY_RIGHT_SHIFT   = GLFW_KEY_RIGHT_SHIFT,
            KEY_RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL,
            KEY_RIGHT_ALT     = GLFW_KEY_RIGHT_ALT,
            KEY_RIGHT_SUPER   = GLFW_KEY_RIGHT_SUPER,
            KEY_MENU          = GLFW_KEY_MENU
        };
        enum class mouse_button {
            BUTTON_LEFT   = GLFW_MOUSE_BUTTON_LEFT,
            BUTTON_RIGHT  = GLFW_MOUSE_BUTTON_RIGHT,
            BUTTON_MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE,

            BUTTON_4 = GLFW_MOUSE_BUTTON_4,
            BUTTON_5 = GLFW_MOUSE_BUTTON_5,
            BUTTON_6 = GLFW_MOUSE_BUTTON_6,
            BUTTON_7 = GLFW_MOUSE_BUTTON_7,
            BUTTON_8 = GLFW_MOUSE_BUTTON_8,

            BUTTON_UNKNOWN = -1
        };

        static bool isKey(key k, key_action a = key_action::ACTION_RELEASE) 
            { return glfwGetKey(_window, static_cast<int32_t>(k)) == static_cast<int32_t>(a); }
        static bool isMouseButton(mouse_button b, key_action a = key_action::ACTION_RELEASE)
            { return glfwGetMouseButton(_window, static_cast<int32_t>(b)) == static_cast<int32_t>(a); }
    private:
        static GLFWwindow* _window;
    }; 
} // namespace core
