#include "GraphicsPipeline.h"
#include "ShaderModule.h"
#include <iostream>

namespace Vk {
	GraphicsPipeline::GraphicsPipeline(const Device& device, const DescriptorSetManager& descriptorSetManager, const RenderInfo& info, bool isWireFrame)
		: _device(device.GetDevice()), _isWireFrame(isWireFrame)
	{
		std::vector<VkVertexInputBindingDescription> vertexBindings;
		std::vector<VkVertexInputAttributeDescription> attributes;

		if (!info.useInstance) {
			vertexBindings = { Vertex::GetBindingDescription() };
			attributes = Vertex::GetAttributeDescriptions();
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindings.size());
		vertexInputInfo.pVertexBindingDescriptions = vertexBindings.data();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(info.viewWidth);
		viewport.height = static_cast<float>(info.viewHeight);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = VkExtent2D{ info.viewWidth , info.viewHeight };

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = isWireFrame ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = (VkCompareOp)info.compareOp;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		//depthStencil.minDepthBounds = 0.0f; // Optional
		//depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = VK_FALSE;
		//depthStencil.front = {}; // Optional
		//depthStencil.back = {}; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicStateCI{};
		dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCI.pDynamicStates = dynamicStateEnables.data();
		dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

		// todo: renderpass

		// Load shaders.
		auto vert_code = ShaderModule::ReadFile(info.shaderName.first, shaderc_glsl_vertex_shader);
		auto frag_code = ShaderModule::ReadFile(info.shaderName.second, shaderc_glsl_fragment_shader);

		const ShaderModule vertShader(_device, vert_code);
		const ShaderModule fragShader(_device, frag_code);

		VkPipelineShaderStageCreateInfo shaderStages[] =
		{
			vertShader.CreateShaderStage(VK_SHADER_STAGE_VERTEX_BIT),
			fragShader.CreateShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT)
		};

		// Pipeline layout
		VkPushConstantRange pushConstant{};
		pushConstant.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT;
		pushConstant.offset = 0;
		pushConstant.size = info.pushConstantSize;

		VkDescriptorSetLayout layouts[2] = {
			descriptorSetManager.GetBindlessBufferLayout(),
			descriptorSetManager.GetBindlessImageLayout()
		};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = layouts;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
		Check(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout),
			"create pipeline layout.");

		// VULKAN 1.3 : without renderpass
		VkPipelineRenderingCreateInfo renderingInfo{};
		//std::vector<VkFormat> colorFormat{ swapChain.GetImageFormat() };
		renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingInfo.colorAttachmentCount = info.colorAttachmentFormats.size();
		renderingInfo.pColorAttachmentFormats = info.colorAttachmentFormats.size() ? info.colorAttachmentFormats.data() : nullptr;
		renderingInfo.depthAttachmentFormat = info.depthAttachmentFormat;

		// Create graphic pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicStateCI; // Optional
		pipelineInfo.layout = _pipelineLayout;
		pipelineInfo.pNext = &renderingInfo;

		Check(vkCreateGraphicsPipelines(_device, nullptr, 1, &pipelineInfo, nullptr, &_graphicsPipeline),
			"create graphics pipeline");
	}

	GraphicsPipeline::~GraphicsPipeline() {
		vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
	}
}