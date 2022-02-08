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
};

struct Quatf {

public:
    float x;
    float y;
    float z;
    float w;

public:
    Quatf(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}

    Quatf() : x(0), y(0), z(0), w(1) {}

    static Quatf FromXYZW(float q[4]) { return Quatf(q[0], q[1], q[2], q[3]); }

    Matrix4x4 ToMatrix();
};

/**
 * Converts an array of floats to a matrix.
 *
 * @param vec GL array
 * @return Obtained matrix
 */
Matrix4x4 GetMatrixFromGlArray(float* vec);

/**
 * Construct a translation matrix.
 *
 * @param translation Translation array
 * @return Obtained matrix
 */
Matrix4x4 GetTranslationMatrix(const std::array<float, 3>& translation);

/**
 * Gets system boot time in nanoseconds.
 *
 * @return System boot time in nanoseconds
 */
int64_t GetBootTimeNano();

#endif //MAZE_MATRIX4X4_H
