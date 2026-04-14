#pragma once
#include <string>

namespace StringConverter {

	/// @brief ファイルパスセット
	struct FilePathSet {
		std::string directory;	// ディレクトリ
		std::string fileName;	// ファイル名
		std::string extension;	// 拡張子
	};

	/// @brief 文字列変換(std::string -> std::wstring)
	/// @param str string文字列
	/// @return wstring文字列
	std::wstring ConvertString(const std::string &str);

	/// @brief 文字列変換(std::wstring -> std::string)
	/// @param str wstring文字列
	/// @return string文字列
	std::string ConvertString(const std::wstring &str);

	/// @brief ファイルパスを分解する
	/// @param filePath ファイルパス
	/// @return ファイルパスセット
	FilePathSet SeparateFilePath(const std::string &filePath);

	/// @brief ファイル名と拡張子を分離する
	/// @param fileNameWithExtension ファイル名+拡張子
	/// @return ファイル名と拡張子のペア
	std::pair<std::string, std::string> SeparateFileNameAndExtension(const std::string &fileNameWithExtension);
}