#pragma once

#include "Zap/Scene/Actor.h"

#include "ZapEditor.h"
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
		TransformEditor(std::vector<Zap::Actor> selectedActors);

		~TransformEditor();

		std::string name();

		void draw();

		bool isValid();

	private:
		std::vector<Zap::Actor> m_selectedActors;
	};

	class ShapeCreateSection {
	public:
		struct MaterialCreationInfo {
			float staticFriction = 0.5;
			float dynamicFriction = 1;
			float restitution = 0.1;
		};
		MaterialCreationInfo m_materialCreationInfo = {};

		struct ShapeCreationInfo {
			uint32_t geometryType = 1;
			uint32_t materialIndex = 0;
			glm::vec3 boxExtent = { 1, 1, 1 };
			float sphereRadius = 1;
			float capsuleRadius = 1;
			float capsuleHalfHeight = 1;
			std::filesystem::path hitMeshPath = "";
		};
		ShapeCreationInfo m_shapeCreationInfo = {};

		void drawAddExclusivePopup(EditorData& editorData, Zap::Actor selectedActor, bool isDynamic);

		void drawAddShapePopup(EditorData& editorData, Zap::Actor selectedActor, bool isDynamic);

		void drawCreateShapePopup(EditorData& editorData, Zap::Actor selectedActor, bool isDynamic, bool isExclusive = false);

		void drawCreateMaterialPopup(EditorData& editorData);

		void draw(EditorData& editorData, Zap::Actor selectedActor, bool isDynamic);
	};

	class RigidDynamicEditor : public ComponentEditor
	{
	public:
		RigidDynamicEditor(EditorData* pEditorData, std::vector<Zap::Actor>& selectedActors);

		~RigidDynamicEditor();

		std::string name();

		void draw();

		bool isValid();

	private:
		EditorData* m_pEditorData;

		std::vector<Zap::Actor>& m_selectedActors;

		ShapeCreateSection m_shapeCreateSection = {};
	};

	class RigidStaticEditor : public ComponentEditor
	{
	public:
		RigidStaticEditor(EditorData* pEditorData, std::vector<Zap::Actor>& selectedActors);

		~RigidStaticEditor();

		std::string name();

		void draw();

		bool isValid();

	private:
		EditorData* m_pEditorData;

		std::vector<Zap::Actor>& m_selectedActors;

		ShapeCreateSection m_shapeCreateSection = {};
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
		ComponentView(EditorData* pEditorData, std::vector<ViewLayer*>& layers, std::vector<Zap::Actor>& selectedActors);
		~ComponentView();

		std::string name();

		void draw();

		ImGuiWindowFlags getWindowFlags();

	private:
		EditorData* m_pEditorData;

		std::vector<ViewLayer*>& m_layers;
		ComponentEditor* m_selectedEditor = nullptr;
		std::vector<Zap::Actor>& m_selectedActors;
	};
}