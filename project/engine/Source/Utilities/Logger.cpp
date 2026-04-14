#include "Logger.h"
#include <debugapi.h>
#include <filesystem>

std::ofstream Logger::OpenLogFile() {

	// ログのディレクトリの作成
	std::filesystem::create_directory("logs");

	// 現在時刻を取得(UTC時刻)
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	// 現在時刻を秒単位に変換
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);

	// 現在時刻を日本時間(PCの設定時間)に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };

	// 現在時刻を文字列に変換
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);

	// ログファイルのパスを作成
	std::string logFilePath = std::string("logs/") + dateString + ".log";

	// ファイルに書き込み準備
	std::ofstream logStream(logFilePath);

	return logStream;

}

void Logger::Log(std::ofstream &os, const std::string &message) {
	os << message << std::endl;
	OutputDebugStringA(message.c_str());
}