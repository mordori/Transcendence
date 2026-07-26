#include "systems/render.hpp"

#include <array>
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

namespace client::systems {

bool CreateRenderPipeline();

static RenderContext ctx{};

RenderContext& get_render_context() {
	return ctx;
}

void resize_surface() {
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
}

EM_BOOL on_window_resize(int eventType, const EmscriptenUiEvent* uiEvent, void* userData) {
	(void)eventType;
	(void)uiEvent;
	(void)userData;
	resize_surface();
	return EM_TRUE;
}

void init_webgpu(std::function<void(bool success)> init_status) {
	ctx.instance = wgpu::CreateInstance();
	if (!ctx.instance) {
		std::cerr << "[WebGPU] Failed to create instance.\n";
		init_status(false);
		return;
	}

	wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector canvas{};
	canvas.selector = "#engine-canvas";
	wgpu::SurfaceDescriptor surface_desc{};
	surface_desc.nextInChain = &canvas;
	ctx.surface = ctx.instance.CreateSurface(&surface_desc);

	wgpu::RequestAdapterOptions adapter_opts{};
	adapter_opts.compatibleSurface = ctx.surface;

	ctx.instance.RequestAdapter(  //
		&adapter_opts,
		wgpu::CallbackMode::AllowSpontaneous,
		[init_status](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, wgpu::StringView msg) {
		(void)msg;
		if (status != wgpu::RequestAdapterStatus::Success) {
			std::cerr << "[WebGPU] Failed to get adapter.\n";
			init_status(false);
			return;
		}
		ctx.adapter = std::move(adapter);

		wgpu::DeviceDescriptor device_desc{};
		ctx.adapter.RequestDevice(	//
			&device_desc,
			wgpu::CallbackMode::AllowSpontaneous,
			[init_status](wgpu::RequestDeviceStatus status, wgpu::Device device, wgpu::StringView msg) {
			(void)msg;
			if (status != wgpu::RequestDeviceStatus::Success) {
				std::cerr << "[WebGPU] Failed to get device.\n";
				init_status(false);
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
				on_window_resize,
				EM_CALLBACK_THREAD_CONTEXT_MAIN_BROWSER_THREAD);
			resize_surface();

			if (!CreateRenderPipeline()) {
				init_status(false);
				return;
			}

			std::cout << "[WebGPU] Initialized successfully.\n";
			init_status(true);
		});
	});

	ctx.instance.ProcessEvents();
}

bool CreateRenderPipeline() {
	std::optional<std::string> wgsl_src = core::utils::load_file("shaders/bsdf.wgsl");
	if (!wgsl_src.has_value()) {
		std::cerr << "[WebGPU] Failed to load shader.\n";
		return false;
	}

	wgpu::ShaderSourceWGSL shader_src{};
	shader_src.code = wgsl_src.value().data();
	wgpu::ShaderModuleDescriptor shader_desc{};
	shader_desc.nextInChain = &shader_src;
	wgpu::ShaderModule shader{ ctx.device.CreateShaderModule(&shader_desc) };

	// BindGroupLayout
	wgpu::BindGroupLayoutEntry bgl_entry{};
	bgl_entry.binding = 0;
	bgl_entry.visibility = wgpu::ShaderStage::Vertex;
	bgl_entry.buffer.type = wgpu::BufferBindingType::Uniform;
	bgl_entry.buffer.minBindingSize = sizeof(glm::mat4);

	wgpu::BindGroupLayoutDescriptor bgl_desc{};
	bgl_desc.entryCount = 1;
	bgl_desc.entries = &bgl_entry;
	ctx.bindGroupLayout = ctx.device.CreateBindGroupLayout(&bgl_desc);

	// Pipeline Layout
	wgpu::PipelineLayoutDescriptor layout_desc{};
	layout_desc.bindGroupLayoutCount = 1;
	layout_desc.bindGroupLayouts = &ctx.bindGroupLayout;
	wgpu::PipelineLayout pipeline_layout = ctx.device.CreatePipelineLayout(&layout_desc);

	// Render Pipeline Construction
	wgpu::RenderPipelineDescriptor pipeline_desc{};
	pipeline_desc.layout = pipeline_layout;

	// Vertex State
	wgpu::VertexState vertex_state{};
	vertex_state.module = shader;
	vertex_state.entryPoint = "vertex";

	std::array<wgpu::VertexAttribute, 2> attributes;
	attributes[0].shaderLocation = 0;
	attributes[0].offset = 0;
	attributes[0].format = wgpu::VertexFormat::Float32x3;
	attributes[1].shaderLocation = 1;
	attributes[1].offset = sizeof(float) * 3;
	attributes[1].format = wgpu::VertexFormat::Float32x3;

	wgpu::VertexBufferLayout vertex_layout{};
	vertex_layout.arrayStride = sizeof(float) * 6;
	vertex_layout.attributeCount = attributes.size();
	vertex_layout.attributes = attributes.data();
	vertex_state.bufferCount = 1;
	vertex_state.buffers = &vertex_layout;
	pipeline_desc.vertex = vertex_state;

	// Fragment State
	wgpu::FragmentState fragment_state{};
	fragment_state.module = shader;
	fragment_state.entryPoint = "fragment";
	fragment_state.targetCount = 1;

	wgpu::ColorTargetState color_target{};
	color_target.format = ctx.config.format;
	color_target.writeMask = wgpu::ColorWriteMask::All;
	fragment_state.targets = &color_target;
	pipeline_desc.fragment = &fragment_state;

	// Primitive State
	pipeline_desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
	pipeline_desc.primitive.cullMode = wgpu::CullMode::Back;

	ctx.pipeline = ctx.device.CreateRenderPipeline(&pipeline_desc);
	return true;
}

void render(entt::registry& registry) {
	if (!ctx.device)
		return;

	wgpu::SurfaceTexture surface_texture;
	ctx.surface.GetCurrentTexture(&surface_texture);
	if (!surface_texture.texture)
		return;

	wgpu::TextureView tex_view = surface_texture.texture.CreateView();

	wgpu::RenderPassColorAttachment color_attachment{};
	color_attachment.view = tex_view;
	color_attachment.loadOp = wgpu::LoadOp::Clear;
	color_attachment.storeOp = wgpu::StoreOp::Store;
	color_attachment.clearValue = { .r = 0.0, .g = 0.0, .b = 0.0, .a = 1.0 };

	wgpu::RenderPassDescriptor render_pass_desc{};
	render_pass_desc.colorAttachmentCount = 1;
	render_pass_desc.colorAttachments = &color_attachment;

	wgpu::CommandEncoder encoder = ctx.device.CreateCommandEncoder();
	wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&render_pass_desc);
	pass.SetPipeline(ctx.pipeline);

	glm::mat4 proj = glm::perspective(glm::radians(45.0f), ctx.aspect, 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(  //
		glm::vec3(0.0f, 5.0f, 10.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 vp = proj * view;

	auto ecs_view = registry.view<const Transform, const Renderable>();
	for (auto [entity, transform, renderable] : ecs_view.each()) {
		glm::mat4 model = glm::translate(glm::mat4(1.0f), transform.pos);
		model = model * glm::mat4_cast(transform.rot);
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
