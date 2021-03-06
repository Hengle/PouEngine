#ifndef EVENTSMANAGER_H
#define EVENTSMANAGER_H

#include <GLFW/glfw3.h>

#include <stack>
#include <glm/vec2.hpp>
#include <string>

class EventsManager
{
    public:
        EventsManager();
        virtual ~EventsManager();

        void init(GLFWwindow *window);

        void update();
        void waitForEvents();

        std::string getTextEnteredAsUtf8() const;
        const std::u32string &getTextEntered() const;

        std::string getClipBoard() const;
        void setClipBoard(const std::string &text) const;

        bool keyPressed(int key, bool allowRepeat = false)    const;
        bool keyIsPressed(int key)  const;
        bool keyReleased(int key)   const;
        bool keyMod(int mod) const;

        bool mouseButtonPressed(int button)     const;
        bool mouseButtonIsPressed(int button)   const;
        bool mouseButtonReleased(int button)    const;

        glm::vec2 mousePosition()   const;
        glm::vec2 normalizedMousePosition()   const;
        glm::vec2 centeredMousePosition()   const;
        glm::vec2 mouseScroll()     const;

        bool isAskingToClose() const;
        bool resizedWindow() const;

        glm::vec2 getFramebufferSize() const;

    protected:
        void updateKey(int key, int action);
        void updateMouseButton(int button, int action);
        void updateMouseScroll(double xoffset, double yoffset);
        void updateMousePosition(double xpos, double ypos, int width, int height);
        //void updateWindowSize(int width, int height);

        static void char_callback(GLFWwindow* window, unsigned int codepoint);
        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
        static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
        static void resize_callback(GLFWwindow* window, int width, int height);

    private:
        GLFWwindow *m_window;

        std::u32string  m_textEntered;

        bool m_keyPressed[GLFW_KEY_LAST+1];
        bool m_keyRepeated[GLFW_KEY_LAST+1];
        bool m_keyIsPressed[GLFW_KEY_LAST+1];
        bool m_keyReleased[GLFW_KEY_LAST+1];
        int  m_keyMod;

        std::stack<int> m_justPressedKeys, m_justReleasedKeys,
                        m_justPressedMouseButtons, m_justReleasedMouseButtons;

        bool m_mouseButtonPressed[GLFW_MOUSE_BUTTON_LAST+1];
        bool m_mouseButtonIsPressed[GLFW_MOUSE_BUTTON_LAST+1];
        bool m_mouseButtonReleased[GLFW_MOUSE_BUTTON_LAST+1];

        glm::vec2 m_mousePosition;
        glm::vec2 m_normalizedMousePosition;
        glm::vec2 m_centeredMousePosition;
        glm::vec2 m_mouseScroll;

        bool m_askingToClose;
        bool m_resizedWindow;
};

#endif // EVENTSMANAGER_H
