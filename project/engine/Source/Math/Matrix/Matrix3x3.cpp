#include "Matrix3x3.h"
#include "Vector2.h"
#include "Vector3.h"
#include <cassert>

Matrix3x3 Matrix3x3::operator+(const Matrix3x3 &other) const {
	Matrix3x3 result;
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			result.m[i][j] = this->m[i][j] + other.m[i][j];
		}
	}
	return result;
}

Matrix3x3 Matrix3x3::operator-(const Matrix3x3 &other) const {
	Matrix3x3 result;
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			result.m[i][j] = this->m[i][j] - other.m[i][j];
		}
	}
	return result;
}

Matrix3x3 Matrix3x3::operator*(const Matrix3x3 &other) const {
	Matrix3x3 result;
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			for (size_t k = 0; k < 3; k++) {
				result.m[i][j] += this->m[i][k] * other.m[k][j];
			}
		}
	}
	return result;
}

Matrix3x3 Matrix3x3::operator*(float other) const {
	Matrix3x3 result;
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			result.m[i][j] = this->m[i][j] * other;
		}
	}
	return result;
}

Matrix3x3 Matrix3x3::operator/(float other) const {
	Matrix3x3 result;
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			result.m[i][j] = this->m[i][j] / other;
		}
	}
	return result;
}

void Matrix3x3::operator+=(const Matrix3x3 &other) {
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			this->m[i][j] += other.m[i][j];
		}
	}
}

void Matrix3x3::operator-=(const Matrix3x3 &other) {
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			this->m[i][j] -= other.m[i][j];
		}
	}
}

void Matrix3x3::operator*=(const Matrix3x3 &other) {
	Matrix3x3 result;
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			for (size_t k = 0; k < 3; k++) {
				result.m[i][j] += this->m[i][k] * other.m[k][j];
			}
		}
	}
	*this = result;
}

void Matrix3x3::operator*=(float other) {
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			m[i][j] *= other;
		}
	}
}

void Matrix3x3::operator/=(float other) {
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			m[i][j] /= other;
		}
	}
}

Vector2 Matrix3x3::operator*(const Vector2 &other) const {
	Vector2 result;
	result.x = m[0][0] * other.x + m[1][0] * other.y + m[2][0];
	result.y = m[0][1] * other.x + m[1][1] * other.y + m[2][1];
	float z = m[0][2] * other.x + m[1][2] * other.y + m[2][2];
	assert(z != 0.0f);
	result.x /= z;
	result.y /= z;
	return result;
}

Vector3 Matrix3x3::operator*(const Vector3 &other) const {
	Vector3 result;
	result.x = m[0][0] * other.x + m[1][0] * other.y + m[2][0] * other.z;
	result.y = m[0][1] * other.x + m[1][1] * other.y + m[2][1] * other.z;
	result.z = m[0][2] * other.x + m[1][2] * other.y + m[2][2] * other.z;
	return result;
}

Matrix3x3 Matrix3x3::operator-() const {
	Matrix3x3 result;
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			result.m[i][j] = -this->m[i][j];
		}
	}
	return result;
}

Matrix3x3 Matrix3x3::transpose() const {
	Matrix3x3 result;
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			result.m[i][j] = this->m[j][i];
		}
	}
	return result;
}

bool Matrix3x3::inverse(Matrix3x3 &result) const {
	float A[3][6] = {};
	constexpr float kMax_err = 1.0e-10f;

	// 拡大行列作成
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			A[i][j] = m[i][j];
			A[i][j + 3] = (i == j) ? 1.0f : 0.0f;
		}
	}

	// ガウス・ジョルダン
	for (size_t i = 0; i < 3; i++) {

		// ピボット選択
		float max = fabsf(A[i][i]);
		size_t max_i = i;

		for (size_t j = i + 1; j < 3; j++) {
			if (fabsf(A[j][i]) > max) {
				max = fabsf(A[j][i]);
				max_i = j;
			}
		}

		// 解けない
		if (max < kMax_err) {
			return false;
		}

		// 行交換
		if (i != max_i) {
			for (size_t j = 0; j < 6; j++) {
				std::swap(A[i][j], A[max_i][j]);
			}
		}

		// 正規化
		float invPivot = 1.0f / A[i][i];
		for (size_t j = 0; j < 6; j++) {
			A[i][j] *= invPivot;
		}

		// 消去
		for (size_t j = 0; j < 3; j++) {
			if (i == j) continue;

			float factor = A[j][i];
			for (size_t k = 0; k < 6; k++) {
				A[j][k] -= factor * A[i][k];
			}
		}
	}

	// 結果取り出し
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			result.m[i][j] = A[i][j + 3];
		}
	}

	return true;
}

Vector2 operator*(const Vector2 &vector, const Matrix3x3 &matrix) {
	Vector2 result;
	result.x = matrix.m[0][0] * vector.x + matrix.m[1][0] * vector.y + matrix.m[2][0];
	result.y = matrix.m[0][1] * vector.x + matrix.m[1][1] * vector.y + matrix.m[2][1];
	float z = matrix.m[0][2] * vector.x + matrix.m[1][2] * vector.y + matrix.m[2][2];
	assert(z != 0.0f);
	result.x /= z;
	result.y /= z;
	return result;
}

Vector3 operator*(const Vector3 &vector, const Matrix3x3 &matrix) {
	Vector3 result;
	result.x = matrix.m[0][0] * vector.x + matrix.m[1][0] * vector.y + matrix.m[2][0] * vector.z;
	result.y = matrix.m[0][1] * vector.x + matrix.m[1][1] * vector.y + matrix.m[2][1] * vector.z;
	result.z = matrix.m[0][2] * vector.x + matrix.m[1][2] * vector.y + matrix.m[2][2] * vector.z;
	return result;
}

Matrix3x3 MakeIdentity3x3() {
	Matrix3x3 result;
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			result.m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
	return result;
}

Matrix3x3 MakeScaleMatrix(const Vector2 &scale) {
	Matrix3x3 result = MakeIdentity3x3();
	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	return result;
}

Matrix3x3 MakeRotateMatrix(float radian) {
	Matrix3x3 result = MakeIdentity3x3();
	result.m[0][0] = std::cos(radian);
	result.m[0][1] = std::sin(radian);
	result.m[1][0] = -std::sin(radian);
	result.m[1][1] = std::cos(radian);
	return result;
}

Matrix3x3 MakeTranslateMatrix(const Vector2 &translation) {
	Matrix3x3 result = MakeIdentity3x3();
	result.m[2][0] = translation.x;
	result.m[2][1] = translation.y;
	return result;
}

Matrix3x3 MakeAffineMatrix(const Vector2 &scale, float radian, const Vector2 &translation) {
	return MakeScaleMatrix(scale) * MakeRotateMatrix(radian) * MakeTranslateMatrix(translation);
}