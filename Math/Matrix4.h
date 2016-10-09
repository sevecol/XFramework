
#pragma once
#include "MathComm.h"

namespace Math
{
	// 按照左乘，同DX
	class XMatrix4
	{
	public :
		union
		{
			struct
			{
				float	_00, _01, _02, _03;
				float	_10, _11, _12, _13;
				float	_20, _21, _22, _23;
				float	_30, _31, _32, _33;
			};
			float _m[4][4];
		};

		// 构造
		XMatrix4()
			: _00(1.0f), _01(0.0f), _02(0.0f), _03(0.0f)
			, _10(0.0f), _11(1.0f), _12(0.0f), _13(0.0f)
			, _20(0.0f), _21(0.0f), _22(1.0f), _23(0.0f)
			, _30(0.0f), _31(0.0f), _32(0.0f), _33(1.0f) {};

		XMatrix4( float f00, float f01, float f02, float f03,
				    float f10, float f11, float f12, float f13,
					float f20, float f21, float f22, float f23,
					float f30, float f31, float f32, float f33 )
			: _00(f00), _01(f01), _02(f02), _03(f03)
			, _10(f10), _11(f11), _12(f12), _13(f13)
			, _20(f20), _21(f21), _22(f22), _23(f23)
			, _30(f30), _31(f31), _32(f32), _33(f33) {};

		XMatrix4( float f[16] )
		{
			memcpy( (void*)(&_m[0][0]), (void*)f, sizeof(XMatrix4) );
		}

		XMatrix4( const XMatrix4& mat )
		{
			memcpy( (void*)(&_m[0][0]), (void*)(&mat._m[0][0]), sizeof(XMatrix4) );
		}

		// 运算符重载

		inline XMatrix4& operator = ( const XMatrix4& mat )
		{
			memcpy( (void*)(&_m[0][0]), (void*)(&mat._m[0][0]), sizeof(XMatrix4) );
			return *this;
		}

		inline XMatrix4 operator + ( const XMatrix4& mat ) const
		{
			XMatrix4 mAdd;
			for( int i = 0; i < 16; i++ )
				*(&mAdd._m[0][0] + i) = *(&_m[0][0] + i) + *(&mat._m[0][0] + i);

			return mAdd;
		}

		inline XMatrix4 operator - ( const XMatrix4& mat ) const
		{
			XMatrix4 mSub;
			for( int i = 0; i < 16; i++ )
				*(&mSub._m[0][0] + i) = *(&_m[0][0] + i) - *(&mat._m[0][0] + i);

			return mSub;
		}

		inline XMatrix4 operator * ( float t ) const
		{
			XMatrix4 mat;
			for( int i = 0; i < 16; i++ )
				*(&mat._m[0][0] + i) = *(&_m[0][0] + i) * t;

			return mat;
		}

		inline XMatrix4 operator / ( float t ) const
		{
			assert( !IsZero(t) );

			XMatrix4 mat;
			for( int i = 0; i < 16; i++ )
				*(&mat._m[0][0] + i) = *(&_m[0][0] + i) / t;

			return mat;
		}
			
		inline XMatrix4& operator *= ( const XMatrix4& mat )
		{
			XMatrix4 mTmp(*this);

			_00 = mTmp._00*mat._00 + mTmp._01*mat._10 + mTmp._02*mat._20 + mTmp._03*mat._30;
			_01 = mTmp._00*mat._01 + mTmp._01*mat._11 + mTmp._02*mat._21 + mTmp._03*mat._31;
			_02 = mTmp._00*mat._02 + mTmp._01*mat._12 + mTmp._02*mat._22 + mTmp._03*mat._32;
			_03 = mTmp._00*mat._03 + mTmp._01*mat._13 + mTmp._02*mat._23 + mTmp._03*mat._33;

			_10 = mTmp._10*mat._00 + mTmp._11*mat._10 + mTmp._12*mat._20 + mTmp._13*mat._30;
			_11 = mTmp._10*mat._01 + mTmp._11*mat._11 + mTmp._12*mat._21 + mTmp._13*mat._31;
			_12 = mTmp._10*mat._02 + mTmp._11*mat._12 + mTmp._12*mat._22 + mTmp._13*mat._32;
			_13 = mTmp._10*mat._03 + mTmp._11*mat._13 + mTmp._12*mat._23 + mTmp._13*mat._33;

			_20 = mTmp._20*mat._00 + mTmp._21*mat._10 + mTmp._22*mat._20 + mTmp._23*mat._30;
			_21 = mTmp._20*mat._01 + mTmp._21*mat._11 + mTmp._22*mat._21 + mTmp._23*mat._31;
			_22 = mTmp._20*mat._02 + mTmp._21*mat._12 + mTmp._22*mat._22 + mTmp._23*mat._32;
			_23 = mTmp._20*mat._03 + mTmp._21*mat._13 + mTmp._22*mat._23 + mTmp._23*mat._33;

			_30 = mTmp._30*mat._00 + mTmp._31*mat._10 + mTmp._32*mat._20 + mTmp._33*mat._30;
			_31 = mTmp._30*mat._01 + mTmp._31*mat._11 + mTmp._32*mat._21 + mTmp._33*mat._31;
			_32 = mTmp._30*mat._02 + mTmp._31*mat._12 + mTmp._32*mat._22 + mTmp._33*mat._32;
			_33 = mTmp._30*mat._03 + mTmp._31*mat._13 + mTmp._32*mat._23 + mTmp._33*mat._33;

			return *this;
		}

		inline XMatrix4& operator += ( const XMatrix4& mat )
		{
			for( int i = 0; i < 16; i++ )
				*(&_m[0][0] + i) += *(&mat._m[0][0] + i);

			return *this;
		}

		inline XMatrix4& operator -= ( const XMatrix4& mat )
		{
			for( int i = 0; i < 16; i++ )
				*(&_m[0][0] + i) -= *(&mat._m[0][0] + i);

			return *this;
		}

		inline XMatrix4& operator *= ( float t )
		{
			for( int i = 0; i < 16; i++ )
				*(&_m[0][0] + i) *= t;

			return *this;
		}

		inline XMatrix4& operator /= ( float t )
		{
			assert( !IsZero(t) );

			for( int i = 0; i < 16; i++ )
				*(&_m[0][0] + i) /= t;

			return *this;
		}


		// 数学运算 

		inline XMatrix4 operator * ( const XMatrix4& mat ) const
		{
			XMatrix4 mMult;

			mMult._00 = _00*mat._00 + _01*mat._10 + _02*mat._20 + _03*mat._30;
			mMult._01 = _00*mat._01 + _01*mat._11 + _02*mat._21 + _03*mat._31;
			mMult._02 = _00*mat._02 + _01*mat._12 + _02*mat._22 + _03*mat._32;
			mMult._03 = _00*mat._03 + _01*mat._13 + _02*mat._23 + _03*mat._33;

			mMult._10 = _10*mat._00 + _11*mat._10 + _12*mat._20 + _13*mat._30;
			mMult._11 = _10*mat._01 + _11*mat._11 + _12*mat._21 + _13*mat._31;
			mMult._12 = _10*mat._02 + _11*mat._12 + _12*mat._22 + _13*mat._32;
			mMult._13 = _10*mat._03 + _11*mat._13 + _12*mat._23 + _13*mat._33;

			mMult._20 = _20*mat._00 + _21*mat._10 + _22*mat._20 + _23*mat._30;
			mMult._21 = _20*mat._01 + _21*mat._11 + _22*mat._21 + _23*mat._31;
			mMult._22 = _20*mat._02 + _21*mat._12 + _22*mat._22 + _23*mat._32;
			mMult._23 = _20*mat._03 + _21*mat._13 + _22*mat._23 + _23*mat._33;

			mMult._30 = _30*mat._00 + _31*mat._10 + _32*mat._20 + _33*mat._30;
			mMult._31 = _30*mat._01 + _31*mat._11 + _32*mat._21 + _33*mat._31;
			mMult._32 = _30*mat._02 + _31*mat._12 + _32*mat._22 + _33*mat._32;
			mMult._33 = _30*mat._03 + _31*mat._13 + _32*mat._23 + _33*mat._33;

			return mMult;
		}

		inline  void Identify()
		{
			memset( (void*)(&_m[0][0]), 0, sizeof(XMatrix4) );

			_00 = _11 = _22 = _33 = 1.0f;
		}


		// 矩阵转置
		inline XMatrix4 Transpose() const
		{
			XMatrix4 mat;
			for (int r = 0 ; r < 4 ; r++)
				for (int c = 0 ; c < 4 ; c++)
					mat._m[c][r] = (*this)._m[r][c] ;

			return mat;
		}


		// 求逆矩阵
		// 只有在矩阵的第四列为[0 0 0 1] 这个函数才能正常工作，自身改变
		// return bool true 求得逆矩阵， false 求不出逆矩阵
		inline bool Invert()
		{
			XMatrix4 InMat( *this );

			//Verify the matrices with [0 0 0 1] for the 4th column.
			if( !IsZero(_33- 1.0f) || !IsZero(_03 ) || !IsZero( _13 ) || !IsZero(_23 ) )
				return false;

			float fDetInv = 1.0f / ( InMat._00 * ( InMat._11 * InMat._22 - InMat._12 * InMat._21 ) -
				InMat._01 * ( InMat._10 * InMat._22 - InMat._12 * InMat._20 ) +
				InMat._02 * ( InMat._10 * InMat._21 - InMat._11 * InMat._20 ) );

			_00 =  fDetInv * ( InMat._11 * InMat._22 - InMat._12 * InMat._21 );
			_01 = -fDetInv * ( InMat._01 * InMat._22 - InMat._02 * InMat._21 );
			_02 =  fDetInv * ( InMat._01 * InMat._12 - InMat._02 * InMat._11 );
			_03 =  0.0f;

			_10 = -fDetInv * ( InMat._10 * InMat._22 - InMat._12 * InMat._20 );
			_11 =  fDetInv * ( InMat._00 * InMat._22 - InMat._02 * InMat._20 );
			_12 = -fDetInv * ( InMat._00 * InMat._12 - InMat._02 * InMat._10 );
			_13 =  0.0f;

			_20 =  fDetInv * ( InMat._10 * InMat._21 - InMat._11 * InMat._20 );
			_21 = -fDetInv * ( InMat._00 * InMat._21 - InMat._01 * InMat._20 );
			_22 =  fDetInv * ( InMat._00 * InMat._11 - InMat._01 * InMat._10 );
			_23 = 0.0f;

			_30 = -( InMat._30 * _00 + InMat._31 * _10 + InMat._32 * _20 );
			_31 = -( InMat._30 * _01 + InMat._31 * _11 + InMat._32 * _21 );
			_32 = -( InMat._30 * _02 + InMat._31 * _12 + InMat._32 * _22 );
			_33 = 1.0f;

			return true;
		}

		//* 行列式
		inline float Determinant() const 
		{
			float const _2031_2130( _20 * _31 - _21 * _30 );
			float const _2032_2230( _20 * _32 - _22 * _30 );
			float const _2033_2330( _20 * _33 - _23 * _30 );
			float const _2132_2231( _21 * _32 - _22 * _31 );
			float const _2133_2331( _21 * _33 - _23 * _31 );
			float const _2233_2332( _22 * _33 - _23 * _32 );

			return _00 * ( _11 * _2233_2332 - _12 * _2133_2331 + _13 * _2132_2231)
					- _01 * ( _10 * _2233_2332 - _12 * _2033_2330 + _13 * _2032_2230)
					+ _02 * ( _10 * _2133_2331 - _11 * _2033_2330 + _13 * _2031_2130)
					- _03 * ( _10 * _2132_2231 - _11 * _2032_2230 + _12 * _2031_2130);
		}

		// 友元函数
		inline friend XMatrix4 operator* ( float t, const XMatrix4& m )
		{
			XMatrix4 mat;
			for( int i = 0; i < 16; i++ )
				*(&mat._m[0][0] + i) = *(&m._m[0][0] + i) * t;

			return mat;
		}
	};

	typedef XMatrix4 Matrix4;
}
using namespace Math;