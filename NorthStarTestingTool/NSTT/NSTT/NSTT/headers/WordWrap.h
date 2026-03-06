#pragma once
#include <imgui_internal.h>
#include <iostream>
#include <string>


struct AutoWrapData {
    float wrap_px; // available pixel width for text inside the box
};

int AutoWrapCallback(ImGuiInputTextCallbackData* data);

void MyWidget();