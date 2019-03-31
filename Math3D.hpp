#pragma once

#include <cmath>


namespace Math3D {

	template <typename T = float>
	class Vector3D {
		public:
			Vector3D(T X, T Y, T Z) : x(X), y(Y), z(Z) {};

			const Vector3D<T> operator*(T coefficient) const {
				return Vector3D(x * coefficient, y * coefficient, z * coefficient);
			};

			const Vector3D<T> operator*(const Vector3D<T>& other) const {
				return Vector3D(x * other.x, y * other.y, z * other.z);
			}

			const Vector3D<T> operator/(T coefficient) const {
				return Vector3D(x / coefficient, y / coefficient, z / coefficient);
			};

			const Vector3D<T> operator/(const Vector3D<T>& other) const {
				return Vector3D(x / other.x, y / other.y, z / other.z);
			}

			T x, y, z;
	};

	template <typename T = float>
	T magnitude(const Vector3D<T>& src) {
		return sqrt(pow(src.x, static_cast<T>(2)) + pow(src.y, static_cast<T>(2)) + pow(src.z, static_cast<T>(2)));
	};

	template <typename T = float>
	Vector3D<T> normalize(const Vector3D<T>& src) {
		return src / magnitude(src);
	};

	template <typename T = float>
	T radians(T degrees) {
		return degrees * static_cast<T>(0.01745329251994329576923690768489);
	};

	template <typename T = float>
	Vector3D<T> rotate(const Vector3D<T>& src, T angle, const Vector3D<T>& axis) {
		Vector3D<T> n = normalize(axis);

		T c = cos(angle);
		T s = sin(angle);
		T ic = static_cast<T>(1) - c;

		Vector3D<T> v = n * ic;

		T x = (
			(c + v.x * n.x) * src.x +
			(v.x * n.y + s * n.z) * src.y +
			(v.x * n.z - s * n.y) * src.z
		);

		T y = (
			(v.y * n.x - s * n.z) * src.x +
			(c + v.y * n.y) * src.y +
			(v.y * n.z + s * n.x) * src.z
		);

		T z = (
			(v.z * n.x + s * n.y) * src.x +
			(v.z * n.y - s * n.x) * src.y +
			(c + v.z * n.z) * src.z
		);

		return Vector3D<T>(x, y, z);
	};

	template <typename T = float>
	Vector3D<T> perspective(const Vector3D<T>& src, T fovy, T aspect, T zNear = 0.5f, T zFar = 1.5f) {
		T halfFov = tan(fovy / static_cast<T>(2));

		T x = src.x / (aspect * halfFov);
		T y = src.y / halfFov;
		T z = (((-zFar - zNear) / (zFar - zNear)) * src.z) + (-static_cast<T>(2) * zFar * zNear / (zFar - zNear));
		T w = -static_cast<T>(1) * src.z;

		return Vector3D<T>(x / w, y / w, z / w);
	}

}
