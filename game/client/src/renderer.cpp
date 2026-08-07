#include "renderer.hpp"

#include <sys/types.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "components/physics.hpp"
#include "components/renderer.hpp"
#include "entt/entity/fwd.hpp"
#include "glm/common.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_common.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/trigonometric.hpp"
#include "input.hpp"
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
	ctx.config.width = static_cast<int32_t>(width / dpr);
	ctx.config.height = static_cast<int32_t>(height / dpr);
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

void setup(std::function<void(bool success)> onComplete) {
	ctx.instance = wgpu::CreateInstance();
	if (!ctx.instance) {
		std::cerr << "[WebGPU] Failed to create instance.\n";
		onComplete(false);
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
		[onComplete](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, wgpu::StringView msg) {
		(void)msg;
		if (status != wgpu::RequestAdapterStatus::Success) {
			std::cerr << "[WebGPU] Failed to get adapter.\n";
			onComplete(false);
			return;
		}
		ctx.adapter = std::move(adapter);

		wgpu::DeviceDescriptor deviceDesc{};
		ctx.adapter.RequestDevice(	//
			&deviceDesc,
			wgpu::CallbackMode::AllowSpontaneous,
			[onComplete](wgpu::RequestDeviceStatus status, wgpu::Device device, wgpu::StringView msg) {
			(void)msg;
			if (status != wgpu::RequestDeviceStatus::Success) {
				std::cerr << "[WebGPU] Failed to get device.\n";
				onComplete(false);
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
				onComplete(false);
				return;
			}

			std::cout << "[WebGPU] Initialized successfully.\n";
			onComplete(true);
		});
	});

	ctx.instance.ProcessEvents();
}

bool CreateRenderPipeline() {
	std::optional<std::string> wgslSrc = core::utils::loadFile("shaders/simple.wgsl");
	if (!wgslSrc.has_value()) {
		std::cerr << "[WebGPU] Failed to load shader.\n";
		return false;
	}

	wgpu::ShaderSourceWGSL shaderSrc{};
	shaderSrc.code = wgslSrc.value().data();
	wgpu::ShaderModuleDescriptor shaderDesc{};
	shaderDesc.nextInChain = &shaderSrc;
	wgpu::ShaderModule shader{ ctx.device.CreateShaderModule(&shaderDesc) };

	// Group 0
	wgpu::BindGroupLayoutEntry bglCamera{};
	bglCamera.binding = 0;
	bglCamera.visibility = wgpu::ShaderStage::Vertex;
	bglCamera.buffer.type = wgpu::BufferBindingType::Uniform;
	bglCamera.buffer.minBindingSize = sizeof(glm::mat4);

	wgpu::BindGroupLayoutDescriptor bglDescCamera{};
	bglDescCamera.entryCount = 1;
	bglDescCamera.entries = &bglCamera;
	ctx.frameBindGroupLayout = ctx.device.CreateBindGroupLayout(&bglDescCamera);

	// Group 1
	wgpu::BindGroupLayoutEntry bglInstances{};
	bglInstances.binding = 0;
	bglInstances.visibility = wgpu::ShaderStage::Vertex;
	bglInstances.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
	bglInstances.buffer.minBindingSize = sizeof(glm::mat4);

	wgpu::BindGroupLayoutDescriptor bglDescInstances{};
	bglDescInstances.entryCount = 1;
	bglDescInstances.entries = &bglInstances;
	ctx.meshBindGroupLayout = ctx.device.CreateBindGroupLayout(&bglDescInstances);

	wgpu::BufferDescriptor camBufDesc{};
	camBufDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
	camBufDesc.size = sizeof(glm::mat4);
	ctx.frameUniformBuffer = ctx.device.CreateBuffer(&camBufDesc);

	wgpu::BindGroupEntry bgCamera{};
	bgCamera.binding = 0;
	bgCamera.buffer = ctx.frameUniformBuffer;
	bgCamera.size = sizeof(glm::mat4);

	wgpu::BindGroupDescriptor bgDesc{};
	bgDesc.layout = ctx.frameBindGroupLayout;
	bgDesc.entryCount = 1;
	bgDesc.entries = &bgCamera;
	ctx.frameBindGroup = ctx.device.CreateBindGroup(&bgDesc);

	// Pipeline Layout
	std::array<wgpu::BindGroupLayout, 2> layouts{ ctx.frameBindGroupLayout, ctx.meshBindGroupLayout };
	wgpu::PipelineLayoutDescriptor layoutDesc{};
	layoutDesc.bindGroupLayoutCount = layouts.size();
	layoutDesc.bindGroupLayouts = layouts.data();
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

	ctx.pipelines[PipelineType::Opaque] = ctx.device.CreateRenderPipeline(&pipelineDesc);
	return true;
}

void updateCameras(entt::registry& registry, float deltaTime, float alpha) {
	if (!ctx.device)
		return;

	auto localPlayer = registry.ctx().get<client::input::LocalPlayer>().id;
	if (registry.all_of<Camera, PlayerController, Transform>(localPlayer)) {
		auto& cam{ registry.get<Camera>(localPlayer) };
		auto& player{ registry.get<PlayerController>(localPlayer) };
		auto& transform{ registry.get<Transform>(localPlayer) };

		if (player.camNeedSnap) {
			cam.yaw = player.camYaw;
			cam.pitch = player.camPitch;
			player.camNeedSnap = false;
		}

		float smoothFactor = std::clamp(15.0f * deltaTime, 0.0f, 1.0f);
		cam.yaw = glm::mix(cam.yaw, player.camYaw, smoothFactor);
		cam.pitch = glm::mix(cam.pitch, player.camPitch, smoothFactor);
		glm::vec3 visualPos = glm::mix(transform.prevPos, transform.pos, alpha);

		glm::vec3 up = glm::vec3{ 0.0f, 1.5f, 0.0f };
		float distance = 4.3f;
		float horizontal{ distance * std::cos(cam.pitch) };
		float vertical{ distance * std::sin(cam.pitch) };

		glm::vec3 offset{ //
			horizontal * std::sin(cam.yaw),
			-vertical,
			horizontal * std::cos(cam.yaw)
		};

		glm::vec3 targetPos = visualPos;
		glm::vec3 cameraPos = visualPos + up + offset;

		glm::mat4 proj = glm::perspectiveZO(glm::radians(75.0f), ctx.aspect, 0.2f, 1000.0f);
		glm::mat4 view = glm::lookAt(cameraPos, targetPos, glm::vec3(0.0f, 1.0f, 0.0f));
		cam.viewProj = proj * view;
	}
}

void prepareScene(entt::registry& registry, float alpha) {
	if (!ctx.device)
		return;

	for (auto& [meshId, batch] : ctx.instanceBatches)
		batch.clear();

	// TODO: Frustum culling

	auto instanceView = registry.view<const Transform, const StaticMeshInstance>();
	for (auto [entity, transform, instance] : instanceView.each()) {
		glm::vec3 visualPos = glm::mix(transform.prevPos, transform.pos, alpha);
		glm::quat targetRot = transform.rot;
		if (glm::dot(transform.prevRot, targetRot) < 0.0f) {
			targetRot = -targetRot;
		}
		glm::quat visualRot = glm::normalize(glm::slerp(transform.prevRot, targetRot, alpha));

		glm::mat4 model = glm::translate(glm::mat4(1.0f), visualPos);
		model = model * glm::mat4_cast(visualRot);
		model = glm::scale(model, transform.scale);

		ctx.instanceBatches[instance.meshId].push_back({ model });
	}
}

void render(entt::registry& registry) {
	if (!ctx.device)
		return;

	wgpu::SurfaceTexture surfaceTexture;
	ctx.surface.GetCurrentTexture(&surfaceTexture);
	if (!surfaceTexture.texture)
		return;

	glm::mat4 vp{ 1.0f };
	auto camView{ registry.view<const Camera>() };
	for (auto entity : camView) {
		vp = camView.get<const Camera>(entity).viewProj;
		break;
	}
	ctx.queue.WriteBuffer(ctx.frameUniformBuffer, 0, &vp, sizeof(glm::mat4));

	for (const auto& [meshId, meshes] : ctx.instanceBatches) {
		if (meshes.empty())
			continue;

		const auto& mesh{ ctx.meshes[meshId] };
		ctx.queue.WriteBuffer(mesh.instanceBuffer, 0, meshes.data(), meshes.size() * sizeof(InstanceData));
	}

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
	pass.SetPipeline(ctx.pipelines[PipelineType::Opaque]);
	pass.SetBindGroup(0, ctx.frameBindGroup);

	for (const auto& [meshId, meshes] : ctx.instanceBatches) {
		if (meshes.empty())
			continue;

		const auto& mesh{ ctx.meshes[meshId] };
		pass.SetBindGroup(1, mesh.bindGroup);
		pass.SetVertexBuffer(0, mesh.vertexBuffer);
		pass.SetIndexBuffer(mesh.indexBuffer, wgpu::IndexFormat::Uint32, 0);
		pass.DrawIndexed(mesh.indexCount, meshes.size(), 0, 0, 0);
	}

	pass.End();
	wgpu::CommandBuffer command = encoder.Finish();
	ctx.queue.Submit(1, &command);
}
}
