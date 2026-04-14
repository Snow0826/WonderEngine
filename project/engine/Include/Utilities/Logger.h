#pragma once
#include <string>
#include <fstream>

namespace Logger {
	/// @brief ログファイルを開く
	std::ofstream OpenLogFile();

	/// @brief ログ出力
	/// @param os ログストリーム
	/// @param message ログメッセージ
	void Log(std::ofstream &os, const std::string &message);
};