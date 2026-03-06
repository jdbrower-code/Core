#include "Opening.h"


float launchPad::saturate(float x)
{
	return x < 0.f ? 0.f : (x > 1.f ? 1.f : x);
}

float launchPad::smooth01(float x)
{
	x = saturate(x); return x * x * (3.f - 2.f * x);
}

float launchPad::easeOutExpo(float t)
{
	t = saturate(t);
	return (t >= 1.f) ? 1.f : 1.f - powf(2.f, -10.f * t);
}

bool launchPad::DrawApproachingText(
	const char* text,
	const ImVec2& screen_center,   // where the text should aim (centered)
	float base_px,                 // "at unit scale" font size in px (e.g. 24)
	float z_start,                 // starting Z "distance" (e.g. 1200)
	float z_end,                   // ending Z (e.g. 10)
	float fov_px,                  // pseudo FOV in pixels (e.g. 350)
	double start_time,             // seconds from ImGui::GetTime() when anim started
	double duration_sec,           // total animation time (e.g. 1.0)
	ImU32 color,
	float shadow_px // 0 to disable shadow
)
{
	ImDrawList* dl = ImGui::GetForegroundDrawList();
	ImFont* font = ImGui::GetFont();

	const double now = ImGui::GetTime();
	const float t_lin = (float)((now - start_time) / duration_sec);
	const float t = saturate(t_lin);

	// Animate Z with easing, then convert to a perspective-like pixel size.
	const float z = z_start + (z_end - z_start) * easeOutExpo(t);
	const float scale = fov_px / (fov_px + z);                  // smaller z => bigger scale
	const float px = base_px * scale;                           // effective pixel size

	// Subtle alpha ramp-in/out (optional). Keep max alpha from input color.
	const float a_in = smooth01(t * 1.2f);                     // fade in early
	const float a_out = 1.f - smooth01(ImClamp(t - 0.85f, 0.f, 0.15f) / 0.15f); // tiny fade-out at end
	const float a_mul = saturate(a_in * a_out);

	// Unpack color -> modulate alpha -> repack
	ImVec4 col = ImGui::ColorConvertU32ToFloat4(color);
	col.w *= a_mul;
	ImU32  col_u32 = ImGui::GetColorU32(col);

	// Compute centered pos for this frame's size
	ImVec2 text_sz = font->CalcTextSizeA(px, FLT_MAX, 0.0f, text);
	ImVec2 pos = ImVec2(screen_center.x - text_sz.x * 0.5f, screen_center.y - text_sz.y * 0.5f);

	// Cheap soft shadow
	if (shadow_px > 0.0f) {
		ImU32 shadow = IM_COL32(0, 0, 0, (ImU32)(col.w * 255 * 0.7f));
		dl->AddText(font, px, ImVec2(pos.x + shadow_px, pos.y + shadow_px), shadow, text);
	}

	// Main text
	dl->AddText(font, px, pos, col_u32, text);

	return t_lin >= 1.0;
}

