#pragma once

#include "Vec2/Vec2.h"

namespace lm
{
	template <typename T> class Mat2
	{
	private:
		Vec2<T> matrix[2];

	public:
		static const Mat2<T> identity;

		Mat2()
		{
			this->matrix[0] = Vec2<T>(1, 0);
			this->matrix[1] = Vec2<T>(0, 1);
		}

		Mat2(const T init)
		{
			this->matrix[0] = Vec2<T>(init, 0);
			this->matrix[1] = Vec2<T>(0, init);
		}

		Mat2(const T x, const T y, const T z, const T w)
		{
			this->matrix[0] = Vec2<T>(x, y);
			this->matrix[1] = Vec2<T>(z, w);
		}

		Mat2(const Vec2<T>& vec1, const Vec2<T>& vec2)
		{
			this->matrix[0] = vec1;
			this->matrix[1] = vec2;
		}

		Mat2(const Mat2<T>& mat2)
		{
			this->matrix[0] = mat2.matrix[0];
			this->matrix[1] = mat2.matrix[1];
		}

		Mat2(Mat2<T>&& mat2)
		{
			this->matrix[0] = std::move(mat2.matrix[0]);
			this->matrix[1] = std::move(mat2.matrix[1]);
		}

		Mat2<T>& operator=(const Mat2<T>& mat2)
		{
			if (this == &mat2)
				return *this;

			this->matrix[0] = mat2.matrix[0];
			this->matrix[1] = mat2.matrix[1];

			return *this;
		}

		Mat2<T>& operator=(Mat2<T>&& mat2) noexcept
		{
			if (this == &mat2)
				return *this;

			this->matrix[0] = std::move(mat2.matrix[0]);
			this->matrix[1] = std::move(mat2.matrix[1]);

			return *this;
		}

		const T operator[](int idx) const
		{
			unsigned int vecIdx = idx / 2;
			idx = idx % 2;

			switch (idx)
			{
				case 0:	return this->matrix[vecIdx].X();
				case 1:	return this->matrix[vecIdx].Y();
				
				default: return this->matrix[0].X();
			}
		}

		T& operator[](int idx)
		{
			unsigned int vecIdx = idx / 2;
			idx = idx % 2;

			switch (idx)
			{
				case 0:	return this->matrix[vecIdx].X();
				case 1:	return this->matrix[vecIdx].Y();

				default: this->matrix[0].X();
			}
		}

		const T operator[](const char* idx) const
		{
			unsigned int vecIdx = idx[1] - '1';
			switch (idx[0])
			{
				case 'x': return this->matrix[vecIdx].X();
				case 'y': return this->matrix[vecIdx].Y();

				default: return this->matrix[0].X();
			}
		}

		T& operator[](const char* idx)
		{
			unsigned int vecIdx = idx[1] - '1';
			switch (idx[0])
			{
				case 'x': return this->matrix[vecIdx].X();
				case 'y': return this->matrix[vecIdx].Y();

				default: return this->matrix[0].X();
			}
		}

		Mat2<T> scale(const float scale) const
		{
			Mat2<T> mat2(*this);
			for (unsigned int i = 0; i < 2; i++)
			{
				mat2.matrix[i].X() *= scale;
				mat2.matrix[i].Y() *= scale;
			}

			return mat2;
		}

		Mat2<T>& scale(const float scale)
		{
			for (unsigned int i = 0; i < 2; i++)
			{
				this.matrix[i].X() *= scale;
				this.matrix[i].Y() *= scale;
			}

			return *this;
		}

		Mat2<T> dot_product(const Mat2<T>& mat2) const
		{
			return Mat2<T>(((this->matrix[0].X() * mat2.matrix[0].X()) + (this->matrix[0].Y() * mat2.matrix[1].X())),
						  ((this->matrix[0].X() * mat2.matrix[0].Y()) + (this->matrix[0].Y() * mat2.matrix[1].Y())),
						  ((this->matrix[1].X() * mat2.matrix[0].X()) + (this->matrix[1].Y() * mat2.matrix[1].X())),
						  ((this->matrix[1].X() * mat2.matrix[0].Y()) + (this->matrix[1].Y() * mat2.matrix[1].Y())));
		}

		Mat2<T> transpose() const
		{
			return Mat2<T>(this->matrix[0].X(), this->matrix[1].X(), this->matrix[0].Y(), this->matrix[1].Y());
		}

		const T determinant() const
		{
			return (this->matrix[0].X() * this->matrix[1].Y()) - (this->matrix[0].Y() * this->matrix[1].X());
		}

		Mat2<T> minor() const
		{
			return Mat2<T>(this->matrix[1].Y(), this->matrix[1].X(), this->matrix[0].Y(), this->matrix[0].X());
		}

		Mat2<T> cofactor() const
		{
			Mat2<T> min = this->minor();
			min.matrix[0].Y() *= -1;
			min.matrix[1].X() *= -1;
			return min;
		}

		Mat2<T> adjugate() const
		{
			Mat2<T> cof = this->cofactor();
			cof = cof.transpose();
			return cof;
		}

		Mat2<T> inverse() const
		{
			const T determinant = this->determinant();
			const T div = 1.0f / determinant;

			Mat2<T> mat2 = this->adjugate();
			return Mat2<T>(mat2.matrix[0].X() * div, mat2.matrix[0].Y() * div, mat2.matrix[1].X() * div, mat2.matrix[1].Y() * div);
		}

		Mat2<T> operator*(const float scale) const
		{
			return this->scale(scale);
		}

		void operator*=(const float scale)
		{
			*this = this->scale(scale);
		}

		Mat2<T> operator*(const Mat2<T>& mat2) const
		{
			return this->dot_product(mat2);
		}

		const bool operator==(const Mat2<T>& mat2) const
		{
			if (this == &mat2)
				return true;

			if (this->matrix[0] != mat2.matrix[0] || this->matrix[1] != mat2.matrix[1])
				return false;

			return true;
		}

		const bool operator!=(const Mat2<T>& mat2) const
		{
			return !(*this == mat2);
		}
	};

	template<class T> const Mat2<T> Mat2<T>::identity = Mat2<T>();

	typedef Mat2<float> mat2;
}