#pragma once

struct InputComponent {
	bool up{};
	bool down{};
	bool left{};
	bool right{};
	bool jump{};
	float yaw{};
	float steering_angle{};

	float cam_yaw{};
	float cam_pitch{ -0.15f };
};
