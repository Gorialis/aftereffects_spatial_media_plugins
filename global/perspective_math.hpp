#pragma once

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


constexpr long double CONSTANT_PI = 3.141592653589793238462643383279502884L;

enum FOV_Type {
    FOV_Type_Vertical = 0,
    FOV_Type_Horizontal,
    FOV_Type_Diagonal
};


float scale_from_fov(glm::ivec2 size, float half_fov, FOV_Type type) {
    switch(type) {
        case FOV_Type_Vertical:
        default:
        return tanf(half_fov) * 2.0f / static_cast<float>(size.y);

        case FOV_Type_Horizontal:
        return tanf(half_fov) * 2.0f / static_cast<float>(size.x);

        case FOV_Type_Diagonal:
        return tanf(half_fov) * 2.0f / sqrtf(powf(static_cast<float>(size.x), 2.0f) + powf(static_cast<float>(size.y), 2.0f));
    }
}

glm::vec3 get_globevec_from_perspective(glm::ivec2 position, glm::ivec2 size, float half_fov, FOV_Type type) {
    float scale = scale_from_fov(size, half_fov, type);

    float half_width = static_cast<float>(size.x) / 2.0f;
    float half_height = static_cast<float>(size.y) / 2.0f;
    float w_x = (static_cast<float>(position.x) - half_width) * scale;
    float w_y = (static_cast<float>(position.y) - half_height) * scale;

    return glm::normalize(
        glm::vec3(
            w_x,
            w_y,
            1.0f
        )
    );
}

glm::vec3 get_globevec_from_equirectangular(glm::ivec2 position, glm::ivec2 size) {
    float half_width = static_cast<float>(size.x) / 2.0f;
    float half_height = static_cast<float>(size.y) / 2.0f;

    float rel_x = static_cast<float>(CONSTANT_PI) * (static_cast<float>(position.x) - half_width) / half_width;
    float rel_y = static_cast<float>(CONSTANT_PI) * (static_cast<float>(position.y) - half_height) / static_cast<float>(size.y);

    float c_y = cosf(rel_y);
    float s_y = sinf(rel_y);

    return glm::vec3(
        sinf(rel_x) * c_y,
        s_y,
        cosf(rel_x) * c_y
    );
}

void transform_globevec_inside_out(float pitch, float yaw, float roll, float* target) {
    float s_p = sinf(pitch);
    float c_p = cosf(pitch);

    float s_y = sinf(yaw);
    float c_y = cosf(yaw);

    float s_r = sinf(roll);
    float c_r = cosf(roll);

    target[0] = c_r * c_y + s_p * s_r * s_y;
    target[1] = -c_y * s_r + c_r * s_p * s_y;
    target[2] = c_p * s_y;
    target[3] = c_p * s_r;
    target[4] = c_p * c_r;
    target[5] = -s_p;
    target[6] = c_y * s_p * s_r - c_r * s_y;
    target[7] = c_r * c_y * s_p + s_r * s_y;
    target[8] = c_p * c_y;
}

void transform_globevec_outside_in(float pitch, float yaw, float roll, float* target) {
    float s_p = sinf(pitch);
    float c_p = cosf(pitch);

    float s_y = sinf(yaw);
    float c_y = cosf(yaw);

    float s_r = sinf(roll);
    float c_r = cosf(roll);

    target[0] = c_r * c_y + s_p * s_r * s_y;
    target[1] = c_p * s_r;
    target[2] = c_y * s_p * s_r - c_r * s_y;
    target[3] = -c_y * s_r + c_r * s_p * s_y;
    target[4] = c_p * c_r;
    target[5] = c_r * c_y * s_p + s_r * s_y;
    target[6] = c_p * s_y;
    target[7] = -s_p;
    target[8] = c_p * c_y;
}

inline glm::vec3 matrix_transform(glm::vec3 vec, float* matrix) {
    return {
        matrix[0] * vec.x + matrix[1] * vec.y + matrix[2] * vec.z,
        matrix[3] * vec.x + matrix[4] * vec.y + matrix[5] * vec.z,
        matrix[6] * vec.x + matrix[7] * vec.y + matrix[8] * vec.z
    };
}

glm::vec2 get_perspective_from_globevec(glm::vec3 globevec, glm::ivec2 size, float half_fov, FOV_Type type) {
    if (globevec.z <= 0.0f)
        return {-1, -1};

    float scale = scale_from_fov(size, half_fov, type);

    float half_width = static_cast<float>(size.x) / 2.0f;
    float half_height = static_cast<float>(size.y) / 2.0f;

    float w_x = globevec.x / (globevec.z * scale);
    float w_y = globevec.y / (globevec.z * scale);

    return glm::vec2(
        w_x + half_width,
        w_y + half_height
    );
}

glm::vec2 get_equirectangular_from_globevec(glm::vec3 globevec, glm::ivec2 size) {
    float half_width = static_cast<float>(size.x) / 2.0f;
    float half_height = static_cast<float>(size.y) / 2.0f;

    float yaw = atan2f(globevec.x, globevec.z);
    float pitch = atan2f(globevec.y, sqrtf(powf(globevec.x, 2.0f) + powf(globevec.z, 2.0f)));

    return glm::vec2(
        half_width + (yaw * half_width / static_cast<float>(CONSTANT_PI)),
        half_height + (pitch * 2.0f * half_height / static_cast<float>(CONSTANT_PI))
    );
}
