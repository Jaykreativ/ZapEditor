#include "Viewport.h"

#include "Zap/EventHandler.h"
#include "Zap/Rendering/RenderTaskTemplate.h"
#include "Zap/Rendering/PBRenderer.h"
#include "Zap/Rendering/RaytracingRenderer.h"
#include "Zap/Rendering/PathTacer.h"
#include "Zap/Rendering/DebugRenderTask.h"
#include "Zap/Scene/Scene.h"
#include "Zap/Scene/Mesh.h"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include <chrono>

namespace editor {
	int forward    = GLFW_KEY_W;// move keybinds to editor::settings
	int backward   = GLFW_KEY_S;
	int right      = GLFW_KEY_D;
	int left       = GLFW_KEY_A;
	int down       = GLFW_KEY_C;
	int up         = GLFW_KEY_SPACE;
	int turnCamera = GLFW_MOUSE_BUTTON_1;

	bool forwardPressed    = false;
	bool backwardPressed   = false;
	bool rightPressed      = false;
	bool leftPressed       = false;
	bool downPressed       = false;
	bool upPressed         = false;
	bool turnCameraPressed = false;

	void mouseButtonCallback(Zap::MouseButtonEvent& params, void* data) {
		if (params.action == GLFW_PRESS) {
			if (params.button == turnCamera)
				turnCameraPressed = true;
		}
		else if (params.action == GLFW_RELEASE) {
			if (params.button == turnCamera)
				turnCameraPressed = false;
		}
	}

	void keyCallback(Zap::KeyEvent& params, void* data) {
		if (params.action == GLFW_PRESS) {
			if (params.key == forward) {
				forwardPressed = true;
			}
			else if (params.key == backward) {
				backwardPressed = true;
			}
			else if (params.key == right) {
				rightPressed = true;
			}
			else if (params.key == left) {
				leftPressed = true;
			}
			else if (params.key == down) {
				downPressed = true;
			}
			else if (params.key == up) {
				upPressed = true;
			}
		}
		else if (params.action == GLFW_RELEASE) {
			if (params.key == forward) {
				forwardPressed = false;
			}
			else if (params.key == backward) {
				backwardPressed = false;
			}
			else if (params.key == right) {
				rightPressed = false;
			}
			else if (params.key == left) {
				leftPressed = false;
			}
			else if (params.key == down) {
				downPressed = false;
			}
			else if (params.key == up) {
				upPressed = false;
			}
		}
	}

	class OutlineRenderTask : public Zap::RenderTaskTemplate {
		// 1. Plain Pass -> PlainColor, DepthStencil
		// 2. Blur Pass -> BlurColor
		// 3. Outline Pass -> Target, DepthStencil
	public:
		OutlineRenderTask(Zap::Scene* pScene, std::vector<Zap::Actor>& actors)
			: RenderTaskTemplate(pScene), m_actors(actors)
		{}

		~OutlineRenderTask() {}

		void updateCamera(Zap::Actor cam) {
			void* rawData;
			m_uniformBuffer.map(&rawData);
			UBO* data = (UBO*)rawData;
			data->perspective = cam.cmpCamera_getPerspective(m_viewport.width / m_viewport.height);
			data->view = cam.cmpCamera_getView();
			m_uniformBuffer.unmap();
		}

	private:
		std::vector<Zap::Actor>& m_actors;

		vk::Image m_plainColorImage_blurB;
		vk::Image m_plainDepthStencilImage;
		vk::Image m_blurA;

		std::vector<vk::Framebuffer> m_framebuffers;

		struct UBO {
			glm::mat4 perspective;
			glm::mat4 view;
		};
		vk::Buffer m_uniformBuffer;

		vk::DescriptorPool m_descriptorPool;
		vk::DescriptorSet m_plainDescriptorSet;
		vk::DescriptorSet m_blurADescriptorSet;
		vk::DescriptorSet m_blurBDescriptorSet;
		vk::DescriptorSet m_outlineDescriptorSet;

		vk::RenderPass m_renderPass;

		vk::Shader m_plainVertexShader;
		vk::Shader m_plainFragmentShader;

		vk::Shader m_blurVertexShader;
		vk::Shader m_blurAFragmentShader;
		vk::Shader m_blurBFragmentShader;
		vk::Shader m_outlineFragmentShader;

		vk::Pipeline m_plainPipeline;
		vk::Pipeline m_blurAPipeline;
		vk::Pipeline m_blurBPipeline;
		vk::Pipeline m_outlinePipeline;

		VkViewport m_viewport;
		VkRect2D m_scissor;

		void init(uint32_t width, uint32_t height, uint32_t imageCount) {
			Zap::Base* base = Zap::Base::getBase();

			/*Viewport & Scissor*/
			{
				m_viewport.x = 0;
				m_viewport.y = 0;
				m_viewport.width = width;
				m_viewport.height = height;
				m_viewport.minDepth = 0;
				m_viewport.maxDepth = 1;

				m_scissor.extent.width = width;
				m_scissor.extent.height = height;
				m_scissor.offset.x = 0;
				m_scissor.offset.y = 0;
			}

			/*UniformBuffer*/
			{
				m_uniformBuffer = vk::Buffer(sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
				m_uniformBuffer.init();
				m_uniformBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			}

			/*Images*/
			m_plainColorImage_blurB = vk::Image();
			m_plainDepthStencilImage = vk::Image();
			{
				m_plainColorImage_blurB.setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
				m_plainColorImage_blurB.setExtent({ width, height, 1 });
				m_plainColorImage_blurB.setFormat(Zap::GlobalSettings::getColorFormat());
				m_plainColorImage_blurB.setLayout(VK_IMAGE_LAYOUT_UNDEFINED);
				m_plainColorImage_blurB.setType(VK_IMAGE_TYPE_2D);
				m_plainColorImage_blurB.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
				m_plainColorImage_blurB.init();
				m_plainColorImage_blurB.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				m_plainColorImage_blurB.initView();

				// Set layout to the the layout expected by the renderpass
				m_plainColorImage_blurB.changeLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

				m_plainDepthStencilImage.setAspect(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
				m_plainDepthStencilImage.setExtent({ width, height, 1 });
				m_plainDepthStencilImage.setFormat(Zap::GlobalSettings::getDepthStencilFormat());
				m_plainDepthStencilImage.setLayout(VK_IMAGE_LAYOUT_UNDEFINED);
				m_plainDepthStencilImage.setType(VK_IMAGE_TYPE_2D);
				m_plainDepthStencilImage.setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
				m_plainDepthStencilImage.init();
				m_plainDepthStencilImage.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				m_plainDepthStencilImage.initView();

				m_plainDepthStencilImage.changeLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
			}
			m_blurA = vk::Image();
			{
				m_blurA.setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
				m_blurA.setExtent({ width, height, 1 });
				m_blurA.setFormat(Zap::GlobalSettings::getColorFormat());
				m_blurA.setLayout(VK_IMAGE_LAYOUT_UNDEFINED);
				m_blurA.setType(VK_IMAGE_TYPE_2D);
				m_blurA.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
				m_blurA.init();
				m_blurA.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				m_blurA.initView();

				// Set layout to the the layout expected by the renderpass
				m_blurA.changeLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
			}

			/*Descriptors*/
			m_descriptorPool = vk::DescriptorPool();
			m_plainDescriptorSet = vk::DescriptorSet();
			{
				vk::DescriptorBufferInfo uniformBufferInfo{};
				uniformBufferInfo.pBuffer = &m_uniformBuffer;
				uniformBufferInfo.offset = 0;
				uniformBufferInfo.range = m_uniformBuffer.getSize();

				vk::Descriptor uniformBufferDescriptor{};
				uniformBufferDescriptor.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				uniformBufferDescriptor.stages = VK_SHADER_STAGE_VERTEX_BIT;
				uniformBufferDescriptor.binding = 0;
				uniformBufferDescriptor.bufferInfos = { uniformBufferInfo };

				m_plainDescriptorSet.addDescriptor(uniformBufferDescriptor);

				vk::DescriptorBufferInfo perMeshBufferInfo;
				perMeshBufferInfo.pBuffer = getScenePerMeshInstanceBuffer();
				perMeshBufferInfo.offset = 0;
				perMeshBufferInfo.range = getScenePerMeshInstanceBuffer()->getSize();

				vk::Descriptor perMeshBufferDescriptor{};
				perMeshBufferDescriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				perMeshBufferDescriptor.stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
				perMeshBufferDescriptor.binding = 1;
				perMeshBufferDescriptor.bufferInfos = { perMeshBufferInfo };

				m_plainDescriptorSet.addDescriptor(perMeshBufferDescriptor);

				m_descriptorPool.addDescriptorSet(m_plainDescriptorSet);

				getRegistery()->connect(getScenePerMeshInstanceBuffer(), &m_plainDescriptorSet, updatePerMeshBufferDescriptorSet);
			}
			m_blurADescriptorSet = vk::DescriptorSet();
			{
				vk::DescriptorImageInfo srcImageInfo{};
				srcImageInfo.pSampler = nullptr;
				srcImageInfo.pImage = &m_plainColorImage_blurB;
				srcImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

				vk::Descriptor srcImageDescriptor{};
				srcImageDescriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				srcImageDescriptor.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
				srcImageDescriptor.binding = 0;
				srcImageDescriptor.imageInfos = { srcImageInfo };

				m_blurADescriptorSet.addDescriptor(srcImageDescriptor);

				m_descriptorPool.addDescriptorSet(m_blurADescriptorSet);
			}
			m_blurBDescriptorSet = vk::DescriptorSet();
			{
				vk::DescriptorImageInfo srcImageInfo{};
				srcImageInfo.pSampler = nullptr;
				srcImageInfo.pImage = &m_blurA;
				srcImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

				vk::Descriptor srcImageDescriptor{};
				srcImageDescriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				srcImageDescriptor.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
				srcImageDescriptor.binding = 0;
				srcImageDescriptor.imageInfos = { srcImageInfo };

				m_blurBDescriptorSet.addDescriptor(srcImageDescriptor);

				m_descriptorPool.addDescriptorSet(m_blurBDescriptorSet);
			}
			m_outlineDescriptorSet = vk::DescriptorSet();
			{
				vk::DescriptorImageInfo srcImageInfo{};
				srcImageInfo.pSampler = nullptr;
				srcImageInfo.pImage = &m_plainColorImage_blurB;
				srcImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				vk::Descriptor srcImageDescriptor{};
				srcImageDescriptor.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
				srcImageDescriptor.stages = VK_SHADER_STAGE_FRAGMENT_BIT;
				srcImageDescriptor.binding = 0;
				srcImageDescriptor.imageInfos = { srcImageInfo };

				m_outlineDescriptorSet.addDescriptor(srcImageDescriptor);

				m_descriptorPool.addDescriptorSet(m_outlineDescriptorSet);
			}

			m_descriptorPool.init();

			m_plainDescriptorSet.init();
			m_plainDescriptorSet.allocate();
			m_plainDescriptorSet.update();

			m_blurADescriptorSet.init();
			m_blurADescriptorSet.allocate();
			m_blurADescriptorSet.update();

			m_blurBDescriptorSet.init();
			m_blurBDescriptorSet.allocate();
			m_blurBDescriptorSet.update();

			m_outlineDescriptorSet.init();
			m_outlineDescriptorSet.allocate();
			m_outlineDescriptorSet.update();

			/*RenderPass*/
			m_renderPass = vk::RenderPass();
			{
				VkAttachmentDescription plainColorAttachment;
				{
					plainColorAttachment.flags = 0;
					plainColorAttachment.format = m_plainColorImage_blurB.getFormat();
					plainColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
					plainColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					plainColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					plainColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					plainColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					plainColorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					plainColorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}
				m_renderPass.addAttachmentDescription(plainColorAttachment);

				VkAttachmentDescription plainDepthStencilAttachment;
				{
					plainDepthStencilAttachment.flags = 0;
					plainDepthStencilAttachment.format = m_plainDepthStencilImage.getFormat();
					plainDepthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
					plainDepthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					plainDepthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					plainDepthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					plainDepthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
					plainDepthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					plainDepthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				}
				m_renderPass.addAttachmentDescription(plainDepthStencilAttachment);

				VkAttachmentDescription blurAColorAttachment;
				{
					blurAColorAttachment.flags = 0;
					blurAColorAttachment.format = m_blurA.getFormat();
					blurAColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
					blurAColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					blurAColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					blurAColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					blurAColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					blurAColorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					blurAColorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}
				m_renderPass.addAttachmentDescription(blurAColorAttachment);

				VkAttachmentDescription targetColorAttachment;
				{
					targetColorAttachment.flags = 0;
					targetColorAttachment.format = Zap::GlobalSettings::getColorFormat();
					targetColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
					targetColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
					targetColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					targetColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					targetColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					targetColorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					targetColorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}
				m_renderPass.addAttachmentDescription(targetColorAttachment);

				VkAttachmentReference* pPlainColorAttachmentReference;
				{
					VkAttachmentReference tmp;
					tmp.attachment = 0;
					tmp.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					pPlainColorAttachmentReference = &tmp;

					m_renderPass.addAttachmentReference(&pPlainColorAttachmentReference);
				}

				VkAttachmentReference* pPlainColorAttachmentReadReference;
				{
					VkAttachmentReference tmp;
					tmp.attachment = 0;
					tmp.layout = VK_IMAGE_LAYOUT_GENERAL;
					pPlainColorAttachmentReadReference = &tmp;

					m_renderPass.addAttachmentReference(&pPlainColorAttachmentReadReference);
				}

				VkAttachmentReference* pPlainColorAttachmentInputReference;
				{
					VkAttachmentReference tmp;
					tmp.attachment = 0;
					tmp.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					pPlainColorAttachmentInputReference = &tmp;

					m_renderPass.addAttachmentReference(&pPlainColorAttachmentInputReference);
				}

				VkAttachmentReference* pPlainDepthStencilAttachmentReference;
				{
					VkAttachmentReference tmp;
					tmp.attachment = 1;
					tmp.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					pPlainDepthStencilAttachmentReference = &tmp;

					m_renderPass.addAttachmentReference(&pPlainDepthStencilAttachmentReference);
				}

				VkAttachmentReference* pBlurAColorAttachmentReference;
				{
					VkAttachmentReference tmp;
					tmp.attachment = 2;
					tmp.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					pBlurAColorAttachmentReference = &tmp;

					m_renderPass.addAttachmentReference(&pBlurAColorAttachmentReference);
				}

				VkAttachmentReference* pBlurAColorAttachmentReadReference;
				{
					VkAttachmentReference tmp;
					tmp.attachment = 2;
					tmp.layout = VK_IMAGE_LAYOUT_GENERAL;
					pBlurAColorAttachmentReadReference = &tmp;

					m_renderPass.addAttachmentReference(&pBlurAColorAttachmentReadReference);
				}

				VkAttachmentReference* pTargetColorAttachmentReference;
				{
					VkAttachmentReference tmp;
					tmp.attachment = 3;
					tmp.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					pTargetColorAttachmentReference = &tmp;

					m_renderPass.addAttachmentReference(&pTargetColorAttachmentReference);
				}

				// Subpass Descriptions
				VkSubpassDescription plainSubpassDescription;
				{
					plainSubpassDescription.flags = 0;
					plainSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
					plainSubpassDescription.inputAttachmentCount = 0;
					plainSubpassDescription.pInputAttachments = nullptr;
					plainSubpassDescription.colorAttachmentCount = 1;
					plainSubpassDescription.pColorAttachments = pPlainColorAttachmentReference;
					plainSubpassDescription.pResolveAttachments = nullptr;
					plainSubpassDescription.pDepthStencilAttachment = pPlainDepthStencilAttachmentReference;
					plainSubpassDescription.preserveAttachmentCount = 0;
					plainSubpassDescription.pPreserveAttachments = nullptr;
				}
				m_renderPass.addSubpassDescription(plainSubpassDescription);

				VkSubpassDescription blurASubpassDescription;
				{
					blurASubpassDescription.flags = 0;
					blurASubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
					blurASubpassDescription.inputAttachmentCount = 1;
					blurASubpassDescription.pInputAttachments = pPlainColorAttachmentReadReference;
					blurASubpassDescription.colorAttachmentCount = 1;
					blurASubpassDescription.pColorAttachments = pBlurAColorAttachmentReference;
					blurASubpassDescription.pResolveAttachments = nullptr;
					blurASubpassDescription.pDepthStencilAttachment = nullptr;
					blurASubpassDescription.preserveAttachmentCount = 1;
					uint32_t i = 1;
					blurASubpassDescription.pPreserveAttachments = &i;
				}
				m_renderPass.addSubpassDescription(blurASubpassDescription);

				VkSubpassDescription blurBSubpassDescription;
				{
					blurBSubpassDescription.flags = 0;
					blurBSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
					blurBSubpassDescription.inputAttachmentCount = 1;
					blurBSubpassDescription.pInputAttachments = pBlurAColorAttachmentReadReference;
					blurBSubpassDescription.colorAttachmentCount = 1;
					blurBSubpassDescription.pColorAttachments = pPlainColorAttachmentReference;
					blurBSubpassDescription.pResolveAttachments = nullptr;
					blurBSubpassDescription.pDepthStencilAttachment = nullptr;
					blurBSubpassDescription.preserveAttachmentCount = 1;
					uint32_t i = 1;
					blurBSubpassDescription.pPreserveAttachments = &i;
				}
				m_renderPass.addSubpassDescription(blurBSubpassDescription);

				VkSubpassDescription outlineSubpassDescription;
				{
					outlineSubpassDescription.flags = 0;
					outlineSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
					outlineSubpassDescription.inputAttachmentCount = 1;
					outlineSubpassDescription.pInputAttachments = pPlainColorAttachmentInputReference;
					outlineSubpassDescription.colorAttachmentCount = 1;
					outlineSubpassDescription.pColorAttachments = pTargetColorAttachmentReference;
					outlineSubpassDescription.pResolveAttachments = nullptr;
					outlineSubpassDescription.pDepthStencilAttachment = pPlainDepthStencilAttachmentReference;
					outlineSubpassDescription.preserveAttachmentCount = 0;
					outlineSubpassDescription.pPreserveAttachments = nullptr;
				}
				m_renderPass.addSubpassDescription(outlineSubpassDescription);

				//Subpass Dependencies
				VkSubpassDependency plainBlurASubpassDependency;
				{
					plainBlurASubpassDependency.srcSubpass = 0;
					plainBlurASubpassDependency.dstSubpass = 1;
					plainBlurASubpassDependency.srcStageMask =
						VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
						VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
						VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
					plainBlurASubpassDependency.dstStageMask =
						VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
						VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					plainBlurASubpassDependency.srcAccessMask =
						VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
						VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					plainBlurASubpassDependency.dstAccessMask =
						VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
						VK_ACCESS_SHADER_READ_BIT;
					plainBlurASubpassDependency.dependencyFlags = 0;
				}
				m_renderPass.addSubpassDependency(plainBlurASubpassDependency);

				VkSubpassDependency blurABlurBSubpassDependency;
				{
					blurABlurBSubpassDependency.srcSubpass = 1;
					blurABlurBSubpassDependency.dstSubpass = 2;
					blurABlurBSubpassDependency.srcStageMask =
						VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
						VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					blurABlurBSubpassDependency.dstStageMask =
						VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
						VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					blurABlurBSubpassDependency.srcAccessMask =
						VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
						VK_ACCESS_SHADER_READ_BIT;
					blurABlurBSubpassDependency.dstAccessMask =
						VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
						VK_ACCESS_SHADER_READ_BIT;
					blurABlurBSubpassDependency.dependencyFlags = 0;
				}
				m_renderPass.addSubpassDependency(blurABlurBSubpassDependency);

				VkSubpassDependency blurBOutlineSubpassDependency;
				{
					blurBOutlineSubpassDependency.srcSubpass = 2;
					blurBOutlineSubpassDependency.dstSubpass = 3;
					blurBOutlineSubpassDependency.srcStageMask =
						VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
						VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					blurBOutlineSubpassDependency.dstStageMask =
						VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
						VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
						VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
						VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
					blurBOutlineSubpassDependency.srcAccessMask =
						VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
						VK_ACCESS_SHADER_READ_BIT;
					blurBOutlineSubpassDependency.dstAccessMask =
						VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
						VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
						VK_ACCESS_SHADER_READ_BIT;
					blurBOutlineSubpassDependency.dependencyFlags = 0;
				}
				m_renderPass.addSubpassDependency(blurBOutlineSubpassDependency);

				m_renderPass.init();
			}

			/*Framebuffers*/
			m_framebuffers.resize(imageCount);
			RenderTaskTemplate::initTargetDependencies();

			/*Shader*/
#ifdef _DEBUG
			static bool areShadersCompiled = false;
			if (!areShadersCompiled) {
				vk::Shader::compile("Shader/src/", { "outlinePlain.vert", "outlinePlain.frag", "outlineBlur.vert", "outlineBlurA.frag", "outlineBlurB.frag", "outline.frag" }, {"./"});
				areShadersCompiled = true;
			}
#endif
			m_plainVertexShader = vk::Shader();
			m_plainFragmentShader = vk::Shader();
			{
				m_plainVertexShader.setStage(VK_SHADER_STAGE_VERTEX_BIT);
				m_plainVertexShader.setPath("outlinePlain.vert.spv");

				m_plainFragmentShader.setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
				m_plainFragmentShader.setPath("outlinePlain.frag.spv");

				m_plainVertexShader.init();
				m_plainFragmentShader.init();
			}
			m_blurVertexShader = vk::Shader();
			m_blurAFragmentShader = vk::Shader();
			m_blurBFragmentShader = vk::Shader();
			{
				m_blurVertexShader.setStage(VK_SHADER_STAGE_VERTEX_BIT);
				m_blurVertexShader.setPath("outlineBlur.vert.spv");

				m_blurAFragmentShader.setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
				m_blurAFragmentShader.setPath("outlineBlurA.frag.spv");

				m_blurBFragmentShader.setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
				m_blurBFragmentShader.setPath("outlineBlurB.frag.spv");

				m_blurVertexShader.init();
				m_blurAFragmentShader.init();
				m_blurBFragmentShader.init();
			}
			m_outlineFragmentShader = vk::Shader();
			{
				m_outlineFragmentShader.setStage(VK_SHADER_STAGE_FRAGMENT_BIT);
				m_outlineFragmentShader.setPath("outline.frag.spv");

				m_outlineFragmentShader.init();
			}

			/*Pipelines*/
			m_plainPipeline = vk::Pipeline();
			{
				m_plainPipeline.addShader(m_plainVertexShader.getShaderStage());
				m_plainPipeline.addShader(m_plainFragmentShader.getShaderStage());

				m_plainPipeline.addDescriptorSetLayout(m_plainDescriptorSet.getVkDescriptorSetLayout());
				m_plainPipeline.addVertexInputAttrubuteDescription(Vertex::getVertexInputAttributeDescriptions()[0]);
				m_plainPipeline.addVertexInputBindingDescription(Vertex::getVertexInputBindingDescription());
				m_plainPipeline.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
				m_plainPipeline.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
				m_plainPipeline.addViewport(m_viewport);
				m_plainPipeline.addScissor(m_scissor);
				m_plainPipeline.setRenderPass(m_renderPass);
				m_plainPipeline.setSubpassIndex(0);
				m_plainPipeline.enableDepthTest();
				m_blurAPipeline.disableBlending();

				VkStencilOpState stencilOpState{};
				stencilOpState.failOp = VK_STENCIL_OP_KEEP;
				stencilOpState.passOp = VK_STENCIL_OP_REPLACE;
				stencilOpState.depthFailOp = VK_STENCIL_OP_KEEP;
				stencilOpState.compareOp = VK_COMPARE_OP_ALWAYS;
				stencilOpState.compareMask = 0x00;
				stencilOpState.writeMask = 0xFF;
				stencilOpState.reference = 0xFF;

				m_plainPipeline.setStencilOpStates(stencilOpState);
				m_plainPipeline.enableStencilTest();

				VkPushConstantRange pushConstantRange{};
				pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				pushConstantRange.offset = 0;
				pushConstantRange.size = sizeof(uint32_t);

				m_plainPipeline.addPushConstantRange(pushConstantRange);

				m_plainPipeline.init();
			}
			m_blurAPipeline = vk::Pipeline();
			{
				m_blurAPipeline.addShader(m_blurVertexShader.getShaderStage());
				m_blurAPipeline.addShader(m_blurAFragmentShader.getShaderStage());

				m_blurAPipeline.addDescriptorSetLayout(m_blurADescriptorSet.getVkDescriptorSetLayout());
				m_blurAPipeline.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
				m_blurAPipeline.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
				m_blurAPipeline.addViewport(m_viewport);
				m_blurAPipeline.addScissor(m_scissor);
				m_blurAPipeline.setRenderPass(m_renderPass);
				m_blurAPipeline.setSubpassIndex(1);
				m_blurAPipeline.disableBlending();

				m_blurAPipeline.init();
			}
			m_blurBPipeline = vk::Pipeline();
			{
				m_blurBPipeline.addShader(m_blurVertexShader.getShaderStage());
				m_blurBPipeline.addShader(m_blurBFragmentShader.getShaderStage());

				m_blurBPipeline.addDescriptorSetLayout(m_blurBDescriptorSet.getVkDescriptorSetLayout());
				m_blurBPipeline.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
				m_blurBPipeline.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
				m_blurBPipeline.addViewport(m_viewport);
				m_blurBPipeline.addScissor(m_scissor);
				m_blurBPipeline.setRenderPass(m_renderPass);
				m_blurBPipeline.setSubpassIndex(2);
				m_blurBPipeline.disableBlending();

				m_blurBPipeline.init();
			}
			m_outlinePipeline = vk::Pipeline();
			{
				m_outlinePipeline.addShader(m_blurVertexShader.getShaderStage());
				m_outlinePipeline.addShader(m_outlineFragmentShader.getShaderStage());

				m_outlinePipeline.addDescriptorSetLayout(m_outlineDescriptorSet.getVkDescriptorSetLayout());
				m_outlinePipeline.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
				m_outlinePipeline.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
				m_outlinePipeline.addViewport(m_viewport);
				m_outlinePipeline.addScissor(m_scissor);
				m_outlinePipeline.setRenderPass(m_renderPass);
				m_outlinePipeline.setSubpassIndex(3);

				VkStencilOpState stencilOpState{};
				stencilOpState.failOp = VK_STENCIL_OP_KEEP;
				stencilOpState.passOp = VK_STENCIL_OP_KEEP;
				stencilOpState.depthFailOp = VK_STENCIL_OP_KEEP;
				stencilOpState.compareOp = VK_COMPARE_OP_NOT_EQUAL;
				stencilOpState.compareMask = 0xFF;
				stencilOpState.writeMask = 0xFF;
				stencilOpState.reference = 0xFF;

				m_outlinePipeline.setStencilOpStates(stencilOpState);
				m_outlinePipeline.enableStencilTest();
				m_outlinePipeline.enableBlending();

				m_outlinePipeline.init();
			}
		}

		void initTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex) {
			m_framebuffers[imageIndex].setWidth(width);
			m_framebuffers[imageIndex].setHeight(height);
			m_framebuffers[imageIndex].addAttachment(m_plainColorImage_blurB.getVkImageView());
			m_framebuffers[imageIndex].addAttachment(m_plainDepthStencilImage.getVkImageView());
			m_framebuffers[imageIndex].addAttachment(m_blurA.getVkImageView());
			m_framebuffers[imageIndex].addAttachment(pTarget->getVkImageView());
			m_framebuffers[imageIndex].setRenderPass(m_renderPass);
			m_framebuffers[imageIndex].init();
		}

		void resize(uint32_t width, uint32_t height, uint32_t imageCount) {
			/*Viewport & Scissor*/
			m_viewport.width = width;
			m_viewport.height = height;

			m_scissor.extent.width = width;
			m_scissor.extent.height = height;

			m_plainColorImage_blurB.resize(width, height);
			m_plainDepthStencilImage.resize(width, height);
			m_blurA.resize(width, height);

			m_blurADescriptorSet.update(); // update descriptor #0 for resize
			m_blurBDescriptorSet.update(); // update descriptor #0 for resize
			m_outlineDescriptorSet.update();

			RenderTaskTemplate::resizeTargetDependencies();
		}

		void resizeTargetDependencies(uint32_t width, uint32_t height, uint32_t imageCount, vk::Image* pTarget, uint32_t imageIndex) {
			m_framebuffers[imageIndex].setWidth(width);
			m_framebuffers[imageIndex].setHeight(height);
			m_framebuffers[imageIndex].delAttachment(0);
			m_framebuffers[imageIndex].delAttachment(0);
			m_framebuffers[imageIndex].delAttachment(0);
			m_framebuffers[imageIndex].delAttachment(0);
			m_framebuffers[imageIndex].addAttachment(m_plainColorImage_blurB.getVkImageView());
			m_framebuffers[imageIndex].addAttachment(m_plainDepthStencilImage.getVkImageView());
			m_framebuffers[imageIndex].addAttachment(m_blurA.getVkImageView());
			m_framebuffers[imageIndex].addAttachment(pTarget->getVkImageView());
			m_framebuffers[imageIndex].update();
		}

		void destroy() {
			m_plainPipeline.destroy();
			m_blurAPipeline.destroy();
			m_blurBPipeline.destroy();
			m_outlinePipeline.destroy();
			m_plainVertexShader.destroy();
			m_plainFragmentShader.destroy();
			m_blurVertexShader.destroy();
			m_blurAFragmentShader.destroy();
			m_blurBFragmentShader.destroy();
			m_outlineFragmentShader.destroy();
			for (auto& framebuffer : m_framebuffers)
				framebuffer.destroy();
			m_framebuffers.clear();
			m_renderPass.destroy();
			m_plainColorImage_blurB.destroy();
			m_plainDepthStencilImage.destroy();
			m_blurA.destroy();
			m_plainDescriptorSet.destroy();
			m_blurADescriptorSet.destroy();
			m_blurBDescriptorSet.destroy();
			m_outlineDescriptorSet.destroy();
			m_descriptorPool.destroy();
			m_uniformBuffer.destroy();
		}

		void beforeRender(vk::Image* pTarget, uint32_t imageIndex) {}

		void afterRender(vk::Image* pTarget, uint32_t imageIndex) {}

		void recordCommands(const vk::CommandBuffer* cmd, vk::Image* pTarget, uint32_t imageIndex) {
			VkRenderPassBeginInfo plainPassBeginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr};
			plainPassBeginInfo.renderPass = m_renderPass;
			plainPassBeginInfo.framebuffer = m_framebuffers[imageIndex];
			plainPassBeginInfo.renderArea = m_scissor;
			
			VkClearValue plainColorClearValue = { 0, 0, 0, 0 };
			VkClearValue plainDepthStencilClearValue = { 1, 0 };
			VkClearValue blurAClearValue = { 0, 0, 0, 0 };
			std::vector<VkClearValue> clearValues = {
				plainColorClearValue,
				plainDepthStencilClearValue,
				blurAClearValue
			};

			plainPassBeginInfo.clearValueCount = clearValues.size();
			plainPassBeginInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(*cmd, &plainPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			
			vkCmdBindPipeline(*cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_plainPipeline);

			vkCmdSetViewport(*cmd, 0, 1, &m_viewport);
			vkCmdSetScissor(*cmd, 0, 1, &m_scissor);

			VkDescriptorSet boundSets[] = { m_plainDescriptorSet };
			vkCmdBindDescriptorSets(*cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_plainPipeline.getVkPipelineLayout(), 0, 1, boundSets, 0, nullptr);

			for (auto actor : m_actors) {
				if (actor.hasModel()) {
					auto* pModel = getActorModel(actor);
					for (Zap::Mesh mesh : pModel->meshes) {
						VkDeviceSize offsets[] = { 0 };
						VkBuffer vertexBuffer = *mesh.getVertexBuffer();
						vkCmdBindVertexBuffers(*cmd, 0, 1, &vertexBuffer, offsets);
						vkCmdBindIndexBuffer(*cmd, *mesh.getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

						uint32_t i = getMeshInstanceIndex(actor, mesh);
						vkCmdPushConstants(*cmd, m_plainPipeline.getVkPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &i);

						vkCmdDrawIndexed(*cmd, mesh.getIndexBuffer()->getSize() / sizeof(uint32_t), 1, 0, 0, 0);
					}
				}
			}

			vkCmdNextSubpass(*cmd, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(*cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_blurAPipeline);

			vkCmdSetViewport(*cmd, 0, 1, &m_viewport);
			vkCmdSetScissor(*cmd, 0, 1, &m_scissor);

			VkDescriptorSet blurABoundSets[] = { m_blurADescriptorSet };
			vkCmdBindDescriptorSets(*cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_blurAPipeline.getVkPipelineLayout(), 0, 1, blurABoundSets, 0, nullptr);
			
			vkCmdDraw(*cmd, 6, 1, 0, 0);

			vkCmdNextSubpass(*cmd, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(*cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_blurBPipeline);

			vkCmdSetViewport(*cmd, 0, 1, &m_viewport);
			vkCmdSetScissor(*cmd, 0, 1, &m_scissor);

			VkDescriptorSet blurBBoundSets[] = { m_blurBDescriptorSet };
			vkCmdBindDescriptorSets(*cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_blurBPipeline.getVkPipelineLayout(), 0, 1, blurBBoundSets, 0, nullptr);
			
			vkCmdDraw(*cmd, 6, 1, 0, 0);

			vkCmdNextSubpass(*cmd, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(*cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_outlinePipeline);

			vkCmdSetViewport(*cmd, 0, 1, &m_viewport);
			vkCmdSetScissor(*cmd, 0, 1, &m_scissor);

			VkDescriptorSet outlineBoundSets[] = { m_outlineDescriptorSet };
			vkCmdBindDescriptorSets(*cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_outlinePipeline.getVkPipelineLayout(), 0, 1, outlineBoundSets, 0, nullptr);
			
			vkCmdDraw(*cmd, 6, 1, 0, 0);

			vkCmdEndRenderPass(*cmd);
		}

		static void updatePerMeshBufferDescriptorSet(vk::Registerable* obj, vk::Registerable* dependency, vk::RegisteryFunction func) {
			if (func != vk::eUPDATE)
				return;

			vk::Buffer* pBuffer = (vk::Buffer*)obj;
			vk::DescriptorSet* pDescriptorSet = (vk::DescriptorSet*)dependency;
			auto descriptor = pDescriptorSet->getDescriptor(1);
			descriptor.bufferInfos[0].range = pBuffer->getSize();
			pDescriptorSet->setDescriptor(1, descriptor);

			pDescriptorSet->update();
		}
	};

	Viewport::Viewport(Zap::Scene* pScene, Zap::Window* pWindow, std::vector<Zap::Actor>& selectedActors)
		: m_pScene(pScene), m_pWindow(pWindow), m_selectedActors(selectedActors)
	{
		m_pWindow->getKeyEventHandler()->addCallback(keyCallback);
		m_pWindow->getMouseButtonEventHandler()->addCallback(mouseButtonCallback);
		m_pWindow->getCursorPosEventHandler()->addCallback(Viewport::cursorPositionCallback, this);

		m_outImage.setFormat(Zap::GlobalSettings::getColorFormat());
		m_outImage.setAspect(VK_IMAGE_ASPECT_COLOR_BIT);
		m_outImage.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		m_outImage.setLayout(VK_IMAGE_LAYOUT_PREINITIALIZED);
		m_outImage.setWidth(1);
		m_outImage.setHeight(1);
		
		m_outImage.init();
		m_outImage.allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_outImage.initView();

		m_pPBRender = new Zap::PBRenderer(pScene);
		m_pPBRender->setViewport(1, 1, 0, 0);

		m_pOutlineRenderTask = new OutlineRenderTask(pScene, m_selectedActors);
		m_pDebugRenderTask = new Zap::DebugRenderTask();

		m_debugVertexBuffer = vk::Buffer(0, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		m_debugVertexBuffer.init();
		m_debugVertexBuffer.allocate(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		m_pDebugRenderTask->addLineVertexBuffer(&m_debugVertexBuffer);

		m_renderer.setTarget(&m_outImage);
		m_renderer.addRenderTask(m_pPBRender);
		m_renderer.addRenderTask(m_pOutlineRenderTask);
		m_renderer.addRenderTask(m_pDebugRenderTask);

		if (Zap::Base::getBase()->getSettings()->enableRaytracing) {
			m_pRTRender = new Zap::RaytracingRenderer(pScene);
			m_renderer.addRenderTask(m_pRTRender);
		
			m_pPathTracer = new Zap::PathTracer(pScene);
			m_renderer.addRenderTask(m_pPathTracer);
		}

		m_renderer.beginRecord();
		m_renderer.recRenderTemplate(m_pPBRender);
		if (m_settings.enableOutlines)
			m_renderer.recRenderTemplate(m_pOutlineRenderTask);
		m_renderer.recRenderTemplate(m_pDebugRenderTask);
		m_renderer.endRecord();

		m_renderer.init();

		m_sampler.init();

		m_imageDescriptorSet = ImGui_ImplVulkan_AddTexture(m_sampler, m_outImage.getVkImageView(), VK_IMAGE_LAYOUT_GENERAL);

		m_camera = Zap::Actor();
		pScene->attachActor(m_camera);
		m_camera.addTransform(glm::mat4(1));
		m_camera.addCamera();

		m_transformEditScene = Zap::Scene();
		m_transformEditScene.init();
		m_transformEditScene.attachActor(m_transformX);
		m_transformEditScene.attachActor(m_transformY);
		m_transformEditScene.attachActor(m_transformZ);

		m_transformMaterial = new Zap::PhysicsMaterial(1, 0.5, 0.2);
		Zap::BoxGeometry box = Zap::BoxGeometry(glm::vec3(0.5, 0.05, 0.05));
		{
			Zap::Shape shape = Zap::Shape(box, *m_transformMaterial, true, glm::mat4(1), physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE);
			m_transformX.addTransform(glm::mat4(1));
			m_transformX.addRigidDynamic(shape);
		}
		{
			Zap::Shape shape = Zap::Shape(box, *m_transformMaterial, true, glm::mat4(1), physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE);
			m_transformY.addTransform(glm::mat4(1));
			m_transformY.cmpTransform_rotateZ(90);
			m_transformY.addRigidDynamic(shape);
		}
		{
			Zap::Shape shape = Zap::Shape(box, *m_transformMaterial, true, glm::mat4(1), physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE);
			m_transformZ.addTransform(glm::mat4(1));
			m_transformZ.cmpTransform_rotateY(-90);
			m_transformZ.addRigidDynamic(shape);
		}
	}

	Viewport::~Viewport() {
		m_renderer.destroy();
		m_sampler.destroy();
		m_outImage.destroy();
		m_debugVertexBuffer.destroy();
		m_transformEditScene.destroy();

		m_pWindow->getKeyEventHandler()->removeCallback(keyCallback);
		m_pWindow->getMouseButtonEventHandler()->removeCallback(mouseButtonCallback);
		m_pWindow->getCursorPosEventHandler()->removeCallback(Viewport::cursorPositionCallback, this);
	}

	std::string Viewport::name() {
		return "Viewport";
	}

	void Viewport::move(float dTime) {
		if (!m_isFocused) return;
		if (forwardPressed) {
			auto res = m_camera.cmpTransform_getTransform();
			glm::vec3 vec = res[2];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * 2.0f, 1);
			m_camera.cmpTransform_setTransform(res);
		}
		if (backwardPressed) {
			auto res = m_camera.cmpTransform_getTransform();
			glm::vec3 vec = -res[2];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * 2.0f, 1);
			m_camera.cmpTransform_setTransform(res);
		}
		if (rightPressed) {
			auto res = m_camera.cmpTransform_getTransform();
			glm::vec3 vec = res[0];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * 2.0f, 1);
			m_camera.cmpTransform_setTransform(res);
		}
		if (leftPressed) {
			auto res = m_camera.cmpTransform_getTransform();
			glm::vec3 vec = -res[0];
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::normalize(glm::vec3{ vec.x, 0, vec.z }) * dTime * 2.0f, 1);
			m_camera.cmpTransform_setTransform(res);
		}
		if (downPressed) {
			auto res = m_camera.cmpTransform_getTransform();
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::vec3{ 0, -2, 0 }*dTime, 1);
			m_camera.cmpTransform_setTransform(res);
		}
		if (upPressed) {
			auto res = m_camera.cmpTransform_getTransform();
			res[3] = glm::vec4(glm::vec3(res[3]) + glm::vec3{ 0, 2, 0 }*dTime, 1);
			m_camera.cmpTransform_setTransform(res);
		}
	}

	float sensitivityX = 0.2;
	float sensitivityY = 0.15;
	void Viewport::cursorPositionCallback(Zap::CursorPosEvent& params, void* viewportData) {
		Viewport* pViewport = (Viewport*)viewportData;
		if (turnCameraPressed && pViewport->isHovered()) {
			glm::mat4 res = pViewport->m_camera.cmpTransform_getTransform();
			glm::mat4 rot = glm::rotate(glm::mat4(1), glm::radians<float>((params.xPos - pViewport->m_xlast) * sensitivityX), glm::vec3{ 0, 1, 0 });
			
			res[0] = rot * res[0];
			res[1] = rot * res[1];
			res[2] = rot * res[2];
			
			pViewport->m_camera.cmpTransform_setTransform(res);
			pViewport->m_camera.cmpTransform_rotateX((params.yPos - pViewport->m_ylast) * sensitivityY);
		}
		
		pViewport->m_xlast = params.xPos;
		pViewport->m_ylast = params.yPos;
	}

	uint32_t countTransformLines() {
		return 3;
	}

	void drawTransformLines(uint32_t& offset, Zap::DebugRenderVertex* data, Zap::Actor actor, uint32_t select, float scale) {
		data += offset*2;

		glm::vec3 pos = actor.cmpTransform_getPos();
		auto transform = actor.cmpTransform_getTransform();
		glm::vec3 xAxis = glm::vec3(1, 0, 0)*scale;
		glm::vec3 yAxis = glm::vec3(0, 1, 0)*scale;
		glm::vec3 zAxis = glm::vec3(0, 0, 1)*scale;

		uint8_t highlight;

		if (select == 0) highlight = 200;
		else highlight = 0;
		data[0] = { pos,         {255,       highlight, highlight} };
		data[1] = { pos + xAxis, {255,       highlight, highlight} };
		if (select == 1) highlight = 200;
		else highlight = 0;
		data[2] = { pos,         {highlight, 255,       highlight} };
		data[3] = { pos + yAxis, {highlight, 255,       highlight} };
		if (select == 2) highlight = 200;
		else highlight = 0;
		data[4] = { pos,         {highlight, highlight, 255      } };
		data[5] = { pos + zAxis, {highlight, highlight, 255      } };

		offset += 3;
	}

	bool intersectPlane(const glm::vec3& n, const glm::vec3& p0, const glm::vec3& l0, const glm::vec3& l, glm::vec3& p)
	{
		// Assuming vectors are all normalized
		float denom = glm::dot(n, l);
		if (std::abs(denom) > 1e-6) {
			glm::vec3 p0l0 = p0 - l0;
			float t = glm::dot(p0l0, n) / denom;
			if (t >= 0) {
				p = l * t + l0;
				return true;
			}
		}

		return false;
	}

	void Viewport::draw() {
		if (ImGui::BeginMenuBar()) {
			std::string mode;
			switch (m_renderType)
			{
			case ePBR:
				mode = "PBR";
				break;
			case eRAYTRACING:
				mode = "Raytracing";
				break;
			case ePATHTRACING:
				mode = "Path Tracing";
				break;
			default:
				break;
			}
			if (ImGui::BeginMenu(("Mode: " + mode).c_str())) {
				if (ImGui::MenuItem("PBR")) {
					changeRenderType(ePBR);
				}

				// disable rendering modes that use raytracing if not enabled
				if (!Zap::Base::getBase()->getSettings()->enableRaytracing)
					ImGui::BeginDisabled();

				if (ImGui::MenuItem("Raytracing")) {
					changeRenderType(eRAYTRACING);
				}
				if (ImGui::MenuItem("Path Tracing")) {
					changeRenderType(ePATHTRACING);
				}

				if (!Zap::Base::getBase()->getSettings()->enableRaytracing)
					ImGui::EndDisabled();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View")) {
				if (ImGui::BeginMenu("Settings")) {
					bool shouldUpdate = false;
					if (ImGui::Checkbox("Outlines", &m_settings.enableOutlines)) {
						shouldUpdate = true;
					}
					ImGui::Checkbox("TransformVisualEditing", &m_settings.enableTransformVisual);
					ImGui::Checkbox("PhysXDebug", &m_settings.enablePxDebug);
					if (shouldUpdate)
						update();
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		auto imageExtent = m_outImage.getExtent();
		auto extent = ImGui::GetContentRegionAvail();

		//Resize outImage and dependencies
		{
			if (extent.x != imageExtent.width || extent.y != imageExtent.height) {// resize
				extent.x = std::max<float>(extent.x, 1);
				extent.y = std::max<float>(extent.y, 1);
				m_outImage.setWidth(extent.x);
				m_outImage.setHeight(extent.y);
				m_pPBRender->setViewport(extent.x, extent.y, 0, 0);
				update();
			}
		}

		ImGui::Image(m_imageDescriptorSet, extent);// Draw the viewport image

		m_isHovered = ImGui::IsItemHovered();
		m_isFocused = ImGui::IsWindowFocused();

		auto timeEndFrame = std::chrono::high_resolution_clock::now();
		extern float dTime;
		move(dTime);

		m_pPBRender->updateCamera(m_camera);
		m_pOutlineRenderTask->updateCamera(m_camera);
		m_pDebugRenderTask->updateCamera(m_camera);
		if (Zap::Base::getBase()->getSettings()->enableRaytracing) {
			m_pRTRender->updateCamera(m_camera);
			m_pPathTracer->updateCamera(m_camera);
		}

		m_pScene->update();

		// update TransformEditScene
		if(!m_isTransformDragged)
			m_axisIndex = 0xFFFFFFFF;
		if (m_settings.enableTransformVisual && m_selectedActors.size() > 0) {
			auto actor = m_selectedActors.back();
			if (actor.hasTransform()) {
				float scale = glm::length(actor.cmpTransform_getPos() - m_camera.cmpTransform_getPos()) / 5.f;
				Zap::BoxGeometry box = Zap::BoxGeometry(glm::vec3(0.5, 0.05, 0.05)*scale);

				m_transformX.cmpTransform_setPos(actor.cmpTransform_getPos()+glm::vec3(0.5 * scale, 0, 0));
				m_transformX.cmpRigidDynamic_updatePose();
				m_transformX.cmpRigidDynamic_getShapes()[0].setGeometry(box);

				m_transformY.cmpTransform_setPos(actor.cmpTransform_getPos()+glm::vec3(0, 0.5 * scale, 0));
				m_transformY.cmpRigidDynamic_updatePose();
				m_transformY.cmpRigidDynamic_getShapes()[0].setGeometry(box);

				m_transformZ.cmpTransform_setPos(actor.cmpTransform_getPos()+glm::vec3(0, 0, 0.5 * scale));
				m_transformZ.cmpRigidDynamic_updatePose();
				m_transformZ.cmpRigidDynamic_getShapes()[0].setGeometry(box);

				float minx = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMin().x;
				float miny = ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMin().y;
				float sizex = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x - minx;
				float sizey = ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMax().y - miny;
				float mouseX = ((ImGui::GetMousePos().x - minx) / sizex) * 2.0 - 1.0;
				float mouseY = ((ImGui::GetMousePos().y - miny) / sizey) * 2.0 - 1.0;

				if (mouseX > -1 && mouseX < 1 && mouseY > -1 && mouseY < 1) {
					glm::vec3 origin = m_camera.cmpTransform_getPos();
					glm::vec4 localDir = glm::inverse(m_camera.cmpCamera_getPerspective(imageExtent.width / (float)imageExtent.height)) * glm::vec4(mouseX, -mouseY, 1, 1);
					glm::vec4 direction = glm::inverse(m_camera.cmpCamera_getView()) * glm::vec4(glm::normalize(glm::vec3(localDir)), 0);
					glm::vec3 axis;

					if (!m_isTransformDragged) {
						Zap::Scene::RaycastOutput out{};
						m_transformEditScene.raycast(origin, direction, 0xFFFFFFFF, &out);
						if (out.actor == m_transformX) {
							m_axisIndex = 0;
						}
						else if (out.actor == m_transformY) {
							m_axisIndex = 1;
						}
						else if (out.actor == m_transformZ) {
							m_axisIndex = 2;
						}
					}

					switch (m_axisIndex)
					{
					case 0:
						axis = { 1, 0, 0 };
						break;
					case 1:
						axis = { 0, 1, 0 };
						break;
					case 2:
						axis = { 0, 0, 1 };
						break;
					default:
						axis = { 0, 0, 0 };
						break;
					}

					if (m_isTransformDragged) {
						m_isTransformDragged = ImGui::IsMouseDown(ImGuiMouseButton_Right);
						if (!m_isTransformDragged && actor.hasRigidDynamic()) {
							actor.cmpRigidDynamic_updatePose();
						}
					}
					else {
						m_isTransformDragged = ImGui::IsMouseDown(ImGuiMouseButton_Right) && m_axisIndex < 0xFFFFFFFF;
						if (m_isTransformDragged)
							intersectPlane(glm::normalize(glm::cross(axis, glm::vec3(m_camera.cmpTransform_getTransform()[0]))), actor.cmpTransform_getPos(), origin, direction, m_mousePlanePos);
					}

					auto oldMousePlanePos = m_mousePlanePos;
					glm::vec3 deltaMousePlane = {0, 0, 0};
					if (m_isTransformDragged && intersectPlane(glm::normalize(glm::cross(axis, glm::vec3(m_camera.cmpTransform_getTransform()[0]))), actor.cmpTransform_getPos(), origin, direction, m_mousePlanePos)) {
						deltaMousePlane = m_mousePlanePos - oldMousePlanePos;
						actor.cmpTransform_setPos(actor.cmpTransform_getPos()+deltaMousePlane*axis);
						if(m_pPathTracer)
							m_pPathTracer->resetRender();
					}
				}
			}
		}

		// render lines
		{
			uint32_t size = 0;
			uint32_t offset = 0;

			// find the line count
			if (m_settings.enableTransformVisual) {
				for (auto actor : m_selectedActors) {
					if (actor.hasTransform()) {
						size += countTransformLines();
					}
				}
			}

			std::vector<Zap::DebugRenderVertex> pxDebugVertices = {};
			if (m_settings.enablePxDebug) {
				m_pScene->getPxDebugVertices(pxDebugVertices);
			}

			size += pxDebugVertices.size()/2;
			size += m_debugLineVector.size() / 2;
			size += 2;

			//render all lines
			if (size > 0) {
				m_debugVertexBuffer.resize(sizeof(Zap::DebugRenderVertex)*size*2);
				void* rawData;
				m_debugVertexBuffer.map(&rawData);
				Zap::DebugRenderVertex* data = (Zap::DebugRenderVertex*)rawData;

				if (m_settings.enableTransformVisual) {
					for (auto actor : m_selectedActors) {
						if (actor.hasTransform()) {
							drawTransformLines(offset, data, actor, m_axisIndex, glm::length(actor.cmpTransform_getPos() - m_camera.cmpTransform_getPos())/5.f);
						}
					}
				}

				if (m_settings.enablePxDebug) {
					memcpy(&data[offset*2], pxDebugVertices.data(), sizeof(Zap::DebugRenderVertex)*pxDebugVertices.size());
					offset += pxDebugVertices.size() / 2;
				}

				memcpy(&data[offset*2], m_debugLineVector.data(), m_debugLineVector.size()*sizeof(Zap::DebugRenderVertex));
				offset += m_debugLineVector.size() / 2;
				m_debugLineVector.clear();
		
				m_debugVertexBuffer.unmap();
			}

			//check if debug vertexBuffer is valid
			m_pDebugRenderTask->delLineVertexBuffers();
			if (size)
				m_pDebugRenderTask->addLineVertexBuffer(&m_debugVertexBuffer);
		}

		m_renderer.render();
	}

	ImGuiWindowFlags Viewport::getWindowFlags() {
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
		if (m_isHovered) windowFlags |= ImGuiWindowFlags_NoMove;
		return windowFlags;
	}

	void Viewport::changeRenderType(RenderType renderType) {
		m_renderType = renderType;
		update();
	}

	void Viewport::update() {
		ImGui_ImplVulkan_RemoveTexture(m_imageDescriptorSet);
		m_outImage.update();
		m_renderer.beginRecord();
		m_pPBRender->disable();
		if (Zap::Base::getBase()->getSettings()->enableRaytracing) {
			m_pRTRender->disable();
			m_pPathTracer->disable();
		}
		switch (m_renderType)
		{
		case ePBR:
			m_renderer.recChangeImageLayout(&m_outImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
			m_renderer.recRenderTemplate(m_pPBRender);
			m_pPBRender->enable();
			break;
		case eRAYTRACING:
			m_renderer.recRenderTemplate(m_pRTRender);
			m_pRTRender->enable();
			m_renderer.recChangeImageLayout(&m_outImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
			break;
		case ePATHTRACING:
			m_renderer.recRenderTemplate(m_pPathTracer);
			m_pPathTracer->enable();
			m_renderer.recChangeImageLayout(&m_outImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
			break;
		default:
			break;
		}
		if (m_settings.enableOutlines) {
			m_renderer.recRenderTemplate(m_pOutlineRenderTask);
		}
		m_renderer.recRenderTemplate(m_pDebugRenderTask);
		m_renderer.recChangeImageLayout(&m_outImage, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT);
		m_renderer.endRecord();
		m_renderer.resize();
		m_imageDescriptorSet = ImGui_ImplVulkan_AddTexture(m_sampler, m_outImage.getVkImageView(), VK_IMAGE_LAYOUT_GENERAL);

	}
}