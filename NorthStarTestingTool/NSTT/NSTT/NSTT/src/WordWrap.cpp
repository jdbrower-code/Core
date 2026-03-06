#include "WordWrap.h"

int AutoWrapCallback(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag != ImGuiInputTextFlags_CallbackEdit)
        return 0;

    AutoWrapData* cfg = (AutoWrapData*)data->UserData;
    if (!cfg || cfg->wrap_px <= 0.0f)
        return 0;

    const char* buf = data->Buf;
    const char* cur = buf + data->CursorPos;

    // Find start of current logical line
    const char* line_start = cur;
    while (line_start > buf && line_start[-1] != '\n')
        line_start--;

    // Measure current line width in pixels (no wrapping here)
    ImVec2 sz = ImGui::CalcTextSize(line_start, cur, false, -1.0f);
    if (sz.x <= cfg->wrap_px)
        return 0;

    // We overflowed: choose a break point
    const char* break_pos = nullptr;
    for (const char* p = cur; p > line_start; --p) {
        char c = p[-1];
        if (c == ' ' || c == '\t' || c == '-') { break_pos = p - 1; break; }
    }

    if (!break_pos) {
        // No good break char: newline at cursor
        data->InsertChars(data->CursorPos, "\n");
        return 0;
    }

    int idx = (int)(break_pos - buf);

    // If break at a space/tab, replace it with a newline (tidier)

    if (data->Buf[idx] == ' ' || data->Buf[idx] == '\t') {
        data->DeleteChars(idx, 1);
        data->InsertChars(idx, "\n");
    }
    else {
        data->InsertChars(idx + 1, "\n");
    }

    return 0;
}

void MyWidget()
{
    static std::string text;
    text.reserve(4096);

    // Decide the widget size
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 size(400.0f, ImGui::GetTextLineHeight() * 10.0f); // width x height

    // Compute the inner wrap width (subtract frame paddings)
    float inner_wrap_px = size.x - style.FramePadding.x * 2.0f;
    if (inner_wrap_px < 1.0f) inner_wrap_px = 1.0f;

    AutoWrapData wrap{ inner_wrap_px };

    ImGuiInputTextFlags flags =
        ImGuiInputTextFlags_CallbackEdit |
        ImGuiInputTextFlags_NoHorizontalScroll; // (optional but nice UX)

    // Important: use CallbackResize pattern if you keep text in std::string
    // Simplest: ensure capacity() >= desired buffer length + 1
    text.resize(std::max<size_t>(1, text.size())); // ensure data() is valid
    bool changed = ImGui::InputTextMultiline(
        "##multiline", (char*)text.data(), text.capacity() + 1, size,
        flags, AutoWrapCallback, &wrap);

    // If InputTextMultiline grew/shrank, handle CallbackResize pattern (omitted here for brevity).
}

