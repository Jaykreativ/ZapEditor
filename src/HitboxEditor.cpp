#include "HitboxEditor.h"

#include "Zap/Scene/Shape.h"
#include "backends/imgui_impl_vulkan.h"

#include "glm/gtc/quaternion.hpp"

#include <array>

namespace editor {
	HitboxEditor::HitboxEditor(EditorData* pEditorData)
		: m_pEditorData(pEditorData), m_pbrTask(&m_scene)
	{
		m_scene.init();
		m_scene.attachActor(m_actor);
		m_scene.attachActor(m_light);
		m_scene.attachActor(m_camera);

		if(m_pEditorData->selectedActors.size() > 0)
			updateActor();

		m_camera.addTransform(glm::mat4(1));
		m_camera.cmpTransform_setPos({0, 0, 0});
		m_camera.addCamera();
		m_camera.cmpCamera_lookAtCenter();
		glm::mat4 offset = glm::mat4(1);
		offset[3] = {0, 0, -m_camDist, 1};
		m_camera.cmpCamera_setOffset(offset);

		m_light.addTransform(glm::mat4(1));
		m_light.cmpTransform_setPos({0, 0, -5});
		m_light.addLight({1, 1, 1}, m_camDist * m_camDist, 1);

		m_outImage.setFormat(Zap::GlobalSettings::getColorFormat());
		m_outImage.setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
		m_outImage.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		m_outImage.setLayout(VK_IMAGE_LAYOUT_PREINITIALIZED);
		m_outImage.setWidth(1);
		m_outImage.setHeight(1);

		m_outImage.init();
		m_outImage.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_outImage.initView();

		m_debugVertexBuffer = vk::Buffer(0, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		m_debugVertexBuffer.init();
		m_debugVertexBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		m_debugTask.addLineVertexBuffer(&m_debugVertexBuffer);

		m_renderer.setTarget(&m_outImage);
		m_renderer.addRenderTask(&m_pbrTask);
		m_renderer.addRenderTask(&m_debugTask);
		m_renderer.init();
		m_renderer.beginRecord();
		m_renderer.recChangeImageLayout(&m_outImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
		m_renderer.recRenderTemplate(&m_pbrTask);
		m_renderer.recRenderTemplate(&m_debugTask);
		m_renderer.recChangeImageLayout(&m_outImage, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT);
		m_renderer.endRecord();

		m_pbrTask.setViewport(1, 1, 0, 0);
	}

	HitboxEditor::~HitboxEditor() {
		m_renderer.destroy();
		m_debugVertexBuffer.destroy();
		m_outImage.destroy();
		m_scene.destroy();
	}

	std::string HitboxEditor::name() {
		return "HitboxEdtior";
	}

	void HitboxEditor::draw() {
		ImGui::BeginChild("Viewport", { ImGui::GetContentRegionAvail().x / 1.5f, ImGui::GetContentRegionAvail().y }, ImGuiChildFlags_ResizeX);
		{
			// update scene
			{
				glm::vec3 camOffset = m_camera.cmpCamera_getOffset()[3];
				glm::vec3 camPos = m_camera.cmpTransform_getPos();
				m_light.cmpTransform_setPos(camPos + camOffset);
				m_light.cmpLight_setStrength(m_camDist * m_camDist);

				if (m_pEditorData->selectedActors.size() > 0) {
					updateActor();
				}
			}

			// resize
			glm::vec2 imageSize = { m_outImage.getExtent().width, m_outImage.getExtent().height };
			glm::vec2 availSize = { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y };
			if (imageSize != availSize) {
				m_outImage.resize(availSize.x, availSize.y);
				m_pbrTask.setViewport(availSize.x, availSize.y, 0, 0);
				m_renderer.resize();
			}

			// render Lines
			std::vector<Zap::DebugRenderVertex> lines;
			m_scene.getPxDebugVertices(lines);

			m_debugTask.delLineVertexBuffers();
			if (lines.size() > 0) {
				m_debugTask.addLineVertexBuffer(&m_debugVertexBuffer);
				m_debugVertexBuffer.resize(sizeof(Zap::DebugRenderVertex) * lines.size());
				void* rawData;
				m_debugVertexBuffer.map(&rawData);
				memcpy(rawData, lines.data(), m_debugVertexBuffer.getSize());
				m_debugVertexBuffer.unmap();
			}

			// render scene
			m_scene.simulate(1 / 60.f);
			m_scene.update();
			m_pbrTask.updateCamera(m_camera);
			m_debugTask.updateCamera(m_camera);
			m_renderer.render();

			// render Gui
			ImGui::Image((Zap::GuiTexture)m_outImage, { static_cast<float>(m_outImage.getExtent().width), static_cast<float>(m_outImage.getExtent().height) });
			m_isImageHovered = ImGui::IsItemHovered();

			// turn camera
			m_doesCamTurn = m_isImageHovered && ImGui::IsMouseDown(ImGuiMouseButton_Left);

			float mouseX = ImGui::GetMousePos().x;
			float mouseY = ImGui::GetMousePos().y;

			float dx = (mouseX - m_lastX) * m_sensVert;
			float dy = -(mouseY - m_lastY) * m_sensHorz;

			m_lastX = mouseX;
			m_lastY = mouseY;

			glm::mat4 res = m_camera.cmpCamera_getOffset();

			if (m_doesCamTurn) {
				glm::mat4 rot = glm::rotate(glm::mat4(1), dx, glm::vec3(0, 1, 0));

				res[0] = rot * res[0];
				res[1] = rot * res[1];
				res[2] = rot * res[2];

				res = glm::rotate(res, dy, glm::vec3(1, 0, 0));
			}

			res[3] = glm::vec4(glm::vec3(res[2]) * glm::vec3(m_camDist), 1);

			m_camera.cmpCamera_setOffset(res);
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
					case physx::PxGeometryType::eBOX: {
							auto boxGeometry = (Zap::BoxGeometry*)geometry.get();
							auto extent = boxGeometry->getHalfExtents();
							ImGui::DragFloat3("HalfExtents", (float*)(&extent), glm::length(extent)*0.01, 0.001, FLT_MAX, "%.3f", ImGuiSliderFlags_NoRoundToFormat | ImGuiSliderFlags_AlwaysClamp);
							boxGeometry->setHalfExtents(extent);
							break;
					}
					case physx::PxGeometryType::ePLANE:
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
			if (!selectedActor.hasModel()) {
				m_actor.destroy();
				return;
			}
			auto model = selectedActor.cmpModel_getModel();
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
			m_actor.addModel(model);

			m_camDist = glm::max(glm::length(transform * glm::vec4(model.boundMin, 1)), glm::length(transform * glm::vec4(model.boundMax, 1))) * 1.5f;
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
}