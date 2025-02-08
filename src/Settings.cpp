#include "Settings.h"

#include "imgui.h"

namespace editor {
	Settings::Settings(EditorData* pEditorData)
		: m_pEditorData(pEditorData)
	{

	}
	Settings::~Settings() {

	}

	std::string Settings::name() {
		return "Settings";
	}

	void Settings::drawMain() {
		ImGui::Text("Go to a section to change your settings");
	}

	void Settings::drawProject() {
		auto& project = m_pEditorData->project;

		const size_t bufSize = 50;
		{
			char buf[bufSize] = "";
			memcpy(buf, project.name.c_str(), std::min<size_t>(bufSize, project.name.size()));
			ImGui::InputText("Name(WIP)", buf, bufSize);
			//project.name = buf;
		}
		{
			char buf[bufSize] = "";
			memcpy(buf, project.assetLibraryPath.c_str(), std::min<size_t>(bufSize, project.assetLibraryPath.size()));
			ImGui::InputText("Asset Library", buf, bufSize);
			project.assetLibraryPath = buf;
		}
	}

	void Settings::draw() {
		auto availableWindowSize = ImGui::GetContentRegionAvail();
		uint32_t initialMenuSize = 300;
		ImGui::BeginChild("SectionMenu##Settings", ImVec2(initialMenuSize, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
		if (ImGui::Button("Project"))
			m_section = ePROJECT;
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginChild("SectionView##Settings", ImVec2(availableWindowSize.x-initialMenuSize, 0));
		switch (m_section)
		{
		case editor::Settings::eMAIN:
			drawMain();
			break;
		case editor::Settings::ePROJECT:
			drawProject();
			break;
		default:
			break;
		}
		ImGui::EndChild();
	}

	ImGuiWindowFlags Settings::getWindowFlags() {
		return 0;
	}

	void Settings::openSectionMain() {
		m_section = eMAIN;
	}
	void Settings::openSectionProject() {
		m_section = ePROJECT;
	}
}