#pragma once
#include <imgui_internal.h>
#include <imgui.h>
#include "SecGroupFlags.h"
#include "sql.h"

class launchPad
{
public:
	static float saturate(float x);
	static float smooth01(float x);
	static inline float easeOutExpo(float t);

	static bool DrawApproachingText(
		const char* text,
		const ImVec2& screen_center,   // where the text should aim (centered)
		float base_px,                 // "at unit scale" font size in px (e.g. 24)
		float z_start,                 // starting Z "distance" (e.g. 1200)
		float z_end,                   // ending Z (e.g. 10)
		float fov_px,                  // pseudo FOV in pixels (e.g. 350)
		double start_time,             // seconds from ImGui::GetTime() when anim started
		double duration_sec,           // total animation time (e.g. 1.0)
		ImU32 color = IM_COL32(255, 255, 255, 255),
		float shadow_px = 2.0f         // 0 to disable shadow
	);

};

