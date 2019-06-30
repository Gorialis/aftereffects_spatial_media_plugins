#pragma once

#include <cmath>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace Math3D {
	template <typename T = float>
	T sinc(T x) {
		if (x == static_cast<T>(0))
			return static_cast<T>(1);

		T xi = glm::radians(x * static_cast<T>(180));

		return glm::sin(xi) / xi;
	}

	template <typename T = float>
	T lanczos(T x, T a = static_cast<T>(3)) {
		if (-a <= x || x < a)
			return sinc(x) * sinc(x / a);

		return 0;
	}

	template <typename T = float>
	class TransformRenderer {
		public:
			TransformRenderer(glm::vec2 input, glm::vec2 output, T fov, T pitch, T yaw) : input(input), output(output) {
				glm::mat4 view = glm::rotate(
					glm::rotate(
						glm::mat4(static_cast<T>(1)),
						-pitch,
						glm::vec3(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0))
					),
					-yaw,
					glm::vec3(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0))
				);

				glm::mat4 projection = glm::perspective(
					fov,
					input.x / input.y,
					static_cast<T>(0.5),
					static_cast<T>(1.5)
				);

				glm::mat4 offset = glm::translate(
					glm::scale(
						glm::mat4(static_cast<T>(1)),
						glm::vec3(-input.x / static_cast<T>(2), input.y / static_cast<T>(2), 1)
					),
					glm::vec3(static_cast<T>(-1), static_cast<T>(1), static_cast<T>(0))
				);

				transform = offset * projection * view;
			}

			glm::vec3 Render(int32_t x, int32_t y) {
				T yaw = glm::radians((static_cast<T>(x) * 360.0f / output.x) - 180.0f);
				T pitch = glm::radians((static_cast<T>(y) * 180.0f / output.y) - 90.0f);

				T y_c = glm::cos(yaw);
				T y_s = glm::sin(yaw);
				T p_c = glm::cos(pitch);
				T p_s = glm::sin(pitch);

				glm::vec4 vector(
					-y_s * p_c,
					p_s,
					-y_c * p_c,
					static_cast<T>(1)
				);

				glm::vec4 homogenous(
					transform[0].x * vector.x + transform[1].x * vector.y + transform[2].x * vector.z + transform[3].x * vector.w,
					transform[0].y * vector.x + transform[1].y * vector.y + transform[2].y * vector.z + transform[3].y * vector.w,
					transform[0].z * vector.x + transform[1].z * vector.y + transform[2].z * vector.z + transform[3].z * vector.w,
					transform[0].w * vector.x + transform[1].w * vector.y + transform[2].w * vector.z + transform[3].w * vector.w
				);

				return homogenous / homogenous.w;
			}

			bool inBounds(glm::vec3 r) {
				return (
					r.z >= static_cast<T>(-1) &&
					r.z <= static_cast<T>(1) &&
					r.x >= static_cast<T>(-1) &&
					r.x <= input.x + static_cast<T>(1) &&
					r.y >= static_cast<T>(-1) &&
					r.y <= input.y + static_cast<T>(1)
				);
			}

		private:
			glm::vec2 input, output;
			glm::mat4 transform;
	};

}
