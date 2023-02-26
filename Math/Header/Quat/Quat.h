#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include "cmath"

#include "Vec3/Vec3.h"
#include "Mat4/Mat4.h"
#include "Mat3/Mat3.h"

namespace lm
{
	template <typename T> class Quat
	{
		private:
			T w;
			Vec3<T> v;

		public:
			Quat() : v(0, 0, 0)
			{
				this->w = 0;
				
			}

			Quat(const T init) : v(init)
			{
				this->w = init;
				
			}

			Quat(const T w, const T x, const T y, const T z) : v(x, y, z)
			{
				this->w = w;
			}

			Quat(const T w, lm::Vec3<T> pV) : v(pV)
			{
				this->w = w;
			}

			Quat(const Quat<T>& quat)
			{
				this->w = quat.w;
				this->v = quat.v;
			}

			Quat(Quat<T>&& quat) noexcept
			{
				this->w = std::move(quat.w);
				this->v = std::move(quat.v);
			}

			Quat& operator=(const Quat<T>& quat)
			{
				if (this == &quat)
					return *this;

				this->w = quat.w;
				this->v = quat.v;

				return *this;
			}

			Quat& operator=(Quat<T>&& quat) noexcept
			{
				if (this == &quat)
					return *this;

				this->w = std::move(quat.w);
				this->v = std::move(quat.v);

				return *this;
			}

			float norm() 
			{

				T scalar = w * w;
				float imaginary = v.dotProduct(v);

				return std::sqrt(scalar + imaginary);
			}

			void normalize() 
			{
				float n = norm();
				if (n != 0) 
				{

					float normValue = 1 / n;

					w *= normValue;
					v *= normValue;
				}
			}

			Quat<T> normalized()
			{
				float n = norm();
				if (n != 0)
				{

					float normValue = 1 / n;

					T newW = w * normValue;
					Vec3<T> newV = v * normValue;
					return Quat<T>(newW, newV);
				}

				return Quat<T>(*this);
			}

			Mat4<T> toMat4()
			{
				lm::Quat<T> base = this->normalized();
				lm::Mat4<T> mat4;

				mat4[0][0] = 1 - (2 * (base.v.Y() * base.v.Y())) - (2 * (base.v.Z() * base.v.Z()));
				mat4[1][0] = 2 * ((base.v.X() * base.v.Y()) - (base.v.Z() * base.w));
				mat4[2][0] = 2 * ((base.v.X() * base.v.Z()) + (base.w * base.v.Y()));
				mat4[3][0] = 0;

				mat4[0][1] = 2 * ((base.v.X() * base.v.Y()) + (base.w * base.v.Z()));
				mat4[1][1] = 1 - (2 * (base.v.X() * base.v.X())) - (2 * (base.v.Z() * base.v.Z()));
				mat4[2][1] = 2 * ((base.v.Y() * base.v.Z()) - (base.w * base.v.X()));
				mat4[3][1] = 0;

				mat4[0][2] = 2 * ((base.v.X() * base.v.Z()) - (base.w * base.v.Y()));
				mat4[1][2] = 2 * ((base.v.Y() * base.v.Z()) + (base.w * base.v.X()));
				mat4[2][2] = 1 - (2 * (base.v.X() * base.v.X())) - (2 * (base.v.Y() * base.v.Y()));
				mat4[3][2] = 0;

				mat4[0][3] = 0;
				mat4[1][3] = 0;
				mat4[2][3] = 0;
				mat4[3][3] = 1;

				return mat4;
			}

			static Quat<T> toQuat(Mat4<T> a)
			{
				Quat<T> q;

				float trace = a[0][0] + a[1][1] + a[2][2];
				if (trace > 0)
				{
					q.w = std::sqrtf(trace + 1.0f) * 0.5f;
					float s = 0.25f / q.w;

					q.v.X() = (a[2][1] - a[1][2]) * s;
					q.v.Y() = (a[0][2] - a[2][0]) * s;
					q.v.Z() = (a[1][0] - a[0][1]) * s;
				}
				else if (a[0][0] > a[1][1] && a[0][0] > a[2][2])
				{
					q.v.X() = std::sqrtf(a[0][0] - a[1][1] - a[2][2] + 1.0f) * 0.5f;
					float s = 0.25f / q.v.X();
					
					q.v.Y() = (a[1][0] + a[0][1]) * s;
					q.v.Z() = (a[0][2] + a[2][0]) * s;
					q.w = (a[2][1] - a[1][2]) * s;
				}
				else if (a[1][1] > a[2][2])
				{
					q.v.Y() = std::sqrtf(a[1][1] - a[0][0] - a[2][2] + 1.0f) * 0.5f;
					float s = 0.25f / q.v.Y();
					
					q.v.X() = (a[1][0] + a[0][1]) * s;
					q.v.Z() = (a[2][1] + a[1][2]) * s;
					q.w = (a[0][2] - a[2][0]) * s;
				}
				else
				{
					q.v.Z() = std::sqrtf(a[2][2] - a[0][0] - a[1][1] + 1.0f) * 0.5f;
					float s = 0.25f / q.v.Z();

					q.v.X() = (a[0][2] + a[2][0]) * s;
					q.v.Y() = (a[2][1] + a[1][2]) * s;
					q.w = (a[1][0] - a[0][1]) * s;
				}

				return q;
			}

			static Quat<T> slerp(Quat<T>& a, Quat<T>& b, float t)
			{
				Quat<T> r;
				float t_ = 1 - t;
				float Wa, Wb;
				float theta = std::acos(a.v.X() * b.v.X() + a.v.Y() * b.v.Y() + a.v.Z() * b.v.Z() + a.w * b.w);
				float sn = std::sin(theta);
				Wa = std::sin(t_ * theta) / sn;
				Wb = std::sin(t * theta) / sn;
				r.v.X() = Wa * a.v.X() + Wb * b.v.X();
				r.v.Y() = Wa * a.v.Y() + Wb * b.v.Y();
				r.v.Z() = Wa * a.v.Z() + Wb * b.v.Z();
				r.w = Wa * a.w + Wb * b.w;
				r.normalize();
				return a;
			}

			Vec3<T> toEuler()
			{
				
			}
	};

	//Quaternion RotateAboutAxis(Point3D pt, double angle, Point3D axis)
	//{
	//	Quaternion q, p, qinv;

	//	q.w = cos(0.5 * angle);
	//	q.u.x = sin(0.5 * angle) * axis.x;
	//	q.u.y = sin(0.5 * angle) * axis.y;
	//	q.u.z = sin(0.5 * angle) * axis.z;

	//	p.w = 0;
	//	p.u = pt;

	//	qinv = q;
	//	qinv.Inverse();

	//	q.Multiply(p);
	//	q.Multiply(qinv);

	//	return q;
	//}


	typedef Quat<float> quat;
}