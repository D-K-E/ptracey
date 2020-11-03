#pragma once
// matrix library from P. Shirley, Realistic Ray Tracing
#include <vec3.hpp>
//
class matrix {
public:
  matrix() {}
  matrix(const matrix &orig) {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        x[i][j] = orig.x[i][j];
  }
  void invert() {
    double det = determinant();
    matrix inverse;
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
        double temp = x[i][j];
        x[i][j] = x[j][i];
        x[j][i] = temp;
      }
    }
  }
  matrix getInverse() const {
    matrix ret = *this;
    ret.invert();
    return ret;
  }
  matrix getTranspose() const {
    matrix ret = *this;
    ret.transpose();
    return ret;
  }
  matrix &operator+=(const matrix &right_op) {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        x[i][j] += right_op.x[i][j];
    return *this;
  }
  matrix &operator-=(const matrix &right_op) {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        x[i][j] -= right_op.x[i][j];
    return *this;
  }
  matrix &operator*=(const matrix &right_op) {
    matrix ret = *this;
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++) {
        double sum = 0;
        for (int k = 0; k < 4; k++)
          sum += ret.x[i][k] * right_op.x[k][j];
        x[i][j] = sum;
      }
    return *this;
  }
  matrix &operator*=(double right_op) {
    //
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        x[i][j] *= right_op;
    return *this;
  }
  friend matrix operator-(const matrix &left_op,
                          const matrix &right_op);
  friend matrix operator+(const matrix &left_op,
                          const matrix &right_op);
  friend matrix operator*(const matrix &left_op,
                          const matrix &right_op);
  friend vec3 operator*(const matrix &left_op,
                        const vec3 &right_op);
  friend matrix operator*(const matrix &left_op,
                          double right_op);
  friend vec3 transformLoc(const matrix &left_op,
                           const vec3 &right_op);
  friend vec3 transformVec(const matrix &left_op,
                           const vec3 &right_op);
  friend matrix zeroMatrix();
  friend matrix identityMatrix();
  friend matrix translateM(double _x, double _y, double _z);
  friend matrix translateM(const point3 &disp);
  friend matrix scaleM(double _x, double _y, double _z);
  friend matrix scaleM(const vec3 &d);
  friend matrix scale_translate(const point3 &disp,
                                const vec3 &s);
  friend matrix rotate_translate(const point3 &disp,
                                 const vec3 &s,
                                 double angle);
  friend matrix rotateM(const vec3 &axis, double angle);
  friend matrix rotateXM(double angle);
  friend matrix rotateYM(double angle);
  friend matrix rotateZM(double angle);
  friend matrix viewMatrix(const vec3 &eye,
                           const vec3 &gaze,
                           const vec3 &up);
  double determinant() {
    double det;
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
  double x[4][4];
  // 3x3 matrix determinant — helper function
  inline double det3(double a, double b, double c, double d,
                     double e, double f, double g, double h,
                     double i) {
    return a * e * i + d * h * c + g * b * f - g * e * c -
           d * b * i - a * h * f;
  }
};
matrix operator-(const matrix &left_op,
                 const matrix &right_op) {
  matrix ret;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      ret.x[i][j] = left_op.x[i][j] - right_op.x[i][j];
  return ret;
}
matrix operator+(const matrix &left_op,
                 const matrix &right_op) {
  matrix ret;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      ret.x[i][j] = left_op.x[i][j] + right_op.x[i][j];
  return ret;
}
matrix operator*(const matrix &left_op,
                 const matrix &right_op) {
  matrix ret;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++) {
      double subt = 0.0;
      for (int k = 0; k < 4; k++)
        subt += left_op.x[i][k] * right_op.x[k][j];
      ret.x[i][j] = subt;
    }
  return ret;
}
vec3 operator*(const matrix &left_op,
               const vec3 &right_op) {
  vec3 ret;
  double varr[4] = {right_op.x(), right_op.y(),
                    right_op.z(), 1.0f};
  double res[4];
  for (int i = 0; i < 4; i++) {
    double temp = 0.0f;
    for (int j = 0; j < 4; j++)
      temp += left_op.x[i][j] * varr[j];
    res[i] = temp;
  }
  ret[0] = res[0];
  ret[1] = res[1];
  ret[2] = res[2];
  return ret;
}
matrix operator*(const matrix &left_op, double right_op) {
  matrix ret;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      ret.x[i][j] = left_op.x[i][j] * right_op;
  return ret;
}
vec3 transformLoc(const matrix &left_op,
                  const vec3 &right_op) {
  return left_op * right_op;
}
vec3 transformVec(const matrix &left_op,
                  const vec3 &right_op) {
  vec3 ret;
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
matrix zeroMatrix() {
  matrix ret;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      ret.x[i][j] = 0.0f;
  return ret;
}
matrix identityMatrix() {
  //
  matrix ret;
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
matrix translateM(double _x, double _y, double _z) {
  matrix ret = identityMatrix();
  ret.x[0][3] = _x;
  ret.x[1][3] = _y;
  ret.x[2][3] = _z;
  return ret;
}
matrix translateM(const point3 &disp) {
  return translateM(disp.x(), disp.y(), disp.z());
}
matrix scaleM(double _x, double _y, double _z) {
  matrix ret = zeroMatrix();
  ret.x[0][0] = _x;
  ret.x[1][1] = _y;
  ret.x[2][2] = _z;
  ret.x[3][3] = 1.0;
  return ret;
}
matrix scaleM(const vec3 &d) {
  return scaleM(d.x(), d.y(), d.z());
}
matrix rotateM(const vec3 &axis, double angle) {
  vec3 _axis = unit_vector(axis);
  matrix ret;
  double x = _axis.x();
  double y = _axis.y();
  double z = _axis.z();
  double cosine = cos(angle);
  double sine = sin(angle);
  double t = 1.0f - cosine;
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
matrix rotateXM(double angle) {
  matrix ret = identityMatrix();
  double costheta = cos(angle);
  double sintheta = sin(angle);
  ret.x[1][1] = costheta;
  ret.x[1][2] = -sintheta;
  ret.x[2][1] = sintheta;
  ret.x[2][2] = costheta;
  return ret;
}
matrix rotateYM(double angle) {
  // More efficient than arbitrary axis
  matrix ret = identityMatrix();
  double cosine = cos(angle);
  double sine = sin(angle);
  ret.x[0][0] = cosine;
  ret.x[0][2] = sine;
  ret.x[2][0] = -sine;
  ret.x[2][2] = cosine;
  return ret;
}
matrix rotateZM(double angle) {
  matrix ret = identityMatrix();
  double cosine = cos(angle);
  double sine = sin(angle);
  ret.x[0][0] = cosine;
  ret.x[0][1] = -sine;
  ret.x[1][0] = sine;
  ret.x[1][1] = cosine;
  return ret;
}
matrix scale_translate(const point3 &disp, const vec3 &s) {
  matrix ret = scaleM(s);
  ret.x[0][3] = disp.x();
  ret.x[1][3] = disp.y();
  ret.x[2][3] = disp.z();
  return ret;
}
matrix rotate_translate(const point3 &p, const vec3 &axis,
                        double angle) {
  matrix rotMat;
  if (axis.x() == 1.0f && axis.y() == 0.0f &&
      axis.z() == 0.0f) {
    rotMat = rotateXM(angle);
  } else if (axis.y() == 1.0f && axis.z() == 0.0f &&
             axis.x() == 0.0f) {
    rotMat = rotateYM(angle);
  } else if (axis.z() == 1.0f && axis.x() == 0.0f &&
             axis.y() == 0.0f) {
    rotMat = rotateZM(angle);
  } else {
    rotMat = rotateM(axis, angle);
  }

  rotMat.x[0][3] = p.x();
  rotMat.x[1][3] = p.y();
  rotMat.x[2][3] = p.z();
  return rotMat;
}
matrix viewMatrix(const vec3 &eye, const vec3 &gaze,
                  const vec3 &up) {
  matrix ret = identityMatrix();
  // create an orthoganal basis from parameters
  vec3 w = -(unit_vector(gaze));
  vec3 u = unit_vector(cross(up, w));
  vec3 v = cross(w, u);
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
  matrix move = identityMatrix();
  move.x[0][3] = -(eye.x());
  move.x[1][3] = -(eye.y());
  move.x[2][3] = -(eye.z());
  ret = ret * move;
  return ret;
}
