
#pragma once
#include "MathComm.h"

namespace Math
{
	template< typename T >
	struct XVector2
	{
		union 
		{
			struct 
			{
				T	x,y; // 2 components of the vector
			};

			T	_v[2];    // Array access useful in loops
		};

		// 构造

		XVector2() : x( (T)0 ), y( (T)0 ) {};
		XVector2( T tx, T ty ) : x(tx), y(ty) {};
		XVector2( T t[2] ) : x( t[0] ), y( t[1] ) {};
		XVector2( const XVector2<T>& v ) : x(v.x), y(v.y) {};

		// 设置初值
		inline void SetValue( T tx, T ty )
		{
			x = tx;
			y = ty;
		}

		inline void Zero()
		{
			memset( (void*)(&_v[0]), 0, 2*sizeof(T) );
		}

		// 运算符重载

		inline T operator [] ( size_t i ) const
		{
			assert( i < 2 );
			return *(&x+i);
		}

		inline T& operator [] ( size_t i )
		{
			assert( i < 2 );
			return *(&x+i);
		}

			
		inline XVector2<T>& operator = ( const XVector2<T>& v )
		{
			x = v.x;
			y = v.y;        

			return *this;
		}

		inline bool operator == ( const XVector2<T>& v ) const
		{
			return ( IsZero( x - v.x ) && IsZero( y - v.y ) );
		}

		inline bool operator != ( const XVector2<T>& v ) const
		{
			return ( !IsZero( x - v.x ) || !IsZero( y - v.y ) );
		}

		inline XVector2<T> operator + ( const XVector2<T>& v ) const
		{
			XVector2<T> vSum;

			vSum.x = x + v.x;
			vSum.y = y + v.y;

			return vSum;
		}

		inline XVector2<T> operator - ( const XVector2<T>& v ) const
		{
			XVector2<T> vDiff;

			vDiff.x = x - v.x;
			vDiff.y = y - v.y;

			return vDiff;
		}

		inline XVector2<T> operator * ( T tScalar ) const
		{
			XVector2<T> vProd;

			vProd.x = tScalar*x;
			vProd.y = tScalar*y;

			return vProd;
		}

		inline XVector2<T> operator / ( T tScalar ) const
		{
			assert( !IsZero(tScalar) );

			XVector2<T> vDiv;

			T tInv = 1.0 / tScalar;
			vDiv.x = x * tInv;
			vDiv.y = y * tInv;

			return vDiv;
		}

		inline XVector2<T> operator - () const
		{
			XVector2<T> vNeg;

			vNeg.x = -x;
			vNeg.y = -y;

			return vNeg;
		}

		inline XVector2<T>& operator += ( const XVector2<T>& v )
		{
			x += v.x;
			y += v.y;

			return *this;
		}

		inline XVector2<T>& operator -= ( const XVector2<T>& v )
		{
			x -= v.x;
			y -= v.y;

			return *this;
		}

		inline XVector2<T>& operator *= ( T tScalar )
		{
			x *= tScalar;
			y *= tScalar;

			return *this;
		}

		inline XVector2<T>& operator /= ( T tScalar )
		{
			assert( !IsZero(tScalar) );

			T tInv = 1.0 / tScalar;

			x *= tInv;
			y *= tInv;

			return *this;
		}


		// 数学运算

		inline float Length() const
		{
			return std::sqrt(x*x + y*y);
		}

		// 两点之间距离
		inline float Distance( const XVector2<T>& pos ) const
		{
			XVector2<T> v( x-pos.x, y-pos.y );
			return v.Length();
		}

		// 单位向量
		inline XVector2<T>& Normal() const
		{
			float len = Length();

			if( !IsZero(len) )
                return *this/len;
			else
				return *this;
		}

		// 单位化
		inline void Normalize()
		{
			float len = Length();
				
			if( !IsZero(len) )
			{
				float fReci = 1.0f / len;
				x *= fReci;
				y *= fReci;
			}
		}

		// 点乘
		inline T Dot( const XVector2<T>& v ) const
		{
			return ( x*v.x + y*v.y ); 
		} 

		// 叉乘
        //+[12/9/2006-17:36 Lyan]
        //inline XVector2<T> Cross( const XVector2<T>& v ) const
		//{
		//	return XVector2(-v.y, v.x);
		//}
        inline T Cross( const XVector2<T>& v ) const
        {
            return x * v.y - y * v.x;
        }
        //-[12/9/2006-17:36 Lyan]

		// 两向量夹角(弧度)
		inline float CalAngle( const XVector2<T>& v ) const
		{
			float len = Length();
			if( IsZero(len) )
				return HALFPI;
			else
                return std::acos( Dot(v) / (len*v.Length()) );
		}

		// 友元函数
		inline friend XVector2<T> operator * ( T tScalar, const XVector2<T>& v )
		{
			return v*tScalar;
		}

		// casting
		inline operator T* ()
		{
			return &_v[0];
		}
	};

	typedef XVector2<float> Vector2f;
	typedef XVector2<int>   Pos2;
}
using namespace Math;