#include "Save.h"


static std::filesystem::path user_save_dir() {
	char* p = std::getenv("LOCALAPPDATA");
	std::filesystem::path base = p ? p : "C:\\Temp"; // fallback
	return base / "NSTT" / "Saves";
}

static std::filesystem::path make_save_path(const std::string& username,
	const std::string& dateString) {
	auto dir = user_save_dir();
	std::error_code ec;
	std::filesystem::create_directories(dir, ec); // ignore if exists
	return dir / (username + "_" + dateString + ".txt");
}

std::string FileName()
{
	std::string SaveFileName;
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
	tm t = *std::localtime(&currentTime);
	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &t);
	std::string dateString = buffer;
	wchar_t username[257];  // ? wide character buffer
	DWORD size = 257;

	if (GetUserName(username, &size))
	{
		std::wstring wusername(username);               // Convert to std::wstring
		std::string a(wusername.begin(), wusername.end());  // Convert username to std::string if needed

		SaveFileName = make_save_path(a /*username*/, dateString).string();
	}

	//std::cout << SaveFileName << std::endl;
	return SaveFileName;
}