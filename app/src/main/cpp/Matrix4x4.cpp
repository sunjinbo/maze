//
// Created by sunjjinbo on 2022/1/26.
//

#include "Matrix4x4.h"

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& right) {
    Matrix4x4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                result.m[i][j] += this->m[k][j] * right.m[i][k];
            }
        }
    }
    return result;
}

std::array<float, 4> Matrix4x4::operator*(const std::array<float, 4>& vec) {
    std::array<float, 4> result;
    for (int i = 0; i < 4; ++i) {
        result[i] = 0;
        for (int k = 0; k < 4; ++k) {
            result[i] += this->m[k][i] * vec[k];
        }
    }
    return result;
}

std::array<float, 16> Matrix4x4::ToGlArray() {
    std::array<float, 16> result;
    memcpy(&result[0], m, 16 * sizeof(float));
    return result;
}

Matrix4x4 Matrix4x4::GetMatrixFromGlArray(float* vec) {
    Matrix4x4 result;
    memcpy(result.m, vec, 16 * sizeof(float));
    return result;
}

Matrix4x4 Matrix4x4::GetTranslationMatrix(const std::array<float, 3>& translation) {
    return {{{1.0f, 0.0f, 0.0f, 0.0f},
                    {0.0f, 1.0f, 0.0f, 0.0f},
                    {0.0f, 0.0f, 1.0f, 0.0f},
                    {translation.at(0), translation.at(1), translation.at(2), 1.0f}}};
}

float Matrix4x4::AngleBetweenVectors(const std::array<float, 4>& vec1,
                          const std::array<float, 4>& vec2) {
    return std::acos(
            std::max(-1.f, std::min(1.f, VectorDotProduct(vec1, vec2) /
                                         (VectorNorm(vec1) * VectorNorm(vec2)))));
}
