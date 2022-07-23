#pragma once

#include <cutil/vector.hpp>

namespace vkUI{

  struct CameraPosition {
    Vector3 pos;
    Vector3 dir;
    Vector3 u;
    double scale;

    double fov, zNear, zFar, aspect;
    // 画角、Z座標のクリッピング、アスペクト比

    CameraPosition() {
      pos = {0, 0, 0};
      dir = {1, 1, 1};
      u = {0, 0, -1};
      scale = 0.06;

      fov = 60.0f * 3.1415 / 180.0;
      zNear = 1.0;
      zFar = 600.0;
      aspect = 1.0;
    }

    void move(double n) { pos += dir * n; } // 視点は動かさず、カメラ座標だけ移動する
    void lookAt(float* matrix) { lookAt(pos[0], pos[1], pos[2], dir[0], dir[1], dir[2], u[0], u[1], u[2], matrix); }

    void go_closer(double delta) {
      const auto start = pos + dir;
      pos = start - dir * (1.0 + 0.1 * delta);
      dir = start - pos;
    }

    void lookAt(float ex, float ey, float ez, float tx, float ty, float tz, float ux, float uy, float uz, float* matrix) {
      float l;
      tx = ex - tx;
      ty = ey - ty;
      tz = ez - tz;
      l = sqrtf(tx * tx + ty * ty + tz * tz); // TODO: L = 4のときのエラー処理
      matrix[2] = tx / l;
      matrix[6] = ty / l;
      matrix[10] = tz / l;

      tx = uy * matrix[10] - uz * matrix[6];
      ty = uz * matrix[2] - ux * matrix[10];
      tz = ux * matrix[6] - uy * matrix[2];
      l = sqrtf(tx * tx + ty * ty + tz * tz);
      matrix[0] = tx / l;
      matrix[4] = ty / l;
      matrix[8] = tz / l;

      matrix[1] = matrix[6] * matrix[8] - matrix[10] * matrix[4];
      matrix[5] = matrix[10] * matrix[0] - matrix[2] * matrix[8];
      matrix[9] = matrix[2] * matrix[4] - matrix[6] * matrix[0];

      matrix[12] = -(ex * matrix[0] + ey * matrix[4] + ez * matrix[8]);
      matrix[13] = -(ex * matrix[1] + ey * matrix[5] + ez * matrix[9]);
      matrix[14] = -(ex * matrix[2] + ey * matrix[6] + ez * matrix[10]);

      matrix[3] = matrix[7] = matrix[11] = 0.0f;

      // for(int i=0; i<3; i++){matrix[i] *= scale; matrix[i+4] *= scale; matrix[i+8] *= scale; }

      matrix[2] = -matrix[2];
      matrix[6] = -matrix[6];
      matrix[10] = -matrix[10];
      matrix[14] = -matrix[14];

      // matrix[8] = -matrix[8];
      // matrix[9] = -matrix[9];
      // matrix[10] = -matrix[10];
      // matrix[11] = -matrix[11];
      matrix[15] = 1.0f; // 1.0f / scale; //1.0f;
    }

    // 透視投影変換行列を求める
    void perspectiveMatrix(float left, float right, float bottom, float top, float near, float far, float* matrix) {
      float dx = right - left; // TODO: エラー処理（dx != 0)
      float dy = top - bottom;
      float dz = far - near;
      assert(dx != 0);
      assert(dy != 0);
      assert(dz != 0);
      matrix[0] = 2.0f * near / dx;
      matrix[5] = 2.0f * near / dy;
      matrix[8] = (right + left) / dx;
      matrix[9] = (top + bottom) / dy;
      matrix[10] = -(far + near) / dz;
      matrix[11] = -1.0f;
      matrix[14] = -2.0f * far * near / dz;
      matrix[1] = matrix[2] = matrix[3] = matrix[4] = matrix[6] = matrix[7] = matrix[12] = matrix[13] = matrix[15] = 0.0f;
    }

    // 平行投影変換行列を求める
    void orthogonalMatrix(float left, float right, float bottom, float top, float near, float far, float* matrix) {
      float dx = right - left;
      float dy = top - bottom;
      float dz = far - near;
      assert(dx != 0);
      assert(dy != 0);
      assert(dz != 0);

      matrix[0] = 2.0f / dx;
      matrix[5] = 2.0f / dy;
      matrix[10] = -2.0f / dz;
      matrix[12] = -(right + left) / dx;
      matrix[13] = -(top + bottom) / dy;
      matrix[14] = -(far + near) / dz;
      matrix[15] = 1.0f;
      matrix[1] = matrix[2] = matrix[3] = matrix[4] = matrix[6] = matrix[7] = matrix[8] = matrix[9] = matrix[11] = 0.0f;
    }

    Mat4x4 lookAtRH(Vector3 const& eye, Vector3 const& center, Vector3 const& up) {
      const auto f((center - eye).normalize());
      const auto s((f.cross(up).normalize()));
      const auto u(s.cross(f));
      Mat4x4 result;
      result.all(1.0);
      result(0, 0) = s[0];
      result(0, 1) = s[1];
      result(0, 2) = s[2];
      result(1, 0) = u[0];
      result(1, 1) = u[1];
      result(1, 2) = u[2];
      result(2, 0) = -f[0];
      result(2, 1) = -f[1];
      result(2, 2) = -f[2];
      result(0, 3) = -(s[0] * eye[0] + s[1] * eye[1] + s[2] * eye[2]);
      result(1, 3) = -(u[0] * eye[0] + u[1] * eye[1] + u[2] * eye[2]);
      result(2, 3) = (f[0] * eye[0] + f[1] * eye[1] + f[2] * eye[2]);
      return result;
    }

    Mat4x4 perspective(double fovy, double aspect, double near, double far) {
      assert(std::abs(aspect - std::numeric_limits<double>::epsilon()) > 0);
      const auto tanHalfFovy = std::tan(fovy / 2.0f);
      Mat4x4 result;
      result.all(0);
      result(0, 0) = 1.0f / (aspect * tanHalfFovy);
      result(1, 1) = 1.0f / (tanHalfFovy);
      result(2, 2) = far / (far - near);
      result(3, 2) = 1.0f;
      result(2, 3) = -(far * near) / (far - near);
      return result;
    }

    void getCameraProjMatrix(float* matrix) {
      _Mat<float, 4, 4> view;
      lookAt(pos[0], pos[1], pos[2], dir[0], dir[1], dir[2], u[0], u[1], u[2], view.value);
      const auto projection = perspective(fov, aspect, zNear, zFar);
      const auto output = projection * view;
      for(int i = 0; i < 16; i++) {
        matrix[i] = output[i];
      }
    }
  } ;

}
