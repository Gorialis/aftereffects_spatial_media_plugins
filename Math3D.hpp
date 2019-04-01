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

		return sin(xi) / xi;
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
				float yawDeg = (static_cast<float>(x) * 360.0f / output.x) - 180.0f;
				float pitchDeg = (static_cast<float>(y) * 180.0f / output.y) - 90.0f;

				glm::mat4 model = glm::rotate(
					glm::rotate(
						glm::mat4(static_cast<T>(1)),
						glm::radians(yawDeg),
						glm::vec3(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0))
					),
					glm::radians(pitchDeg),
					glm::vec3(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0))
				);

				glm::vec4 homogenous = transform * model * glm::vec4(static_cast<T>(0), static_cast<T>(0), static_cast<T>(-1), static_cast<T>(1));

				return glm::vec3(homogenous / homogenous.w);
			}

			bool inBounds(glm::vec3 r) {
				return !(
					r.z < static_cast<T>(-1) ||
					r.z > static_cast<T>(1) ||
					r.x < static_cast<T>(-1) ||
					r.x > input.x + static_cast<T>(1) ||
					r.y < static_cast<T>(-1) ||
					r.y > input.y + static_cast<T>(1)
				);
			}

		private:
			glm::vec2 input, output;
			glm::mat4 transform;
	};

}
