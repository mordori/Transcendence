struct Camera {
	viewProj: mat4x4<f32>,
};

struct InstanceData {
	model: mat4x4<f32>,
};

@group(0) @binding(0) var<uniform> camera: Camera;
@group(1) @binding(0) var<storage, read> instances: array<InstanceData>;

struct VertexInput {
	@location(0) position: vec3<f32>,
	@location(1) normal: vec3<f32>,
};

struct VertexOutput {
	@builtin(position) position: vec4<f32>,
	@location(0) normal: vec3<f32>,
};

@vertex
fn vertex(in: VertexInput, @builtin(instance_index) instanceIdx: u32) -> VertexOutput {
	var out: VertexOutput;

	let model = instances[instanceIdx].model;
	out.position = camera.viewProj * model * vec4<f32>(in.position, 1.0);
	out.normal = (model * vec4<f32>(in.normal, 0.0)).xyz;

	return out;
}

@fragment
fn fragment(in: VertexOutput) -> @location(0) vec4<f32> {
	// let lightDir = normalize(vec3<f32>(0.5, 1.0, 0.3));
	// let n = normalize(in.normal);
	// let diff = max(dot(n, lightDir), 0.2);
	// return vec4<f32>(vec3<f32>(0.8, 0.2, 0.2) * diff, 1.0);
	let normal = normalize(in.normal);
	let color = (normal + vec3<f32>(1.0)) * 0.5;
	return vec4<f32>(color, 1.0);
}
