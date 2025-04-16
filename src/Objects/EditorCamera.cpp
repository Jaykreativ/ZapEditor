#include "EditorCamera.h"

namespace editor {
	float Camera::m_sensHorz = 1;
	float Camera::m_sensVert = 1;

	CameraKeybinds Camera::m_keybinds = {};

	Camera::Camera(Zap::Scene& scene)
		: m_camera()
	{
		scene.attachActor(m_camera);
		m_camera.addTransform();
		m_camera.addCamera();
		switch (m_mode)
		{
		case eFLY:
			m_camera.cmpCamera_lookAtFront();
			break;
		case eORBIT: {
			m_camera.cmpCamera_lookAtFront();
			glm::mat4 offset = glm::mat4(1);
			offset[3] = { 0, 0, -m_orbitDist, 1 };
			m_camera.cmpCamera_setOffset(offset);
			break;
		}
		default:
			break;
		}
	}

	Camera::~Camera() {
		m_camera.destroy();
	}

	void Camera::drawSettings() {
		ImGui::SeparatorText("Local Settings");

		static const int camModeCount = 3;
		static const std::string camModeStrings[] = {
			"None",
			"Fly",
			"Orbit"
		};
		if (ImGui::BeginCombo("Mode", camModeStrings[m_mode].c_str())) {
			for (size_t i = 1; i < camModeCount; i++) {
				if (ImGui::Selectable(camModeStrings[i].c_str(), m_mode == i)) {
					setMode(i);
				}
			}
			ImGui::EndCombo();
		}

		ImGui::SeparatorText("Global Settings");

		ImGui::DragFloat("Horizontal Sensitivity", &m_sensHorz, 0.01f);
		ImGui::DragFloat("Vertical Sensitivity", &m_sensVert, 0.01f);

		if (ImGui::Button("Camera Keybinds")) {
			ImGui::OpenPopup("Camera Keybinds##Popup");
		}

		if (ImGui::BeginPopupModal("Camera Keybinds##Popup")) {
			ImGui::SeparatorText("Fly");
			ImGui::Text("Forward:"); ImGui::SameLine(); ImGui::Text(ImGui::GetKeyName(m_keybinds.forward));
			ImGui::Text("Backward:"); ImGui::SameLine(); ImGui::Text(ImGui::GetKeyName(m_keybinds.backward));
			ImGui::Text("Left:"); ImGui::SameLine(); ImGui::Text(ImGui::GetKeyName(m_keybinds.left));
			ImGui::Text("Right:"); ImGui::SameLine(); ImGui::Text(ImGui::GetKeyName(m_keybinds.right));
			ImGui::Text("Up:"); ImGui::SameLine(); ImGui::Text(ImGui::GetKeyName(m_keybinds.up));
			ImGui::Text("Down:"); ImGui::SameLine(); ImGui::Text(ImGui::GetKeyName(m_keybinds.down));
			ImGui::SeparatorText("Orbit");
			ImGui::Text("Move:"); ImGui::SameLine(); ImGui::Text(ImGui::GetKeyName(m_keybinds.moveCenter));

			if (ImGui::Button("Done")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	void Camera::orbitMovement(float dTime, bool hovered, bool focused) {
		if (hovered) {
			m_orbitDist += ImGui::GetIO().MouseWheel * m_orbitDist * 0.1f;
		}

		glm::mat4 transform = m_camera.cmpTransform_getTransform();
		if (focused && hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			auto mouseDelta = ImGui::GetIO().MouseDelta;
			glm::vec3 pos = transform[3];
			if (ImGui::IsKeyDown(m_keybinds.moveCenter)) { // move only when move button pressed
				// move
				pos += glm::vec3(transform[0]) * -mouseDelta.x * m_orbitDist * 0.005f;
				pos += glm::vec3(transform[1]) * mouseDelta.y * m_orbitDist * 0.005f;
			}
			else {
				// turn

				glm::mat4 rotHorz = glm::rotate(glm::mat4(1), mouseDelta.x * m_sensHorz * 0.01f, { 0, 1, 0 }); // global space
				glm::mat4 rotVert = glm::rotate(glm::mat4(1), mouseDelta.y * m_sensVert * 0.01f, { 1, 0, 0 }); // transform space

				transform = rotHorz * transform * rotVert;
			}
			transform[3] = { pos , 1 };
		}
		m_camera.cmpTransform_setTransform(transform);
		glm::mat4 off = glm::translate(glm::mat4(1), -glm::vec3(transform[2]*m_orbitDist));
		m_camera.cmpCamera_setOffset(off);
	}

	void Camera::flyMovement(float dTime, bool hovered, bool focused) {
		glm::mat4 transform = m_camera.cmpTransform_getTransform();
		bool moved = false;
		// move
		if (focused) {
			if (ImGui::IsKeyDown(m_keybinds.forward)) {
				glm::vec3 vec = transform[2];
				transform[3] = glm::vec4(glm::vec3(transform[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * m_flySpeed, 1);
				moved = true;
			}
			if (ImGui::IsKeyDown(m_keybinds.backward)) {
				glm::vec3 vec = -transform[2];
				transform[3] = glm::vec4(glm::vec3(transform[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * m_flySpeed, 1);
				moved = true;
			}
			if (ImGui::IsKeyDown(m_keybinds.right)) {
				glm::vec3 vec = transform[0];
				transform[3] = glm::vec4(glm::vec3(transform[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * m_flySpeed, 1);
				moved = true;
			}
			if (ImGui::IsKeyDown(m_keybinds.left)) {
				glm::vec3 vec = -transform[0];
				transform[3] = glm::vec4(glm::vec3(transform[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * m_flySpeed, 1);
				moved = true;
			}
			if (ImGui::IsKeyDown(m_keybinds.down)) {
				transform[3] = glm::vec4(glm::vec3(transform[3]) + glm::vec3{ 0, -2, 0 }*dTime * m_flySpeed, 1);
				moved = true;
			}
			if (ImGui::IsKeyDown(m_keybinds.up)) {
				transform[3] = glm::vec4(glm::vec3(transform[3]) + glm::vec3{ 0, 2, 0 }*dTime * m_flySpeed, 1);
				moved = true;
			}
		}

		if (moved) {
			m_flySpeed += ImGui::GetIO().MouseWheel * m_flySpeed * 0.1f;
		}

		// turn
		if (focused && hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			auto mouseDelta = ImGui::GetIO().MouseDelta;

			glm::mat4 rotHorz = glm::rotate(glm::mat4(1), mouseDelta.x * m_sensHorz * 0.01f, { 0, 1, 0 }); // global space
			glm::mat4 rotVert = glm::rotate(glm::mat4(1), mouseDelta.y * m_sensVert * 0.01f, { 1, 0, 0 }); // transform space

			glm::vec3 pos = transform[3];
			transform = rotHorz * transform * rotVert;
			transform[3] = { pos , 1 };
		}
		m_camera.cmpTransform_setTransform(transform);
		m_camera.cmpCamera_setOffset(glm::mat4(1));
	}

	void Camera::updateMovement(float dTime, bool hovered, bool focused) {
		switch (m_mode)
		{
		case eORBIT:
			orbitMovement(dTime, hovered, focused);
			break;
		case eFLY:
			flyMovement(dTime, hovered, focused);
			break;
		default:
			break;
		}
	}

	void Camera::resetTo(glm::vec3 pos, float dist) {
		setOrbitDistance(dist);
		switch (m_mode)
		{
		case eORBIT:
			setPosition(pos);
			break;
		case eFLY:
			setPosition(pos + glm::vec3(0, 0, dist));
			break;
		default:
			break;
		}
	}

	void Camera::setMode(int mode) {
		m_mode = mode;
	}
	void Camera::setMode(CameraMode mode) {
		setMode((int)mode);
	}

	void Camera::setPosition(glm::vec3 pos) {
		m_camera.cmpTransform_setPos(pos);
	}

	void Camera::setOrbitDistance(float dist) {
		m_orbitDist = dist;
	}

	glm::vec3 Camera::getCenterPosition() {
		return m_camera.cmpTransform_getPos();
	}

	glm::vec3 Camera::getPosition() {
		return getCenterPosition() + glm::vec3(m_camera.cmpCamera_getOffset()[3]);
	}

	glm::mat4 Camera::getTransform() {
		return m_camera.cmpTransform_getTransform();
	}

	glm::mat4 Camera::getPerspective(float aspect) {
		return m_camera.cmpCamera_getPerspective(aspect);
	}

	glm::mat4 Camera::getView() {
		return m_camera.cmpCamera_getView();
	}

	Zap::Actor Camera::getZapCamera() {
		return *this;
	}

	float Camera::getOrbitDistance() {
		return m_orbitDist;
	}
}