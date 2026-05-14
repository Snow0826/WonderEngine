#include "Matrix4x4.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Quaternion.h"
#include <cassert>
#include <algorithm>
#include <numbers>

Matrix4x4 Matrix4x4::operator+(const Matrix4x4 &other) const {
	Matrix4x4 result;
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			result.m[i][j] = this->m[i][j] + other.m[i][j];
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::operator-(const Matrix4x4 &other) const {
	Matrix4x4 result;
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			result.m[i][j] = this->m[i][j] - other.m[i][j];
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4 &other) const {
	Matrix4x4 result;
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			for (size_t k = 0; k < 4; k++) {
				result.m[i][j] += this->m[i][k] * other.m[k][j];
			}
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::operator*(float other) const {
	Matrix4x4 result;
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			result.m[i][j] = this->m[i][j] * other;
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::operator/(float other) const {
	Matrix4x4 result;
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			result.m[i][j] = this->m[i][j] / other;
		}
	}
	return result;
}

void Matrix4x4::operator+=(const Matrix4x4 &other) {
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			this->m[i][j] += other.m[i][j];
		}
	}
}

void Matrix4x4::operator-=(const Matrix4x4 &other) {
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			this->m[i][j] -= other.m[i][j];
		}
	}
}

void Matrix4x4::operator*=(const Matrix4x4 &other) {
	Matrix4x4 result;
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			for (size_t k = 0; k < 4; k++) {
				result.m[i][j] += this->m[i][k] * other.m[k][j];
			}
		}
	}
	*this = result;
}

void Matrix4x4::operator*=(float other) {
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			m[i][j] *= other;
		}
	}
}

void Matrix4x4::operator/=(float other) {
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			m[i][j] /= other;
		}
	}
}

Vector3 Matrix4x4::operator*(const Vector3 &other) const {
	Vector3 result;
	result.x = m[0][0] * other.x + m[1][0] * other.y + m[2][0] * other.z + m[3][0];
	result.y = m[0][1] * other.x + m[1][1] * other.y + m[2][1] * other.z + m[3][1];
	result.z = m[0][2] * other.x + m[1][2] * other.y + m[2][2] * other.z + m[3][2];
	float w = m[0][3] * other.x + m[1][3] * other.y + m[2][3] * other.z + m[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;
	return result;
}

Vector4 Matrix4x4::operator*(const Vector4 &other) const {
	Vector4 result;
	result.x = m[0][0] * other.x + m[1][0] * other.y + m[2][0] * other.z + m[3][0] * other.w;
	result.y = m[0][1] * other.x + m[1][1] * other.y + m[2][1] * other.z + m[3][1] * other.w;
	result.z = m[0][2] * other.x + m[1][2] * other.y + m[2][2] * other.z + m[3][2] * other.w;
	result.w = m[0][3] * other.x + m[1][3] * other.y + m[2][3] * other.z + m[3][3] * other.w;
	return result;
}

Matrix4x4 Matrix4x4::operator-() const {
	Matrix4x4 result;
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			result.m[i][j] = -this->m[i][j];
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::transpose() const {
	Matrix4x4 result;
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			result.m[i][j] = this->m[j][i];
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::inverse() const {
	Matrix4x4 result;
	float A[4][8] = { 0 };
	size_t n = 4;
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

Vector3 operator*(const Vector3 &vector, const Matrix4x4 &matrix) {
	Vector3 result;
	result.x = matrix.m[0][0] * vector.x + matrix.m[1][0] * vector.y + matrix.m[2][0] * vector.z + matrix.m[3][0];
	result.y = matrix.m[0][1] * vector.x + matrix.m[1][1] * vector.y + matrix.m[2][1] * vector.z + matrix.m[3][1];
	result.z = matrix.m[0][2] * vector.x + matrix.m[1][2] * vector.y + matrix.m[2][2] * vector.z + matrix.m[3][2];
	float w = matrix.m[0][3] * vector.x + matrix.m[1][3] * vector.y + matrix.m[2][3] * vector.z + matrix.m[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;
	return result;
}

Vector4 operator*(const Vector4 &vector, const Matrix4x4 &matrix) {
	Vector4 result;
	result.x = matrix.m[0][0] * vector.x + matrix.m[1][0] * vector.y + matrix.m[2][0] * vector.z + matrix.m[3][0] * vector.w;
	result.y = matrix.m[0][1] * vector.x + matrix.m[1][1] * vector.y + matrix.m[2][1] * vector.z + matrix.m[3][1] * vector.w;
	result.z = matrix.m[0][2] * vector.x + matrix.m[1][2] * vector.y + matrix.m[2][2] * vector.z + matrix.m[3][2] * vector.w;
	result.w = matrix.m[0][3] * vector.x + matrix.m[1][3] * vector.y + matrix.m[2][3] * vector.z + matrix.m[3][3] * vector.w;
	return result;
}

Vector3 TransformNormal(const Vector3 &vector, const Matrix4x4 &matrix) {
	Vector3 result;
	result.x = matrix.m[0][0] * vector.x + matrix.m[1][0] * vector.y + matrix.m[2][0] * vector.z;
	result.y = matrix.m[0][1] * vector.x + matrix.m[1][1] * vector.y + matrix.m[2][1] * vector.z;
	result.z = matrix.m[0][2] * vector.x + matrix.m[1][2] * vector.y + matrix.m[2][2] * vector.z;
	return result;
}

Vector3 TransformNormalAbs(const Vector3 &vector, const Matrix4x4 &matrix) {
	Vector3 result;
	result.x = std::abs(matrix.m[0][0]) * vector.x + std::abs(matrix.m[1][0]) * vector.y + std::abs(matrix.m[2][0]) * vector.z;
	result.y = std::abs(matrix.m[0][1]) * vector.x + std::abs(matrix.m[1][1]) * vector.y + std::abs(matrix.m[2][1]) * vector.z;
	result.z = std::abs(matrix.m[0][2]) * vector.x + std::abs(matrix.m[1][2]) * vector.y + std::abs(matrix.m[2][2]) * vector.z;
	return result;
}

Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result;
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			result.m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
	return result;
}

Matrix4x4 MakeScaleMatrix(const Vector3 &scale) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	return result;
}

Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[1][1] = std::cos(radian);
	result.m[1][2] = std::sin(radian);
	result.m[2][1] = -std::sin(radian);
	result.m[2][2] = std::cos(radian);
	return result;
}

Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[0][0] = std::cos(radian);
	result.m[0][2] = -std::sin(radian);
	result.m[2][0] = std::sin(radian);
	result.m[2][2] = std::cos(radian);
	return result;
}

Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[0][0] = std::cos(radian);
	result.m[0][1] = std::sin(radian);
	result.m[1][0] = -std::sin(radian);
	result.m[1][1] = std::cos(radian);
	return result;
}

Matrix4x4 MakeRotateMatrix(const Vector3 &rotate) {
	return MakeRotateXMatrix(rotate.x) * MakeRotateYMatrix(rotate.y) * MakeRotateZMatrix(rotate.z);
}

Matrix4x4 MakeRotateMatrixAroundAxis(const Vector3 &axis, float angle) {
	Matrix4x4 result = MakeIdentity4x4();
	float c = std::cos(angle);
	float s = std::sin(angle);
	float t = 1.0f - c;
	result.m[0][0] = t * axis.x * axis.x + c;
	result.m[0][1] = t * axis.x * axis.y + s * axis.z;
	result.m[0][2] = t * axis.x * axis.z - s * axis.y;
	result.m[1][0] = t * axis.x * axis.y - s * axis.z;
	result.m[1][1] = t * axis.y * axis.y + c;
	result.m[1][2] = t * axis.y * axis.z + s * axis.x;
	result.m[2][0] = t * axis.x * axis.z + s * axis.y;
	result.m[2][1] = t * axis.y * axis.z - s * axis.x;
	result.m[2][2] = t * axis.z * axis.z + c;
	return result;
}

Matrix4x4 DirectionToDirection(const Vector3 &from, const Vector3 &to) {
	if (from.lengthSquare() < 1e-6f || to.lengthSquare() < 1e-6f) {
		return MakeIdentity4x4();
	}

	Vector3 f = from.normalized();
	Vector3 t = to.normalized();
	float dot = f.dot(t);
	dot = std::clamp(dot, -1.0f, 1.0f);

	// 同方向
	if (dot > 0.999999f) {
		return MakeIdentity4x4();
	}

	// 逆方向
	if (dot < -0.999999f) {
		Vector3 axis = Vector3{ 0.0f, 0.0f, 1.0f }.cross(f);
		if (std::abs(f.x) < std::abs(f.y)) {
			if (std::abs(f.x) < std::abs(f.z)) {
				axis = Vector3{ 1.0f, 0.0f, 0.0f }.cross(f);
			}
		} else if (std::abs(f.y) < std::abs(f.z)) {
			axis = Vector3{ 0.0f, 1.0f, 0.0f }.cross(f);
		}
		axis = axis.normalized();
		return MakeRotateMatrixAroundAxis(axis, std::numbers::pi_v<float>);
	}

	Vector3 axis = f.cross(t).normalized();
	float angle = std::acos(dot);
	return MakeRotateMatrixAroundAxis(axis, angle);
}

Matrix4x4 LookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up) {
	Vector3 zAxis = eye.normalized(target);
	Vector3 xAxis = up.cross(zAxis).normalized();
	Vector3 yAxis = zAxis.cross(xAxis).normalized();
	Matrix4x4 result = MakeIdentity4x4();
	result.m[0][0] = xAxis.x;
	result.m[0][1] = xAxis.y;
	result.m[0][2] = xAxis.z;
	result.m[1][0] = yAxis.x;
	result.m[1][1] = yAxis.y;
	result.m[1][2] = yAxis.z;
	result.m[2][0] = zAxis.x;
	result.m[2][1] = zAxis.y;
	result.m[2][2] = zAxis.z;
	return result;
}

Matrix4x4 MakeTranslateMatrix(const Vector3 &translate) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	return result;
}

Matrix4x4 MakeAffineMatrix(const Vector3 &scale, const Vector3 &rotate, const Vector3 &translate) {
	return MakeScaleMatrix(scale) * MakeRotateMatrix(rotate) * MakeTranslateMatrix(translate);
}

Matrix4x4 MakeAffineMatrix(const Vector3 &scale, const Matrix4x4 &rotateMatrix, const Vector3 &translate) {
	return MakeScaleMatrix(scale) * rotateMatrix * MakeTranslateMatrix(translate);
}

Matrix4x4 MakeAffineMatrix(const Vector3 &scale, const Quaternion &rotate, const Vector3 &translate) {
	return MakeScaleMatrix(scale) * rotate.MakeRotateMatrix() * MakeTranslateMatrix(translate);
}

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result;
	float f = 1.0f / std::tan(fovY / 2.0f);
	float d = farClip - nearClip;
	result.m[0][0] = f / aspectRatio;
	result.m[1][1] = f;
	result.m[2][2] = farClip / d;
	result.m[2][3] = 1.0f;
	result.m[3][2] = -nearClip * farClip / d;
	return result;
}

Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[0][0] = 2.0f / (right - left);
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[2][2] = 1.0f / (farClip - nearClip);
	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);
	return result;
}

Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[0][0] = width / 2.0f;
	result.m[1][1] = -height / 2.0f;
	result.m[2][2] = maxDepth - minDepth;
	result.m[3][0] = left + width / 2.0f;
	result.m[3][1] = top + height / 2.0f;
	result.m[3][2] = minDepth;
	return result;
}