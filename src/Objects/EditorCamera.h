#pragma once

#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Actor.h"

#include "imgui.h"
#include "glm.hpp"

namespace editor {
	enum CameraMode {
		eNONE = 0,
		eFLY = 1,
		eORBIT = 2
	};

	struct CameraKeybinds {
		// fly
		ImGuiKey forward = ImGuiKey_W;
		ImGuiKey backward = ImGuiKey_S;
		ImGuiKey left = ImGuiKey_A;
		ImGuiKey right = ImGuiKey_D;
		ImGuiKey up = ImGuiKey_Space;
		ImGuiKey down = ImGuiKey_C;
		// orbit
		ImGuiKey moveCenter = ImGuiKey_LeftShift;
	};

	class Camera {
	public:
		Camera(Zap::Scene& scene);
		~Camera();

		operator Zap::Actor() { return m_camera; }

		void drawSettings();

		void updateMovement(float dTime, bool hovered = true, bool focused = true);

		// resets the camera to the given position at a distance
		void resetTo(glm::vec3 pos = {0, 0, 0}, float dist = 0);

		void setMode(int mode);
		void setMode(CameraMode mode);

		void setPosition(glm::vec3 pos);

		void setOrbitDistance(float dist);

		glm::vec3 getCenterPosition();

		glm::vec3 getPosition();

		glm::mat4 getTransform();

		glm::mat4 getPerspective(float aspect);

		glm::mat4 getView();

		Zap::Actor getZapCamera();

		float getOrbitDistance();

	private:
		Zap::Actor m_camera;
		int m_mode = eFLY;

		// fly
		float m_flySpeed = 1;

		// orbit
		float m_orbitDist = 5;

		static float m_sensHorz;
		static float m_sensVert;

		static CameraKeybinds m_keybinds;

		void orbitMovement(float dTime, bool hovered, bool focused);

		void flyMovement(float dTime, bool hovered, bool focused);
	};
}