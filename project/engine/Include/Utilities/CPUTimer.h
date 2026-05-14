#pragma once
#include <chrono>

/// @brief CPUタイマー
class CPUTimer {
public:
    /// @brief タイマーの開始
    void Begin() {
        start_ = std::chrono::high_resolution_clock::now();
    }

	/// @brief タイマーの終了
    void End() {
        end_ = std::chrono::high_resolution_clock::now();
    }

	/// @brief 経過時間の取得
	/// @return 経過時間(ミリ秒)
    double GetMs() const {
        return std::chrono::duration<double, std::milli>(end_ - start_).count();
    }

private:
    std::chrono::high_resolution_clock::time_point start_;
    std::chrono::high_resolution_clock::time_point end_;
};