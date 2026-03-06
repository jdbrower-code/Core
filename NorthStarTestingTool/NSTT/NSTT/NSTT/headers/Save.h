#pragma once
#include <iostream>
#include <string>
#include <urlmon.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <windows.h>

static std::filesystem::path user_save_dir();
static std::filesystem::path make_save_path(const std::string& username,
    const std::string& dateString);
std::string FileName();