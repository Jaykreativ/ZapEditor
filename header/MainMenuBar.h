#pragma once

namespace editor {
    class MainMenuBar
    {
    public:
        MainMenuBar();
        ~MainMenuBar();

        void draw();

        bool shouldSimulate();

    private:
        bool m_shouldSimulate = false;
    };
}

