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

Matrix3x3 Matrix3x3::inverse() const {

	Matrix3x3 result;
	float A[3][6] = { 0 };
	size_t n = 3;
	float a;
	constexpr float kMax_err = 1.0E-10f;

	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			A[i][j] = m[i][j];
			A[i][j + n] = (i == j) ? 1.0f : 0.0f;
		}
	}

	for (size_t i = 0; i < n; i++) {

		float max = fabsf(A[i][i]);
		size_t max_i = i;

		for (size_t j = i + 1; j < n; j++) {
			if (fabsf(A[i][i]) > max) {
				max = fabsf(A[i][i]);
				max_i = i;
			}
		}

		if (fabsf(A[i][i]) <= kMax_err) {
			break;
		}

		if (i != max_i) {
			for (size_t j = 0; j < n * 2; j++) {
				float tmp = A[max_i][j];
				A[max_i][j] = A[i][j];
				A[i][j] = tmp;
			}
		}

		a = 1.0f / A[i][i];

		for (size_t j = 0; j < n * 2; j++) {
			A[i][j] *= a;
		}

		for (size_t j = 0; j < n; j++) {

			if (i == j) {
				continue;
			}

			a = -A[j][i];

			for (size_t k = 0; k < n * 2; k++) {
				A[j][k] += A[i][k] * a;
			}

		}

	}

	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			result.m[i][j] = A[i][j + n];
		}
	}

	return result;

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