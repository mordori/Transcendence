#include "render.hpp"

#include <sys/types.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <utility>

#include "components/physics.hpp"
#include "components/renderable.hpp"
#include "entt/entity/fwd.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/trigonometric.hpp"
#include "utils/files.hpp"
#include "webgpu/webgpu.h"
#include "webgpu/webgpu_cpp.h"

#ifdef __EMSCRIPTEN__
#include "emscripten/html5.h"
#endif

namespace client::renderer {

bool CreateRenderPipeline();

static RenderContext ctx{};

RenderContext& getRenderContext() {
	return ctx;
}

void resizeSurface() {
	if (!ctx.device || !ctx.surface)
		return;
	double width{}, height{};
	emscripten_get_element_css_size("#engine-canvas", &width, &height);
	double dpr = emscripten_get_device_pixel_ratio();
	ctx.config.width = static_cast<int32_t>(width * dpr);
	ctx.config.height = static_cast<int32_t>(height * dpr);
	ctx.aspect = static_cast<float>(ctx.config.width) / static_cast<float>(ctx.config.height);
	ctx.config.usage = wgpu::TextureUsage::RenderAttachment;
	ctx.config.alphaMode = wgpu::CompositeAlphaMode::Auto;
	ctx.surface.Configure(&ctx.config);

	wgpu::TextureDescriptor depthDesc{};
	depthDesc.dimension = wgpu::TextureDimension::e2D;
	depthDesc.size = { //
		.width = ctx.config.width,
		.height = ctx.config.height,
		.depthOrArrayLayers = 1
	};
	depthDesc.mipLevelCount = 1;
	depthDesc.sampleCount = 1;
	depthDesc.format = wgpu::TextureFormat::Depth24Plus;
	depthDesc.usage = wgpu::TextureUsage::RenderAttachment;
	ctx.depthTexture = ctx.device.CreateTexture(&depthDesc);
	ctx.depthView = ctx.depthTexture.CreateView();
}

EM_BOOL onWindowResize(int eventType, const EmscriptenUiEvent* uiEvent, void* userData) {
	(void)eventType;
	(void)uiEvent;
	(void)userData;
	resizeSurface();
	return EM_TRUE;
}

void initWebGPU(std::function<void(bool success)> initStatus) {
	ctx.instance = wgpu::CreateInstance();
	if (!ctx.instance) {
		std::cerr << "[WebGPU] Failed to create instance.\n";
		initStatus(false);
		return;
	}

	wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector canvas{};
	canvas.selector = "#engine-canvas";
	wgpu::SurfaceDescriptor surfaceDesc{};
	surfaceDesc.nextInChain = &canvas;
	ctx.surface = ctx.instance.CreateSurface(&surfaceDesc);

	wgpu::RequestAdapterOptions adapterOpts{};
	adapterOpts.compatibleSurface = ctx.surface;

	ctx.instance.RequestAdapter(  //
		&adapterOpts,
		wgpu::CallbackMode::AllowSpontaneous,
		[initStatus](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, wgpu::StringView msg) {
		(void)msg;
		if (status != wgpu::RequestAdapterStatus::Success) {
			std::cerr << "[WebGPU] Failed to get adapter.\n";
			initStatus(false);
			return;
		}
		ctx.adapter = std::move(adapter);

		wgpu::DeviceDescriptor deviceDesc{};
		ctx.adapter.RequestDevice(	//
			&deviceDesc,
			wgpu::CallbackMode::AllowSpontaneous,
			[initStatus](wgpu::RequestDeviceStatus status, wgpu::Device device, wgpu::StringView msg) {
			(void)msg;
			if (status != wgpu::RequestDeviceStatus::Success) {
				std::cerr << "[WebGPU] Failed to get device.\n";
				initStatus(false);
				return;
			}
			ctx.device = std::move(device);
			ctx.queue = ctx.device.GetQueue();

			wgpu::SurfaceCapabilities capabilities{};
			ctx.surface.GetCapabilities(ctx.adapter, &capabilities);

			ctx.config.device = ctx.device;
			ctx.config.format = capabilities.formats[0];
			ctx.config.presentMode = wgpu::PresentMode::Fifo;
			ctx.config.usage = wgpu::TextureUsage::RenderAttachment;
			ctx.config.alphaMode = wgpu::CompositeAlphaMode::Auto;

			emscripten_set_resize_callback_on_thread(  //
				EMSCRIPTEN_EVENT_TARGET_WINDOW,
				nullptr,
				false,
				onWindowResize,
				EM_CALLBACK_THREAD_CONTEXT_MAIN_BROWSER_THREAD);
			resizeSurface();

			if (!CreateRenderPipeline()) {
				initStatus(false);
				return;
			}

			std::cout << "[WebGPU] Initialized successfully.\n";
			initStatus(true);
		});
	});

	ctx.instance.ProcessEvents();
}

bool CreateRenderPipeline() {
	std::optional<std::string> wgslSrc = core::utils::loadFile("shaders/bsdf.wgsl");
	if (!wgslSrc.has_value()) {
		std::cerr << "[WebGPU] Failed to load shader.\n";
		return false;
	}

	wgpu::ShaderSourceWGSL shaderSrc{};
	shaderSrc.code = wgslSrc.value().data();
	wgpu::ShaderModuleDescriptor shaderDesc{};
	shaderDesc.nextInChain = &shaderSrc;
	wgpu::ShaderModule shader{ ctx.device.CreateShaderModule(&shaderDesc) };

	// BindGroupLayout
	wgpu::BindGroupLayoutEntry bglEntry{};
	bglEntry.binding = 0;
	bglEntry.visibility = wgpu::ShaderStage::Vertex;
	bglEntry.buffer.type = wgpu::BufferBindingType::Uniform;
	bglEntry.buffer.minBindingSize = sizeof(glm::mat4);

	wgpu::BindGroupLayoutDescriptor bglDesc{};
	bglDesc.entryCount = 1;
	bglDesc.entries = &bglEntry;
	ctx.bindGroupLayout = ctx.device.CreateBindGroupLayout(&bglDesc);

	// Pipeline Layout
	wgpu::PipelineLayoutDescriptor layoutDesc{};
	layoutDesc.bindGroupLayoutCount = 1;
	layoutDesc.bindGroupLayouts = &ctx.bindGroupLayout;
	wgpu::PipelineLayout pipeline_layout = ctx.device.CreatePipelineLayout(&layoutDesc);

	// Render Pipeline Construction
	wgpu::RenderPipelineDescriptor pipelineDesc{};
	pipelineDesc.layout = pipeline_layout;

	// Vertex
	wgpu::VertexState vertexState{};
	vertexState.module = shader;
	vertexState.entryPoint = "vertex";

	std::array<wgpu::VertexAttribute, 2> attributes;
	attributes[0].shaderLocation = 0;
	attributes[0].offset = 0;
	attributes[0].format = wgpu::VertexFormat::Float32x3;
	attributes[1].shaderLocation = 1;
	attributes[1].offset = sizeof(float) * 3;
	attributes[1].format = wgpu::VertexFormat::Float32x3;

	wgpu::VertexBufferLayout vertexLayout{};
	vertexLayout.arrayStride = sizeof(float) * 6;
	vertexLayout.attributeCount = attributes.size();
	vertexLayout.attributes = attributes.data();
	vertexState.bufferCount = 1;
	vertexState.buffers = &vertexLayout;
	pipelineDesc.vertex = vertexState;

	// Fragment
	wgpu::FragmentState fragmentState{};
	fragmentState.module = shader;
	fragmentState.entryPoint = "fragment";
	fragmentState.targetCount = 1;

	wgpu::ColorTargetState colorTarget{};
	colorTarget.format = ctx.config.format;
	colorTarget.writeMask = wgpu::ColorWriteMask::All;
	fragmentState.targets = &colorTarget;
	pipelineDesc.fragment = &fragmentState;

	// Depth
	wgpu::DepthStencilState depthState{};
	depthState.format = wgpu::TextureFormat::Depth24Plus;
	depthState.depthWriteEnabled = true;
	depthState.depthCompare = wgpu::CompareFunction::Less;
	pipelineDesc.depthStencil = &depthState;

	// Primitive
	pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
	pipelineDesc.primitive.cullMode = wgpu::CullMode::Back;

	ctx.pipeline = ctx.device.CreateRenderPipeline(&pipelineDesc);
	return true;
}

void render(entt::registry& registry) {
	if (!ctx.device)
		return;

	wgpu::SurfaceTexture surfaceTexture;
	ctx.surface.GetCurrentTexture(&surfaceTexture);
	if (!surfaceTexture.texture)
		return;

	wgpu::TextureView texView = surfaceTexture.texture.CreateView();

	wgpu::RenderPassColorAttachment colorAttachment{};
	colorAttachment.view = texView;
	colorAttachment.loadOp = wgpu::LoadOp::Clear;
	colorAttachment.storeOp = wgpu::StoreOp::Store;
	colorAttachment.clearValue = { .r = 0.0, .g = 0.0, .b = 0.0, .a = 1.0 };

	wgpu::RenderPassDepthStencilAttachment depthAttachment{};
	depthAttachment.depthLoadOp = wgpu::LoadOp::Clear;
	depthAttachment.depthStoreOp = wgpu::StoreOp::Store;
	depthAttachment.depthClearValue = 1.0f;
	depthAttachment.view = ctx.depthView;

	wgpu::RenderPassDescriptor renderPassDesc{};
	renderPassDesc.colorAttachmentCount = 1;
	renderPassDesc.colorAttachments = &colorAttachment;
	renderPassDesc.depthStencilAttachment = &depthAttachment;

	wgpu::CommandEncoder encoder = ctx.device.CreateCommandEncoder();
	wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
	pass.SetPipeline(ctx.pipeline);

	glm::vec3 cameraPos{ 0.0f, 5.0f, 10.0f };
	glm::vec3 targetPos{ 0.0f, 0.0f, 0.0f };

	auto playerView = registry.view<const PlayerTag, const Transform, const PlayerController>();
	for (auto [entity, transform, player] : playerView.each()) {
		glm::vec3 up = glm::vec3{ 0.0f, 1.5f, 0.0f };
		float distance = 4.3f;
		float horizontal{ distance * std::cos(player.camPitch) };
		float vertical{ distance * std::sin(player.camPitch) };

		glm::vec3 offset{ //
			horizontal * std::sin(player.camYaw),
			-vertical,
			horizontal * std::cos(player.camYaw)
		};

		targetPos = transform.pos;
		cameraPos = transform.pos + up + offset;
	}

	glm::mat4 proj = glm::perspective(glm::radians(75.0f), ctx.aspect, 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(cameraPos, targetPos, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 vp = proj * view;

	auto renderableView = registry.view<const Transform, const Renderable>();
	for (auto [entity, transform, renderable] : renderableView.each()) {
		glm::mat4 model = glm::translate(glm::mat4(1.0f), transform.pos);
		model = model * glm::mat4_cast(transform.rot);
		model = glm::scale(model, transform.scale);
		glm::mat4 mvp = vp * model;

		ctx.queue.WriteBuffer(renderable.uniformBuffer, 0, &mvp, sizeof(glm::mat4));
		pass.SetBindGroup(0, renderable.bindGroup);
		pass.SetVertexBuffer(0, renderable.vertexBuffer);
		pass.SetIndexBuffer(renderable.indexBuffer, wgpu::IndexFormat::Uint16, 0);

		pass.DrawIndexed(renderable.indexCount);
	}
	pass.End();

	wgpu::CommandBuffer command = encoder.Finish();
	ctx.queue.Submit(1, &command);
}
}
