#include "Matrix.h"

#include <cassert>

#include "MathHelpers.h"
#include <cmath>
#include <immintrin.h>

//#define USE_SIMD

namespace dae {
	Matrix::Matrix(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis, const Vector3& t) :
		Matrix({ xAxis, 0 }, { yAxis, 0 }, { zAxis, 0 }, { t, 1 })
	{
	}

	Matrix::Matrix(const Vector4& xAxis, const Vector4& yAxis, const Vector4& zAxis, const Vector4& t)
	{
		data[0] = xAxis;
		data[1] = yAxis;
		data[2] = zAxis;
		data[3] = t;
	}

	Matrix::Matrix(const Matrix& m)
	{
		data[0] = m[0];
		data[1] = m[1];
		data[2] = m[2];
		data[3] = m[3];
	}

	Vector3 Matrix::TransformVector(const Vector3& v) const
	{
		return TransformVector(v[0], v[1], v[2]);
	}

	Vector3 Matrix::TransformVector(float x, float y, float z) const
	{
		#ifdef USE_SIMD
		__m128 _xyz, _curr, _xm, _total{};

		_xyz = _mm_set_ps(x, y, z, 0);

		for (uint32_t i{}; i < 3; ++i)
		{
			_xm = _mm_set_ps(data[i].x, data[i].y, data[i].z, 0);
			_curr = _mm_set1_ps(_xyz.m128_f32[3 - i]);

			_total = _mm_add_ps(_total, _mm_mul_ps(_xm, _curr));
		}

		return Vector3{ _total.m128_f32[3], _total.m128_f32[2], _total.m128_f32[1] };
		#endif
		
		const Vector4 d0{ data[0] * x };
		const Vector4 d1{ data[1] * y };
		const Vector4 d2{ data[2] * z };

		return Vector3{
			d0.x + d1.x + d2.x,
			d0.y + d1.y + d2.y,
			d0.z + d1.z + d2.z,
		};


		/*return Vector3{
			data[0].x * x + data[1].x * y + data[2].x * z,
			data[0].y * x + data[1].y * y + data[2].y * z,
			data[0].z * x + data[1].z * y + data[2].z * z
		};*/
	}

	Vector3 Matrix::TransformPoint(const Vector3& p) const
	{
		return TransformPoint(p[0], p[1], p[2]);
	}

	Vector3 Matrix::TransformPoint(float x, float y, float z) const
	{
		#ifdef USE_SIMD
		__m128 _xyz, _curr, _xm, _total{};

		_xyz = _mm_set_ps(x, y, z, 0);

		for (uint32_t i{}; i < 3; ++i)
		{
			_xm = _mm_set_ps(data[i].x, data[i].y, data[i].z, 0);
			_curr = _mm_set1_ps(_xyz.m128_f32[3 - i]);

			_total = _mm_add_ps(_total, _mm_mul_ps(_xm, _curr));
		}

		_xm = _mm_set_ps(data[3].x, data[3].y, data[3].z, 0);
		_total = _mm_add_ps(_total, _xm);

		return Vector3{ _total.m128_f32[3], _total.m128_f32[2], _total.m128_f32[1] };
		#endif

		const Vector4 d0{ data[0] * x };
		const Vector4 d1{ data[1] * y };
		const Vector4 d2{ data[2] * z };

		return Vector3{
			d0.x + d1.x + d2.x + data[3].x,
			d0.y + d1.y + d2.y + data[3].y,
			d0.z + d1.z + d2.z + data[3].z,
		};

		/*return Vector3{
			data[0].x * x + data[1].x * y + data[2].x * z + data[3].x,
			data[0].y * x + data[1].y * y + data[2].y * z + data[3].y,
			data[0].z * x + data[1].z * y + data[2].z * z + data[3].z,
		};*/
	}

	const Matrix& Matrix::Transpose()
	{
		Matrix result{};
		for (int r{ 0 }; r < 4; ++r)
		{
			for (int c{ 0 }; c < 4; ++c)
			{
				result[r][c] = data[c][r];
			}
		}

		data[0] = result[0];
		data[1] = result[1];
		data[2] = result[2];
		data[3] = result[3];

		return *this;
	}

	Matrix Matrix::Transpose(const Matrix& m)
	{
		Matrix out{ m };
		out.Transpose();

		return out;
	}

	Vector3 Matrix::GetAxisX() const
	{
		return data[0];
	}

	Vector3 Matrix::GetAxisY() const
	{
		return data[1];
	}

	Vector3 Matrix::GetAxisZ() const
	{
		return data[2];
	}

	Vector3 Matrix::GetTranslation() const
	{
		return data[3];
	}
	Matrix Matrix::CreateTranslation(float x, float y, float z)
	{
		Matrix matrix = {
			Vector4{1,0,0,0},
			Vector4{0,1,0,0},
			Vector4{0,0,1,0},
			Vector4{x,y,z,1}
		};

		return matrix;
	}

	Matrix Matrix::CreateTranslation(const Vector3& t)
	{
		return Matrix::CreateTranslation(t.x, t.y, t.z);
	}

	Matrix Matrix::CreateRotationX(float pitch)
	{
		Matrix matrix = {
			Vector4{1, 0 ,0 ,0},
			Vector4{0, cosf(pitch), -sin(pitch), 0},
			Vector4{0, sinf(pitch), cosf(pitch), 0},
			Vector4{0, 0, 0, 1},
		};

		return matrix;
	}

	Matrix Matrix::CreateRotationY(float yaw)
	{
		Matrix matrix = {
			Vector4{cosf(yaw), 0, -sinf(yaw), 0},
			Vector4{0, 1, 0, 0},
			Vector4{sinf(yaw), 0, cos(yaw), 0},
			Vector4{0,0,0,1}
		};

		return matrix;
	}

	Matrix Matrix::CreateRotationZ(float roll)
	{
		Matrix matrix = {
			Vector4{cosf(roll), sinf(roll), 0, 0},
			Vector4{-sinf(roll), cosf(roll), 0, 0},
			Vector4{0, 0, 1, 0},
			Vector4{0, 0, 0, 1},
		};

		return matrix;
	}

	Matrix Matrix::CreateRotation(const Vector3& r)
	{
		return CreateRotationX(r.x) * CreateRotationY(r.y) * CreateRotationZ(r.z);
	}

	Matrix Matrix::CreateRotation(float pitch, float yaw, float roll)
	{
		return CreateRotation({ pitch, yaw, roll });
	}

	Matrix Matrix::CreateScale(float sx, float sy, float sz)
	{
		Matrix matrix = {
			Vector4{sx, 0, 0, 0},
			Vector4{0, sy, 0, 0},
			Vector4{0, 0, sz, 0},
			Vector4{0,0,0,1 },
		};

		return matrix;
	}

	Matrix Matrix::CreateScale(const Vector3& s)
	{
		return CreateScale(s[0], s[1], s[2]);
	}

#pragma region Operator Overloads
	Vector4& Matrix::operator[](int index)
	{
		assert(index <= 3 && index >= 0);
		return data[index];
	}

	Vector4 Matrix::operator[](int index) const
	{
		assert(index <= 3 && index >= 0);
		return data[index];
	}

	Matrix Matrix::operator*(const Matrix& m) const
	{
		Matrix result{};
		Matrix m_transposed = Transpose(m);

		for (int r{ 0 }; r < 4; ++r)
		{
			for (int c{ 0 }; c < 4; ++c)
			{
				auto inter = Vector4::Dot(data[r], m_transposed[c]);
				result[r][c] = inter;
			}
		}

		return result;
	}

	const Matrix& Matrix::operator*=(const Matrix& m)
	{
		Matrix copy{ *this };
		Matrix m_transposed = Transpose(m);

		for (int r{ 0 }; r < 4; ++r)
		{
			for (int c{ 0 }; c < 4; ++c)
			{
				data[r][c] = Vector4::Dot(copy[r], m_transposed[c]);
			}
		}

		return *this;
	}
#pragma endregion
}