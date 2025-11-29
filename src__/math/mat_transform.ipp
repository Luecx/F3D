//
// Created by finn on 5/31/24.
//

#ifndef F3D_MAT_TRANSFORM_IMPL
#define F3D_MAT_TRANSFORM_IMPL

template<typename TYPE, int ROWS, int COLS>
template<int R, int C>
typename std::enable_if<R == 4 && C == 4, void>::type Matrix<TYPE, ROWS, COLS>::rotate_3d(TYPE angle, const Matrix<TYPE, 3, 1>& axis) {
    TYPE c         = (TYPE) cos(angle);
    TYPE s         = (TYPE) sin(angle);
    TYPE oneminusc = 1.0f - c;
    TYPE xy        = axis[0] * axis[1];
    TYPE yz        = axis[1] * axis[2];
    TYPE xz        = axis[0] * axis[2];
    TYPE xs        = axis[0] * s;
    TYPE ys        = axis[1] * s;
    TYPE zs        = axis[2] * s;

    TYPE f00       = axis[0] * axis[0] * oneminusc + c;
    TYPE f01       = xy * oneminusc + zs;
    TYPE f02       = xz * oneminusc - ys;
    // n[3] not used
    TYPE f10 = xy * oneminusc - zs;
    TYPE f11 = axis[1] * axis[1] * oneminusc + c;
    TYPE f12 = yz * oneminusc + xs;
    // n[7] not used
    TYPE f20      = xz * oneminusc + ys;
    TYPE f21      = yz * oneminusc - xs;
    TYPE f22      = axis[2] * axis[2] * oneminusc + c;

    TYPE t00      = (*this)(0, 0) * f00 + (*this)(0, 1) * f01 + (*this)(0, 2) * f02;
    TYPE t01      = (*this)(1, 0) * f00 + (*this)(1, 1) * f01 + (*this)(1, 2) * f02;
    TYPE t02      = (*this)(2, 0) * f00 + (*this)(2, 1) * f01 + (*this)(2, 2) * f02;
    TYPE t03      = (*this)(3, 0) * f00 + (*this)(3, 1) * f01 + (*this)(3, 2) * f02;
    TYPE t10      = (*this)(0, 0) * f10 + (*this)(0, 1) * f11 + (*this)(0, 2) * f12;
    TYPE t11      = (*this)(1, 0) * f10 + (*this)(1, 1) * f11 + (*this)(1, 2) * f12;
    TYPE t12      = (*this)(2, 0) * f10 + (*this)(2, 1) * f11 + (*this)(2, 2) * f12;
    TYPE t13      = (*this)(3, 0) * f10 + (*this)(3, 1) * f11 + (*this)(3, 2) * f12;
    (*this)(0, 2) = (*this)(0, 0) * f20 + (*this)(0, 1) * f21 + (*this)(0, 2) * f22;
    (*this)(1, 2) = (*this)(1, 0) * f20 + (*this)(1, 1) * f21 + (*this)(1, 2) * f22;
    (*this)(2, 2) = (*this)(2, 0) * f20 + (*this)(2, 1) * f21 + (*this)(2, 2) * f22;
    (*this)(3, 2) = (*this)(3, 0) * f20 + (*this)(3, 1) * f21 + (*this)(3, 2) * f22;
    (*this)(0, 0) = t00;
    (*this)(1, 0) = t01;
    (*this)(2, 0) = t02;
    (*this)(3, 0) = t03;
    (*this)(0, 1) = t10;
    (*this)(1, 1) = t11;
    (*this)(2, 1) = t12;
    (*this)(3, 1) = t13;
}
template<typename TYPE, int ROWS, int COLS>
template<int R, int C>
typename std::enable_if<R == 4 && C == 4, void>::type Matrix<TYPE, ROWS, COLS>::scale_3d(const Matrix<TYPE, 3, 1>& scale) {
    (*this)(0, 0) = (*this)(0, 0) * scale(0);
    (*this)(1, 0) = (*this)(1, 0) * scale(0);
    (*this)(2, 0) = (*this)(2, 0) * scale(0);
    (*this)(3, 0) = (*this)(3, 0) * scale(0);
    (*this)(0, 1) = (*this)(0, 1) * scale(1);
    (*this)(1, 1) = (*this)(1, 1) * scale(1);
    (*this)(2, 1) = (*this)(2, 1) * scale(1);
    (*this)(3, 1) = (*this)(3, 1) * scale(1);
    (*this)(0, 2) = (*this)(0, 2) * scale(2);
    (*this)(1, 2) = (*this)(1, 2) * scale(2);
    (*this)(2, 2) = (*this)(2, 2) * scale(2);
    (*this)(3, 2) = (*this)(3, 2) * scale(2);
}
template<typename TYPE, int ROWS, int COLS>
template<int R, int C>
typename std::enable_if<R == 4 && C == 4, void>::type Matrix<TYPE, ROWS, COLS>::translate_3d(const Matrix<TYPE, 3, 1>& translation) {
    (*this)(0, 3) += (*this)(0, 0) * translation[0] + (*this)(0, 1) * translation[1] + (*this)(0, 2) * translation[2];
    (*this)(1, 3) += (*this)(1, 0) * translation[0] + (*this)(1, 1) * translation[1] + (*this)(1, 2) * translation[2];
    (*this)(2, 3) += (*this)(2, 0) * translation[0] + (*this)(2, 1) * translation[1] + (*this)(2, 2) * translation[2];
    (*this)(3, 3) += (*this)(3, 0) * translation[0] + (*this)(3, 1) * translation[1] + (*this)(3, 2) * translation[2];
}


#endif    // F3D_MAT_TRANSFORM_IMPL
