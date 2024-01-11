#include "Viewport.h"

#include "Zap/Rendering/Renderer.h"
#include "Zap/Rendering/PBRenderer.h"
#include "imgui.h"

namespace editor {
	Viewport::Viewport(Zap::PBRenderer* render, Zap::Renderer* renderer)
		: m_render(render), m_renderer(renderer)
	{}

	void Viewport::updateGui() {
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGuiWindowFlags windowFlags = 0;
		if (!canMove) windowFlags |= ImGuiWindowFlags_NoMove;
		ImGui::Begin("Viewport", nullptr, windowFlags);
		auto pos = ImGui::GetWindowPos();
		auto size = ImGui::GetWindowSize();
		m_render->setViewport(size.x, size.y-19, pos.x, pos.y+19);
		m_renderer->update();
		ImGui::End();
		ImGui::PopStyleColor();
	}
}