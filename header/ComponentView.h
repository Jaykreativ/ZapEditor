#pragma once

#include "Zap/Scene/Actor.h"

#include "ViewLayer.h"

namespace editor {
	class ComponentEditor : public ViewLayer
	{
	public:
		virtual void draw() = 0;

		virtual bool isValid() = 0;

		ImGuiWindowFlags getWindowFlags();
	};

	class TransformEditor : public ComponentEditor
	{
	public:
		TransformEditor(std::vector<Zap::Actor>& selectedActors);

		~TransformEditor();

		std::string name();

		void draw();

		bool isValid();

	private:
		std::vector<Zap::Actor>& m_selectedActors;
	};

	class RigidDynamicEditor : public ComponentEditor
	{
	public:
		RigidDynamicEditor(std::vector<Zap::Actor>& selectedActors);

		~RigidDynamicEditor();

		std::string name();

		void draw();

		bool isValid();

	private:
		std::vector<Zap::Actor>& m_selectedActors;
	};

	class LightEditor : public ComponentEditor
	{
	public:
		LightEditor(std::vector<Zap::Actor>& selectedActors);

		~LightEditor();

		std::string name();

		void draw();

		bool isValid();

	private:
		std::vector<Zap::Actor>& m_selectedActors;
	};

	class ComponentView : public ViewLayer
	{
	public:
		ComponentView(std::vector<Zap::Actor>& selectedActors);
		~ComponentView();

		std::string name();

		void draw();

		ImGuiWindowFlags getWindowFlags();

	private:
		ComponentEditor* m_selectedEditor = nullptr;
		std::vector<Zap::Actor>& m_selectedActors;
	};
}