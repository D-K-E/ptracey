#pragma once
// matrix library from P. Shirley, Realistic Ray Tracing
#include <external.hpp>
#include <vec3.hpp>
//
class Matrix {
public:
  Matrix() {}
  Matrix(const Matrix &orig) {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        x[i][j] = orig.x[i][j];
  }
  void invert() {
    float det = determinant();
    Matrix inverse;
    inverse.x[0][0] =
        det3(x[1][1], x[1][2], x[1][3], x[2][1], x[2][2],
             x[2][3], x[3][1], x[3][2], x[3][3]) /
        det;
    inverse.x[0][1] =
        -det3(x[0][1], x[0][2], x[0][3], x[2][1], x[2][2],
              x[2][3], x[3][1], x[3][2], x[3][3]) /
        det;
    inverse.x[0][2] =
        det3(x[0][1], x[0][2], x[0][3], x[1][1], x[1][2],
             x[1][3], x[3][1], x[3][2], x[3][3]) /
        det;
    inverse.x[0][3] =
        -det3(x[0][1], x[0][2], x[0][3], x[1][1], x[1][2],
              x[1][3], x[2][1], x[2][2], x[2][3]) /
        det;

    inverse.x[1][0] =
        -det3(x[1][0], x[1][2], x[1][3], x[2][0], x[2][2],
              x[2][3], x[3][0], x[3][2], x[3][3]) /
        det;
    inverse.x[1][1] =
        det3(x[0][0], x[0][2], x[0][3], x[2][0], x[2][2],
             x[2][3], x[3][0], x[3][2], x[3][3]) /
        det;
    inverse.x[1][2] =
        -det3(x[0][0], x[0][2], x[0][3], x[1][0], x[1][2],
              x[1][3], x[3][0], x[3][2], x[3][3]) /
        det;
    inverse.x[2][0] =
        det3(x[1][0], x[1][1], x[1][3], x[2][0], x[2][1],
             x[2][3], x[3][0], x[3][1], x[3][3]) /
        det;
    inverse.x[2][1] =
        -det3(x[0][0], x[0][1], x[0][3], x[2][0], x[2][1],
              x[2][3], x[3][0], x[3][1], x[3][3]) /
        det;
    inverse.x[2][2] =
        det3(x[0][0], x[0][1], x[0][3], x[1][0], x[1][1],
             x[1][3], x[3][0], x[3][1], x[3][3]) /
        det;
    inverse.x[2][3] =
        -det3(x[0][0], x[0][1], x[0][3], x[1][0], x[1][1],
              x[1][3], x[2][0], x[2][1], x[2][3]) /
        det;

    inverse.x[3][0] =
        -det3(x[1][0], x[1][1], x[1][2], x[2][0], x[2][1],
              x[2][2], x[3][0], x[3][1], x[3][2]) /
        det;
    inverse.x[3][1] =
        det3(x[0][0], x[0][1], x[0][2], x[2][0], x[2][1],
             x[2][2], x[3][0], x[3][1], x[3][2]) /
        det;
    inverse.x[3][2] =
        -det3(x[0][0], x[0][1], x[0][2], x[1][0], x[1][1],
              x[1][2], x[3][0], x[3][1], x[3][2]) /
        det;
    inverse.x[3][3] =
        det3(x[0][0], x[0][1], x[0][2], x[1][0], x[1][1],
             x[1][2], x[2][0], x[2][1], x[2][2]) /
        det;
    *this = inverse;
  }
  void transpose() {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        float temp = x[i][j];
        x[i][j] = x[j][i];
        x[j][i] = temp;
      }
    }
  }
  Matrix getInverse() const {
    Matrix ret = *this;
    ret.invert();
    return ret;
  }
  Matrix getTranspose() const {
    Matrix ret = *this;
    ret.transpose();
    return ret;
  }
  Matrix &operator+=(const Matrix &right_op) {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        x[i][j] += right_op.x[i][j];
    return *this;
  }
  Matrix &operator-=(const Matrix &right_op) {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        x[i][j] -= right_op.x[i][j];
    return *this;
  }
  Matrix &operator*=(const Matrix &right_op) {
    Matrix ret = *this;
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++) {
        float sum = 0;
        for (int k = 0; k < 4; k++)
          sum += ret.x[i][k] * right_op.x[k][j];
        x[i][j] = sum;
      }
    return *this;
  }
  Matrix &operator*=(float right_op) {
    //
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        x[i][j] *= right_op;
    return *this;
  }
  friend Matrix operator-(const Matrix &left_op,
                          const Matrix &right_op);
  friend Matrix operator+(const Matrix &left_op,
                          const Matrix &right_op);
  friend Matrix operator*(const Matrix &left_op,
                          const Matrix &right_op);
  friend Vec3 operator*(const Matrix &left_op,
                        const Vec3 &right_op);
  friend Matrix operator*(const Matrix &left_op,
                          float right_op);
  friend Vec3 transformLoc(const Matrix &left_op,
                           const Vec3 &right_op);
  friend Vec3 transformVec(const Matrix &left_op,
                           const Vec3 &right_op);
  friend Matrix zeroMatrix();
  friend Matrix identityMatrix();
  friend Matrix translate(float _x, float _y, float _z);
  friend Matrix translate(const Point3 &disp);
  friend Matrix scale(float _x, float _y, float _z);
  friend Matrix scale(const Vec3 &d);
  friend Matrix scale_translate(const Point3 &disp,
                                const Vec3 &s);
  friend Matrix rotate_translate(const Point3 &disp,
                                 const Vec3 &s,
                                 float angle);
  friend Matrix rotate(const Vec3 &axis, float angle);
  friend Matrix rotateX(float angle);
  friend Matrix rotateY(float angle);
  friend Matrix rotateZ(float angle);
  friend Matrix viewMatrix(const Vec3 &eye,
                           const Vec3 &gaze,
                           const Vec3 &up);
  float determinant() {
    float det;
    //
    det = x[0][0] * det3(x[1][1], x[1][2], x[1][3], x[2][1],
                         x[2][2], x[2][3], x[3][1], x[3][2],
                         x[3][3]);

    det -= x[0][1] * det3(x[1][0], x[1][2], x[1][3],
                          x[2][0], x[2][2], x[2][3],
                          x[3][0], x[3][2], x[3][3]);

    det += x[0][2] * det3(x[1][0], x[1][1], x[1][3],
                          x[2][0], x[2][1], x[2][3],
                          x[3][0], x[3][1], x[3][3]);

    det -= x[0][3] * det3(x[1][0], x[1][1], x[1][2],
                          x[2][0], x[2][1], x[2][2],
                          x[3][0], x[3][1], x[3][2]);
    return det;
  }
  float x[4][4];
  // 3x3 matrix determinant — helper function
  inline float det3(float a, float b, float c, float d,
                    float e, float f, float g, float h,
                    float i) {
    return a * e * i + d * h * c + g * b * f - g * e * c -
           d * b * i - a * h * f;
  }
};
Matrix operator-(const Matrix &left_op,
                 const Matrix &right_op) {
  Matrix ret;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      ret.x[i][j] = left_op.x[i][j] - right_op.x[i][j];
  return ret;
}
Matrix operator+(const Matrix &left_op,
                 const Matrix &right_op) {
  Matrix ret;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      ret.x[i][j] = left_op.x[i][j] + right_op.x[i][j];
  return ret;
}
Matrix operator*(const Matrix &left_op,
                 const Matrix &right_op) {
  Matrix ret;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++) {
      float subt = 0.0;
      for (int k = 0; k < 4; k++)
        subt += left_op.x[i][k] * right_op.x[k][j];
      ret.x[i][j] = subt;
    }
  return ret;
}
Vec3 operator*(const Matrix &left_op,
               const Vec3 &right_op) {
  Vec3 ret;
  float varr[4] = {right_op.x(), right_op.y(), right_op.z(),
                   1.0f};
  float res[4];
  for (int i = 0; i < 4; i++) {
    float temp = 0.0f;
    for (int j = 0; j < 4; j++)
      temp += left_op.x[i][j] * varr[j];
    res[i] = temp;
  }
  ret[0] = res[0];
  ret[1] = res[1];
  ret[2] = res[2];
  return ret;
}
Matrix operator*(const Matrix &left_op, float right_op) {
  Matrix ret;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      ret.x[i][j] = left_op.x[i][j] * right_op;
  return ret;
}
Vec3 transformLoc(const Matrix &left_op,
                  const Vec3 &right_op) {
  return left_op * right_op;
}
Vec3 transformVec(const Matrix &left_op,
                  const Vec3 &right_op) {
  Vec3 ret;
  ret[0] = (right_op[0] * left_op.x[0][0]) +
           (right_op[1] * left_op.x[0][1]) +
           (right_op[2] * left_op.x[0][2]);

  ret[1] = right_op[0] * left_op.x[1][0] +
           right_op[1] * left_op.x[1][1] +
           right_op[2] * left_op.x[1][2];
  ret[2] = right_op[0] * left_op.x[2][0] +
           right_op[1] * left_op.x[2][1] +
           right_op[2] * left_op.x[2][2];
  return ret;
}
Matrix zeroMatrix() {
  Matrix ret;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      ret.x[i][j] = 0.0f;
  return ret;
}
Matrix identityMatrix() {
  //
  Matrix ret;
  ret.x[0][0] = 1.0;
  ret.x[0][1] = 0.0;
  ret.x[0][2] = 0.0;
  ret.x[0][3] = 0.0;
  ret.x[1][0] = 0.0;
  ret.x[1][1] = 1.0;
  ret.x[1][2] = 0.0;
  ret.x[1][3] = 0.0;
  ret.x[2][0] = 0.0;
  ret.x[2][1] = 0.0;
  ret.x[2][2] = 1.0;
  ret.x[2][3] = 0.0;
  ret.x[3][0] = 0.0;
  ret.x[3][1] = 0.0;
  ret.x[3][2] = 0.0;
  ret.x[3][3] = 1.0;
  return ret;
}
Matrix translate(float _x, float _y, float _z) {
  Matrix ret = identityMatrix();
  ret.x[0][3] = _x;
  ret.x[1][3] = _y;
  ret.x[2][3] = _z;
  return ret;
}
Matrix translate(const Point3 &disp) {
  return translate(disp.x(), disp.y(), disp.z());
}
Matrix scale(float _x, float _y, float _z) {
  Matrix ret = zeroMatrix();
  ret.x[0][0] = _x;
  ret.x[1][1] = _y;
  ret.x[2][2] = _z;
  ret.x[3][3] = 1.0;
  return ret;
}
Matrix scale(const Vec3 &d) {
  return scale(d.x(), d.y(), d.z());
}
Matrix rotate(const Vec3 &axis, float angle) {
  Vec3 _axis = to_unit(axis);
  Matrix ret;
  float x = _axis.x();
  float y = _axis.y();
  float z = _axis.z();
  float cosine = cos(angle);
  float sine = sin(angle);
  float t = 1.0f - cosine;
  ret.x[0][0] = t * x * x + cosine;
  ret.x[0][1] = t * x * y - sine * y;
  ret.x[0][2] = t * x * z + sine * y;
  ret.x[0][3] = 0.0f;
  ret.x[1][0] = t * x * y + sine * z;
  ret.x[1][1] = t * y * y + cosine;
  ret.x[1][2] = t * y * z - sine * x;
  ret.x[1][3] = 0.0f;
  ret.x[2][0] = t * x * z - sine * y;
  ret.x[2][1] = t * y * z + sine * x;
  ret.x[2][2] = t * z * z + cosine;
  ret.x[2][3] = 0.0f;
  ret.x[3][0] = 0.0f;
  ret.x[3][1] = 0.0f;
  ret.x[3][2] = 0.0f;
  ret.x[3][3] = 1.0f;
  return ret;
}
Matrix rotateX(float angle) {
  Matrix ret = identityMatrix();
  float costheta = cos(angle);
  float sintheta = sin(angle);
  ret.x[1][1] = costheta;
  ret.x[1][2] = -sintheta;
  ret.x[2][1] = sintheta;
  ret.x[2][2] = costheta;
  return ret;
}
Matrix rotateY(float angle) {
  // More efficient than arbitrary axis
  Matrix ret = identityMatrix();
  float cosine = cos(angle);
  float sine = sin(angle);
  ret.x[0][0] = cosine;
  ret.x[0][2] = sine;
  ret.x[2][0] = -sine;
  ret.x[2][2] = cosine;
  return ret;
}
Matrix rotateZ(float angle) {
  Matrix ret = identityMatrix();
  float cosine = cos(angle);
  float sine = sin(angle);
  ret.x[0][0] = cosine;
  ret.x[0][1] = -sine;
  ret.x[1][0] = sine;
  ret.x[1][1] = cosine;
  return ret;
}
Matrix scale_translate(const Point3 &disp, const Vec3 &s) {
  Matrix ret = scale(s);
  ret.x[0][3] = disp.x();
  ret.x[1][3] = disp.y();
  ret.x[2][3] = disp.z();
  return ret;
}
Matrix rotate_translate(const Point3 &p, const Vec3 &axis,
                        float angle) {
  Matrix rotMat;
  if (axis.x() == 1.0f && axis.y() == 0.0f &&
      axis.z() == 0.0f) {
    rotMat = rotateX(angle);
  } else if (axis.y() == 1.0f && axis.z() == 0.0f &&
             axis.x() == 0.0f) {
    rotMat = rotateY(angle);
  } else if (axis.z() == 1.0f && axis.x() == 0.0f &&
             axis.y() == 0.0f) {
    rotMat = rotateZ(angle);
  } else {
    rotMat = rotate(axis, angle);
  }

  rotMat.x[0][3] = p.x();
  rotMat.x[1][3] = p.y();
  rotMat.x[2][3] = p.z();
  return rotMat;
}
Matrix viewMatrix(const Vec3 &eye, const Vec3 &gaze,
                  const Vec3 &up) {
  Matrix ret = identityMatrix();
  // create an orthoganal basis from parameters
  Vec3 w = -(to_unit(gaze));
  Vec3 u = to_unit(cross(up, w));
  Vec3 v = cross(w, u);
  // rotate orthoganal basis to xyz basis
  ret.x[0][0] = u.x();
  ret.x[0][1] = u.y();
  ret.x[0][2] = u.z();
  ret.x[1][0] = v.x();
  ret.x[1][1] = v.y();
  ret.x[1][2] = v.z();
  ret.x[2][0] = w.x();
  ret.x[2][1] = w.y();
  ret.x[2][2] = w.z();
  // translare eye to xyz origin
  Matrix move = identityMatrix();
  move.x[0][3] = -(eye.x());
  move.x[1][3] = -(eye.y());
  move.x[2][3] = -(eye.z());
  ret = ret * move;
  return ret;
}
