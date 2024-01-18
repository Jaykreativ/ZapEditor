#include "MainMenuBar.h"
#include "imgui.h"

namespace editor {
    MainMenuBar::MainMenuBar() {}
    MainMenuBar::~MainMenuBar() {}

    void MainMenuBar::draw() {
        ImGui::BeginMainMenuBar();
        if (m_shouldSimulate) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5, 0, 0, 0.5));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5, 0, 0, 1));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9, 0.1, 0.1, 1));
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0.5, 0, 0.5));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0.5, 0, 1));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1, 0.9, 0.1, 1));
        }
        if (ImGui::Button("Simulate")) {
            m_shouldSimulate = !m_shouldSimulate;
        }
        ImGui::PopStyleColor(3);
        ImGui::EndMainMenuBar();
    }

    bool MainMenuBar::shouldSimulate() {
        return m_shouldSimulate;
    }
}