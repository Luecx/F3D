//
// Created by finn on 5/31/24.
//

#ifndef F3D_MAT_H
#define F3D_MAT_H

#include <array>
#include <iostream>
#include <iomanip>
#include <cmath>

template<typename TYPE, int ROWS, int COLS>
class Matrix {

    using TTYPE = Matrix<TYPE, ROWS, COLS>;
    using DTYPE = std::array<TYPE, ROWS*COLS>;

    DTYPE data{};

    public:

    // construction from values, arrays and other matrices
    Matrix (const DTYPE& data) : data(data) {}
    Matrix () = default;

    // construction from other matrix and filling other values with 0
    template<typename T, int R, int C>
    Matrix(const Matrix<T, R, C>& other) {
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                if (i < R && j < C) {
                    (*this)(i, j) = (T) other(i, j);
                } else {
                    (*this)(i, j) = 0;
                }
            }
        }
    }

    // Constructor for 2-element vectors, enabling easy instantiation
    Matrix(TYPE x, TYPE y) {
        data[0] = x;
        data[1] = y;
    }

    // Constructor for 3-element vectors, enabling easy instantiation
    Matrix(TYPE x, TYPE y, TYPE z) {
        data[0] = x;
        data[1] = y;
        data[2] = z;
    }

    // Constructor for 4-element vectors, enabling easy instantiation
    Matrix(TYPE x, TYPE y, TYPE z, TYPE w) {
        data[0] = x;
        data[1] = y;
        data[2] = z;
        data[3] = w;
    }

    // operators for *=, +=, -=, /=
    TTYPE& operator*=(const TTYPE& other) {
        for (int i = 0; i < ROWS*COLS; ++i) { data[i] *= other.data[i]; }
        return *this;
    }
    TTYPE  operator* (const TTYPE& other) const {
        TTYPE result{*this};
        result *= other;
        return result;
    }
    TTYPE& operator*=(const TYPE& value) {
        for (auto& d : data) { d *= value; }
        return *this;
    }
    TTYPE  operator* (const TYPE& value) const {
        TTYPE result = *this;
        result *= value;
        return result;
    }

    TTYPE& operator+=(const TTYPE& other) {
        for (int i = 0; i < ROWS*COLS; ++i) { data[i] += other.data[i]; }
        return *this;
    }
    TTYPE  operator+ (const TTYPE& other) const {
        TTYPE result = *this;
        result += other;
        return result;
    }
    TTYPE& operator+=(const TYPE& value) {
        for (int i = 0; i < ROWS*COLS; ++i) { data[i] = data[i] + value; }
        return *this;
    }
    TTYPE  operator+ (const TYPE& value) const {
        TTYPE result;
        for (int i = 0; i < ROWS*COLS; ++i) { result.data[i] = data[i] + value; }
        return result;
    }

    TTYPE& operator-=(const TTYPE& other) {
        for (int i = 0; i < ROWS*COLS; ++i) { data[i] -= other.data[i]; }
        return *this;
    }
    TTYPE  operator- (const TTYPE& other) const {
        TTYPE result = *this;
        result -= other;
        return result;
    }
    TTYPE& operator-=(const TYPE& other) {
        for (int i = 0; i < ROWS*COLS; ++i) { data[i] -= other; }
        return *this;
    }
    TTYPE  operator- (const TYPE& other) const {
        TTYPE result = *this;
        result -= other;
        return result;
    }
    TTYPE operator-() const {
        TTYPE result;
        for (int i = 0; i < ROWS*COLS; ++i) { result.data[i] = -data[i]; }
        return result;
    }

    TTYPE& operator/=(const TTYPE& other) {
        for (int i = 0; i < ROWS*COLS; ++i) { data[i] /= other.data[i]; }
        return *this;
    }
    TTYPE  operator/ (const TTYPE& other) const {
        TTYPE result = *this;
        result /= other;
        return result;
    }
    TTYPE& operator/=(const TYPE& other) {
        for (int i = 0; i < ROWS*COLS; ++i) { data[i] /= other; }
        return *this;
    }
    TTYPE  operator/ (const TYPE& other) const {
        TTYPE result = *this;
        result /= other;
        return result;
    }

    // operators for comparison
    bool operator==(const TTYPE& other) const {
        for (int i = 0; i < ROWS*COLS; ++i) {
            if (data[i] != other.data[i]) { return false; }
        }
        return true;
    }
    bool operator!=(const TTYPE& other) const {
        return !(*this == other);
    }

    // stream operator with fixed width and precision
    friend std::ostream& operator<<(std::ostream& os, const TTYPE& mat) {
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                os << std::setw(10) << std::setprecision(4) << mat(i, j) << " ";
            }
            if (i < ROWS - 1)
                os << std::endl;
        }
        return os;
    }

    // transposition
    Matrix<TYPE, COLS, ROWS> transpose() const {
        Matrix<TYPE, COLS, ROWS> result;
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                result(j, i) = (*this)(i, j);
            }
        }
        return result;
    }

    // if either rows or cols is 1, enable the length function
    template<int R = ROWS, int C = COLS>
    typename std::enable_if<R == 1 || C == 1, TYPE>::type length() const {
        TYPE result = 0;
        for (int i = 0; i < ROWS*COLS; ++i) {
            result += data[i] * data[i];
        }
        return std::sqrt(result);
    }

    // if either rows or cols is 1, enable normalise and normalised functions
    // normalised returns a new vector, normalise modifies the current one
    template<int R = ROWS, int C = COLS>
    typename std::enable_if<R == 1 || C == 1, TTYPE>::type normalised() const {
        TYPE len = length();
        TTYPE result = *this;
        for (int i = 0; i < ROWS*COLS; ++i) {
            result.data[i] /= len;
        }
        return result;
    }
    template<int R = ROWS, int C = COLS>
    typename std::enable_if<R == 1 || C == 1, TTYPE>::type& normalise() {
        TYPE len = length();
        for (int i = 0; i < ROWS*COLS; ++i) {
            data[i] /= len;
        }
        return *this;
    }

    // sum function
    TYPE sum() const {
        TYPE result = 0;
        for (int i = 0; i < ROWS*COLS; ++i) {
            result += data[i];
        }
        return result;
    }

    // max and min
    TYPE max() const {
        TYPE result = data[0];
        for (int i = 1; i < ROWS*COLS; ++i) {
            if (data[i] > result) { result = data[i]; }
        }
        return result;
    }

    TYPE min() const {
        TYPE result = data[0];
        for (int i = 1; i < ROWS*COLS; ++i) {
            if (data[i] < result) { result = data[i]; }
        }
        return result;
    }

    // access operator
    TYPE& operator()(int row, int col) {
        return data[row*COLS + col];
    }
    // access operator in case one dimension is 1
    TYPE& operator()(int index) {
        return data[index];
    }
    // const access operator
    const TYPE& operator()(int row, int col) const {
        return data[row*COLS + col];
    }
    // const access operator in case one dimension is 1
    const TYPE& operator()(int index) const {
        return data[index];
    }
    // array operator only for vectors
    TYPE& operator[](int index) {
        return data[index];
    }
    const TYPE& operator[](int index) const {
        return data[index];
    }

    // for a 3x3 matrix, enable rotation functions as static functions
    template<int R = ROWS, int C = COLS>
    typename std::enable_if<R == 3 && C == 3, TTYPE>::type static rot_x(TYPE angle) {
        TTYPE result{{1, 0, 0,
                       0, std::cos(angle), -std::sin(angle),
                       0, std::sin(angle), std::cos(angle)}};
        return result;
    }
    template<int R = ROWS, int C = COLS>
    typename std::enable_if<R == 3 && C == 3, TTYPE>::type static rot_y(TYPE angle) {
        TTYPE result{{std::cos(angle), 0, std::sin(angle),
                       0, 1, 0,
                       -std::sin(angle), 0, std::cos(angle)}};
        return result;
    }
    template<int R = ROWS, int C = COLS>
    typename std::enable_if<R == 3 && C == 3, TTYPE>::type static rot_z(TYPE angle) {
        TTYPE result{{std::cos(angle), -std::sin(angle), 0,
                       std::sin(angle), std::cos(angle), 0,
                       0, 0, 1}};
        return result;
    }

    // for square matrices, enable identity function as static function
    template<int R = ROWS, int C = COLS>
    typename std::enable_if<R == C, TTYPE>::type static eye() {
        TTYPE result;
        for (int i = 0; i < ROWS; ++i) {
                result(i, i) = 1;
        }
        return result;
    }

    template<int R, int C>
    typename std::enable_if<R * C == 3 && ROWS * COLS == 3, TTYPE>::type cross(const Matrix<TYPE, R, C>& other) const {
        TTYPE result;
        result(0) = this->operator()(1) * other(2) - this->operator()(2) * other(1);
        result(1) = this->operator()(2) * other(0) - this->operator()(0) * other(2);
        result(2) = this->operator()(0) * other(1) - this->operator()(1) * other(0);
        return result;
    }

    // dot product as a sum of hadamard multiplication
    template<int R, int C>
    TYPE dot(const Matrix<TYPE, R, C>& other) const {
        TYPE result = 0;
        for (int i = 0; i < ROWS*COLS; ++i) {
            result += data[i] * other(i);
        }
        return result;
    }

    // matrix vector multiplication called matmul
    template<int R, int C>
    Matrix<TYPE, ROWS, C> matmul(const Matrix<TYPE, R, C>& other) const {
        Matrix<TYPE, ROWS, C> result;
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < C; ++j) {
                for (int k = 0; k < COLS; ++k) {
                    result(i, j) += (*this)(i, k) * other(k, j);
                }
            }
        }
        return result;
    }


    template<int R=ROWS, int C=COLS>
    typename std::enable_if<R * C == 3, TTYPE>::type reflect(const TTYPE& normal) const {
        TYPE dot_product = this->dot(normal);
        return *this - normal * (2 * dot_product);
    }

    template<int R=ROWS, int C=COLS>
    typename std::enable_if<R * C == 3, TTYPE>::type refract(const TTYPE& normal, TYPE eta) const {
        TYPE cos_theta_i = -this->dot(normal);
        TYPE sin_theta_t2 = eta * eta * (1 - cos_theta_i * cos_theta_i);
        if (sin_theta_t2 > 1) {
            // Total internal reflection
            return TTYPE{}; // or some other indication that no refraction occurs
        }
        TYPE cos_theta_t = std::sqrt(1 - sin_theta_t2);
        return (*this) * eta + normal * (eta * cos_theta_i - cos_theta_t);
    }

    template<int R = ROWS, int C = COLS>
    typename std::enable_if<R == 4 && C == 4, void>::type translate_3d(const Matrix<TYPE, 3, 1>& translation);

    template<int R = ROWS, int C = COLS>
    typename std::enable_if<R == 4 && C == 4, void>::type scale_3d(const Matrix<TYPE, 3, 1>& scale);

    template<int R = ROWS, int C = COLS>
    typename std::enable_if<R == 4 && C == 4, void>::type rotate_3d(TYPE angle, const Matrix<TYPE, 3, 1>& axis);

    template<int R = ROWS, int C = COLS>
    typename std::enable_if<R == 4 && C == 4, Matrix<TYPE, ROWS, COLS>>::type view_orthogonal(TYPE left, TYPE right, TYPE bottom, TYPE top, TYPE near, TYPE far);

    template<int R = ROWS, int C = COLS>
    typename std::enable_if<R == 4 && C == 4, Matrix<TYPE, ROWS, COLS>>::type view_perspective(TYPE fov, TYPE aspectRatio, TYPE near, TYPE far);

    template<int R = ROWS, int C = COLS>
    typename std::enable_if<R == 4 && C == 4, Matrix<TYPE, ROWS, COLS>>::type view_look_at(const Matrix<TYPE, 3, 1>& eye, const Matrix<TYPE, 3, 1>& center, const Matrix<TYPE, 3, 1>& up);

};





// definitions of some implementations
#include "mat_project.impl"
#include "mat_transform.impl"

// define basic types in short form
using Mat2f = Matrix<float, 2, 2>;
using Mat3f = Matrix<float, 3, 3>;
using Mat4f = Matrix<float, 4, 4>;

// vector types
using Vec2f = Matrix<float, 2, 1>;
using Vec3f = Matrix<float, 3, 1>;
using Vec4f = Matrix<float, 4, 1>;


#endif    // F3D_MAT_H
