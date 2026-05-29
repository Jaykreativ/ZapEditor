#include "HitboxEditor.h"

#include "Zap/Physics/Shape.h"
#include "backends/imgui_impl_vulkan.h"

#include "glm/gtc/quaternion.hpp"

#include <array>

namespace editor {
	HitboxEditor::HitboxEditor(EditorData* pEditorData)
		: m_pEditorData(pEditorData)//, m_pbrTask(&m_scene)
	{
		setupScene();
		m_spLineBuffer = std::make_shared<Zap::LineBuffer>();
		activateRenderer();
	}

	HitboxEditor::~HitboxEditor() {
		disableRenderer();
	}

	std::string HitboxEditor::name() {
		return "HitboxEdtior";
	}

	void HitboxEditor::draw() {
		if(ImGui::BeginMenuBar()){
			if (ImGui::BeginMenu("View")) {
				if (ImGui::BeginMenu("Camera")) {
					m_upCamera->drawSettings();
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::BeginChild("Viewport", { ImGui::GetContentRegionAvail().x / 1.5f, ImGui::GetContentRegionAvail().y }, ImGuiChildFlags_ResizeX);
		{
			// update scene
			{
				m_light.cmpTransform_setPos(m_upCamera->getPosition());
				m_light.cmpLight_setStrength(m_upCamera->getOrbitDistance() * m_upCamera->getOrbitDistance());

				if (m_pEditorData->selectedActors.size() > 0) {
					updateActor();
				}
			}

			auto imageExtent = m_finalTarget->getExtent();
			auto extent = ImGui::GetContentRegionAvail();

			// resize
			if (extent.x != imageExtent.width || extent.y != imageExtent.height) {
				extent.x = std::max<float>(extent.x, 1);
				extent.y = std::max<float>(extent.y, 1);
				m_renderer->resize({ extent.x, extent.y });
				if (m_pbrTask)
					m_pbrTask->setViewport(extent.x, extent.y, 0, 0);
			}

			// render Lines
			std::vector<Zap::LineVertex> lines;
			m_scene.getPxDebugVertices(lines);

			if (lines.size() > 0) {
				m_spLineBuffer->resize(sizeof(Zap::LineVertex) * lines.size()+2);
				void* rawData;
				m_spLineBuffer->map(0, rawData);
				memcpy(rawData, lines.data(), sizeof(Zap::LineVertex) * lines.size());
				m_spLineBuffer->unmap();
			}

			// render scene
			m_scene.simulate(1 / 60.f);
			m_scene.update();
			if(m_pbrTask)
				m_pbrTask->updateCamera(*m_upCamera);
			if(m_lineTask)
				m_lineTask->updateCamera(*m_upCamera);
			m_renderer->render();

			// render Gui
			ImGui::Image(*m_finalTarget.get(), extent);
			m_isImageHovered = ImGui::IsItemHovered();
			m_isFocused = ImGui::IsWindowFocused();

			// turn camera
			m_upCamera->updateMovement(m_pEditorData->dTime, m_isImageHovered, m_isFocused);
		}
		ImGui::EndChild(); 

		ImGui::SameLine();

		ImGui::BeginChild("Edit", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX);
		{
			for (size_t i = 0; i < m_shapes.size(); i++) {
				if (ImGui::TreeNode(("Shape" + std::to_string(i)).c_str())) {
					auto geometry = m_shapes[i].getGeometry();
					switch (geometry->getType())
					{
					case Zap::eGEOMETRY_TYPE_BOX: {
							auto boxGeometry = (Zap::BoxGeometry*)geometry.get();
							auto extent = boxGeometry->getHalfExtents();
							ImGui::DragFloat3("HalfExtents", (float*)(&extent), glm::length(extent)*0.01, 0.001, FLT_MAX, "%.3f", ImGuiSliderFlags_NoRoundToFormat | ImGuiSliderFlags_AlwaysClamp);
							boxGeometry->setHalfExtents(extent);
							break;
					}
					case Zap::eGEOMETRY_TYPE_PLANE:
						break;
					default: {
						ImGui::Text("Unknown geometry type");
						break;
					}
					}
					m_shapes[i].setGeometry(*geometry);
					m_editingShapes[i].setGeometry(*geometry);

					glm::vec3 shapePos = m_shapes[i].getLocalPosition();
					ImGui::DragFloat3("Pos", (float*)&shapePos, 0.01);
					m_shapes[i].setLocalPosition(shapePos);
					m_editingShapes[i].setLocalPosition(shapePos);

					glm::quat shapeQuat = m_shapes[i].getLocalRotation();
					glm::vec3 shapeEuler = glm::degrees(glm::eulerAngles(shapeQuat));
					ImGui::DragFloat3("Rotation", (float*)&shapeEuler, 0.1);
					m_shapes[i].setLocalRotation(glm::radians(shapeEuler));
					m_editingShapes[i].setLocalRotation(glm::radians(shapeEuler));

					ImGui::TreePop();
				}
			}
		}
		ImGui::EndChild();
	}

	ImGuiWindowFlags HitboxEditor::getWindowFlags() {
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
		if (m_isImageHovered) windowFlags |= ImGuiWindowFlags_NoMove;
		return windowFlags;

	}

	void HitboxEditor::updateActor() {
		Zap::Actor selectedActor = m_pEditorData->selectedActors.back();
		bool isActorChanged = !(selectedActor == m_oldActor);
		bool areShapesChanged = false;
		if (selectedActor.hasRigidDynamic())
			areShapesChanged = m_shapes.size() != selectedActor.cmpRigidDynamic_getShapes().size();
		else if (selectedActor.hasRigidStatic())
			areShapesChanged = m_shapes.size() != selectedActor.cmpRigidStatic_getShapes().size();
		else if (m_shapes.size() > 0)
			areShapesChanged = true;
		areShapesChanged |= isActorChanged;

		m_oldActor = selectedActor;

		if (isActorChanged) {
			glm::mat4 transform;
			if (selectedActor.hasTransform()) {
				transform = selectedActor.cmpTransform_getTransform();
			}
			else {
				transform = glm::mat4(1);
			}

			//extract scale
			glm::vec3 scale = {1, 1, 1};
			for (int i = 0; i < 3; i++)
				scale[i] = glm::length(glm::vec3(transform[i]));

			transform = glm::scale(glm::mat4(1), scale);

			if (m_actor.hasTransform())
				m_actor.destroyTransform();
			m_actor.addTransform(transform);
			if (m_actor.hasModel())
				m_actor.destroyModel();
			if (selectedActor.hasModel()) {
				auto model = selectedActor.cmpModel_getModel();
				m_actor.addModel(model);
				m_upCamera->resetTo({0, 0, 0}, glm::max(glm::length(transform * glm::vec4(model.boundMin, 1)), glm::length(transform * glm::vec4(model.boundMax, 1))) * 1.5f);
			}
			else
				m_upCamera->resetTo({ 0, 0, 0 }, 5);
		}

		if (areShapesChanged) {
			if (m_actor.hasRigidStatic())
				m_actor.destroyRigidStatic();
			if (m_actor.hasRigidDynamic())
				m_actor.destroyRigidDynamic();

			if (selectedActor.hasRigidDynamic()) {
				m_shapes = selectedActor.cmpRigidDynamic_getShapes();
			}
			else if (selectedActor.hasRigidStatic()) {
				m_shapes = selectedActor.cmpRigidStatic_getShapes();
			}
			else
				m_shapes.clear();

			if (m_shapes.size() > 0) {
				m_editingShapes.resize(m_shapes.size());
				for (uint32_t i = 0; i < m_shapes.size(); i++) {
					m_editingShapes[i] = Zap::Shape(*m_shapes[i].getGeometry(), m_physicsMaterial, true);
				}
				if (selectedActor.hasRigidDynamic()) {
					m_actor.addRigidDynamic(m_editingShapes);
					m_actor.cmpRigidDynamic_setFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true);
				}
				else if (selectedActor.hasRigidStatic()) {
					m_actor.addRigidStatic(m_editingShapes);
					m_actor.cmpRigidStatic_setFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true);
				}
			}
		}
	}

	void HitboxEditor::setupScene() {
		m_scene.init();
		m_scene.attachActor(m_actor);
		m_scene.attachActor(m_light);

		m_upCamera = std::make_unique<editor::Camera>(m_scene); // create the camera
		m_upCamera->setMode(eORBIT);
		m_upCamera->setOrbitDistance(5);

		if (m_pEditorData->selectedActors.size() > 0)
			updateActor();

		m_light.addTransform(glm::mat4(1));
		m_light.cmpTransform_setPos({ 0, 0, -5 });
		m_light.addLight({ 1, 1, 1 }, 25, 1);

		m_scene.update();
	}

	void HitboxEditor::activateRenderer() {
		m_renderer = std::make_unique<Zap::Renderer>();

		// create the final viewport image
		m_finalTarget = m_renderer->createRenderTarget<Zap::RenderTargetGuiImage>(
			Zap::GlobalSettings::getColorFormat(),
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		//create pbr task
		m_pbrTask = m_renderer->createRenderTask<Zap::PBRenderer>(m_finalTarget, &m_scene);
		// create line task
		m_lineTask = m_renderer->createRenderTask<Zap::LineRenderTask>(m_finalTarget, std::initializer_list({ (std::weak_ptr<Zap::LineBuffer>)m_spLineBuffer }));

		m_renderer->beginRecord();
		m_renderer->recRenderTask(m_pbrTask);
		m_renderer->recRenderTask(m_lineTask);
		m_renderer->endRecord();
	}

	void HitboxEditor::disableRenderer() {
		m_finalTarget.reset();
		m_lineTask.reset();
		m_pbrTask.reset();
		m_renderer->destroy();
		m_renderer.reset();
	}
}