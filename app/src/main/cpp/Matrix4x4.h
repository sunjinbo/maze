//
// Created by sunjjinbo on 2022/1/26.
//

#ifndef MAZE_MATRIX4X4_H
#define MAZE_MATRIX4X4_H

#include <array>
#include <vector>

class Matrix4x4 {
public:
    float m[4][4];

    // Multiplies two matrices.
    Matrix4x4 operator*(const Matrix4x4& right);

    // Multiplies a matrix with a vector.
    std::array<float, 4> operator*(const std::array<float, 4>& vec);

    // Converts a matrix to an array of floats suitable for passing to OpenGL.
    std::array<float, 16> ToGlArray();

    /**
 * Converts an array of floats to a matrix.
 *
 * @param vec GL array
 * @return Obtained matrix
 */
    static Matrix4x4 GetMatrixFromGlArray(float* vec);

    /**
 * Construct a translation matrix.
 *
 * @param translation Translation array
 * @return Obtained matrix
 */
    static Matrix4x4 GetTranslationMatrix(const std::array<float, 3>& translation);
};

#endif //MAZE_MATRIX4X4_H
