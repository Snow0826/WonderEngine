#include "StringConverter.h"
#include <stringapiset.h>

std::wstring StringConverter::ConvertString(const std::string &str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char *>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char *>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

std::string StringConverter::ConvertString(const std::wstring &str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}

StringConverter::FilePathSet StringConverter::SeparateFilePath(const std::string &filePath) {
	FilePathSet filePathSet;
	size_t lastSlashPos = filePath.find_last_of("/\\");
	size_t lastDotPos = filePath.find_last_of('.');
	if (lastSlashPos != std::string::npos) {
		filePathSet.directory = filePath.substr(0, lastSlashPos + 1);
	} else {
		filePathSet.directory = "";
	}
	if (lastDotPos != std::string::npos && lastDotPos > lastSlashPos) {
		filePathSet.fileName = filePath.substr(lastSlashPos + 1, lastDotPos - lastSlashPos - 1);
		filePathSet.extension = filePath.substr(lastDotPos + 1);
	} else {
		filePathSet.fileName = filePath.substr(lastSlashPos + 1);
		filePathSet.extension = "";
	}
	return filePathSet;
}

std::pair<std::string, std::string> StringConverter::SeparateFileNameAndExtension(const std::string &fileNameWithExtension) {
	size_t lastDotPos = fileNameWithExtension.find_last_of('.');
	if (lastDotPos != std::string::npos) {
		std::string fileName = fileNameWithExtension.substr(0, lastDotPos);
		std::string extension = fileNameWithExtension.substr(lastDotPos + 1);
		return { fileName, extension };
	} else {
		return { fileNameWithExtension, "" };
	}
}