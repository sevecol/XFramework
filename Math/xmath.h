
#pragma once

#include "MathComm.h"
#include "Rectangle.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4.h"
#include "Quaternion.h"
#include "Color4f.h"
#include "Color4.h"
#include "Plane.h"
#include "Ray.h"
#include "BoundingVolume.h"
#include "AxisAlignedBoundingBox.h"
#include "OptPlane.h"
#include "OptFrustum.h"

namespace Math
{
	//* 向量乘矩阵
	inline Vector3f VectorMultiMatrix( const Vector3f& vIn, const Matrix4& mat )
	{
		Vector3f vOut;

		vOut.x =  vIn.x * mat._00 + vIn.y * mat._10 + vIn.z * mat._20 + mat._30;
		vOut.y =  vIn.x * mat._01 + vIn.y * mat._11 + vIn.z * mat._21 + mat._31;
		vOut.z =  vIn.x * mat._02 + vIn.y * mat._12 + vIn.z * mat._22 + mat._32;

		float w = vIn.x * mat._03 + vIn.y * mat._13 + vIn.z * mat._23 + mat._33;
		if( !IsZero(w - 1.0f) )
			vOut /= w;

		return vOut;
	}

	//* 向量乘矩阵
	inline Vector4f VectorMultiMatrix( const Vector4f& vIn, const Matrix4& mat )
	{
		Vector4f vOut;

		vOut.x =  vIn.x * mat._00 + vIn.y * mat._10 + vIn.z * mat._20 + vIn.w*mat._30;
		vOut.y =  vIn.x * mat._01 + vIn.y * mat._11 + vIn.z * mat._21 + vIn.w*mat._31;
		vOut.z =  vIn.x * mat._02 + vIn.y * mat._12 + vIn.z * mat._22 + vIn.w*mat._32;
		vOut.w =  vIn.x * mat._03 + vIn.y * mat._13 + vIn.z * mat._23 + vIn.w*mat._33;

		return vOut;
	}

	//* 转置矩阵
	inline Matrix4 Transpose( const Matrix4& m )
	{
		Matrix4 mat;

		for (int r = 0 ; r < 4 ; r++)
			for (int c = 0 ; c < 4 ; c++)
				mat._m[c][r] = m._m[r][c] ;

		return mat;
	}

	//* 平移矩阵
	inline Matrix4 Matrix4Translate( float x, float y, float z )
	{
		Matrix4 mat;
		mat.Identify();

		mat._30 = x;
		mat._31 = y;
		mat._32 = z;

		return mat;
	}

	//* 绕X轴旋转
	inline Matrix4 Matrix4RotateX( float fRadian )
	{
		Matrix4 mat;
		mat.Identify();

		float c = std::cos( fRadian );
		float s = std::sin( fRadian );

		mat._11 =  c;
		mat._12 =  s;
		mat._21 = -s;
		mat._22 =  c;

		return mat;
	}

	//* 绕Y轴旋转
	inline Matrix4 Matrix4RotateY( float fRadian )
	{
		Matrix4 mat;
		mat.Identify();

		float c = std::cos( fRadian );
		float s = std::sin( fRadian );

		mat._00 =  c;
		mat._02 = -s;
		mat._20 =  s;
		mat._22 =  c;

		return mat;
	}

	//* 绕Z轴旋转
	inline Matrix4 Matrix4RotateZ( float fRadian )
	{
		Matrix4 mat;
		mat.Identify();

		float c = std::cos( fRadian );
		float s = std::sin( fRadian );

		mat._00 =  c;
		mat._01 =  s;
		mat._10 = -s;
		mat._11 =  c;

		return mat;
	}

	//* 绕轴旋转角度
	inline Matrix4 Matrix4RotateAxisAngle( const Vector3f& vAxis, float fRadian )
	{
		Matrix4 mat;
		mat.Identify();

		float		fCos	= std::cos( fRadian );
		float		fSin	= std::sin( fRadian );

		Vector3f	v		= vAxis;
		v.Normalize();

		mat._00 = ( v.x * v.x ) * ( 1.0f - fCos ) + fCos;
		mat._01 = ( v.x * v.y ) * ( 1.0f - fCos ) + (v.z * fSin);
		mat._02 = ( v.x * v.z ) * ( 1.0f - fCos ) - (v.y * fSin);

		mat._10 = ( v.y * v.x ) * ( 1.0f - fCos ) - (v.z * fSin);
		mat._11 = ( v.y * v.y ) * ( 1.0f - fCos ) + fCos ;
		mat._12 = ( v.y * v.z ) * ( 1.0f - fCos ) + (v.x * fSin);

		mat._20 = ( v.z * v.x ) * ( 1.0f - fCos ) + (v.y * fSin);
		mat._21 = ( v.z * v.y ) * ( 1.0f - fCos ) - (v.x * fSin);
		mat._22 = ( v.z * v.z ) * ( 1.0f - fCos ) + fCos;

		return mat;
	}

	//* scaleMatrix
	inline Matrix4 Matrix4Scale( float sx, float sy, float sz )
	{
		return Matrix4( sx,   0.0f, 0.0f,  0.0f ,
				        0.0f, sy,   0.0f,  0.0f,
						0.0f, 0.0f, sz,    0.0f,
						0.0f, 0.0f, 0.0f,  1.0f );
	}
	//* view
	inline Matrix4 Matrix4ViewLH( const Vector3f& vPos, const Vector3f& vLookPos, const Vector3f& vUp )
	{
		Vector3f zAxis( (vLookPos - vPos).Normal() );
		Vector3f xAxis( vUp.Cross(zAxis).Normal() );
		Vector3f yAxis( (zAxis.Cross(xAxis)).Normal() );

		Matrix4 mat;
		mat.Identify();

		mat._00 = xAxis.x;           mat._01 = yAxis.x;           mat._02 = zAxis.x;
		mat._10 = xAxis.y;           mat._11 = yAxis.y;           mat._12 = zAxis.y;
		mat._20 = xAxis.z;           mat._21 = yAxis.z;           mat._22 = zAxis.z;
		mat._30 = -xAxis.Dot(vPos);  mat._31 = -yAxis.Dot(vPos);  mat._32 = -zAxis.Dot(vPos);

		return mat;
	}

	//* Perspective project
	inline Matrix4 Matrix4PerspectiveFovLH( float fFovRadian, float fAspect, float fNear, float fFar )
	{
		float h = 1.0f / std::tan(fFovRadian / 2);
		float w = h / fAspect;
		float q = fFar / (fFar - fNear);

		return Matrix4(
			w,		0,		0,				0,
			0,		h,		0,				0,
			0,		0,		q,				1,
			0,		0,	   -fNear * q,      0 );
	}

	inline Matrix4 Matrix4PerspectiveLH( float fWidth, float fHeight, float fNear, float fFar )
	{
		float q = fFar / (fFar - fNear);

		return Matrix4(
			2*fNear/fWidth,	0,					0,				0,
			0,				2*fNear/fHeight,	0,				0,
			0,				0,					q,				1,
			0,				0,					-fNear * q,     0 );
	}

	//* Matrix inverse
	inline Matrix4 Matrix4Inverse( const Matrix4& In )
	{
		float const _2132_2231( In._10 * In._21 - In._11 * In._20 );
		float const _2133_2331( In._10 * In._22 - In._12 * In._20 );
		float const _2134_2431( In._10 * In._23 - In._13 * In._20 );
		float const _2142_2241( In._10 * In._31 - In._11 * In._30 );
		float const _2143_2341( In._10 * In._32 - In._12 * In._30 );
		float const _2144_2441( In._10 * In._33 - In._13 * In._30 );
		float const _2233_2332( In._11 * In._22 - In._12 * In._21 );
		float const _2234_2432( In._11 * In._23 - In._13 * In._21 );
		float const _2243_2342( In._11 * In._32 - In._12 * In._31 );
		float const _2244_2442( In._11 * In._33 - In._13 * In._31 );
		float const _2334_2433( In._12 * In._23 - In._13 * In._22 );
		float const _2344_2443( In._12 * In._33 - In._13 * In._32 );
		float const _3142_3241( In._20 * In._31 - In._21 * In._30 );
		float const _3143_3341( In._20 * In._32 - In._22 * In._30 );
		float const _3144_3441( In._20 * In._33 - In._23 * In._30 );
		float const _3243_3342( In._21 * In._32 - In._22 * In._31 );
		float const _3244_3442( In._21 * In._33 - In._23 * In._31 );
		float const _3344_3443( In._22 * In._33 - In._23 * In._32 );

		// 行列式的值
		float const  det = In.Determinant();
		if ( !IsZero(det) )
		{
			float invDet( 1.0f / det );

			return Matrix4(
				+invDet * ( In._11 * _3344_3443 - In._12 * _3244_3442 + In._13 * _3243_3342 ),
				-invDet * ( In._01 * _3344_3443 - In._02 * _3244_3442 + In._03 * _3243_3342 ),
				+invDet * ( In._01 * _2344_2443 - In._02 * _2244_2442 + In._03 * _2243_2342 ),
				-invDet * ( In._01 * _2334_2433 - In._02 * _2234_2432 + In._03 * _2233_2332 ),

				-invDet * ( In._10 * _3344_3443 - In._12 * _3144_3441 + In._13 * _3143_3341 ),
				+invDet * ( In._00 * _3344_3443 - In._02 * _3144_3441 + In._03 * _3143_3341 ),
				-invDet * ( In._00 * _2344_2443 - In._02 * _2144_2441 + In._03 * _2143_2341 ),
				+invDet * ( In._00 * _2334_2433 - In._02 * _2134_2431 + In._03 * _2133_2331 ),

				+invDet * ( In._10 * _3244_3442 - In._11 * _3144_3441 + In._13 * _3142_3241 ),
				-invDet * ( In._00 * _3244_3442 - In._01 * _3144_3441 + In._03 * _3142_3241 ),
				+invDet * ( In._00 * _2244_2442 - In._01 * _2144_2441 + In._03 * _2142_2241 ),
				-invDet * ( In._00 * _2234_2432 - In._01 * _2134_2431 + In._03 * _2132_2231 ),

				-invDet * ( In._10 * _3243_3342 - In._11 * _3143_3341 + In._12 * _3142_3241 ),
				+invDet * ( In._00 * _3243_3342 - In._01 * _3143_3341 + In._02 * _3142_3241 ),
				-invDet * ( In._00 * _2243_2342 - In._01 * _2143_2341 + In._02 * _2142_2241 ),
				+invDet * ( In._00 * _2233_2332 - In._01 * _2133_2331 + In._02 * _2132_2231 ) );
		}
		else
		{
			return In;
		}
	}


	//* ortho project
	inline Matrix4 Matrix4OrthoLH( float w, float h, float fNear, float fFar )
	{
		return Matrix4(
			2.0f/w,  0,       0,                   0,
			0,       2.0f/h,  0,                   0,
			0,       0,       1.0f/(fFar-fNear),   0,
			0,       0,       fNear/(fNear-fFar),  1.0f );
	}
			

    // 面的Transform
    inline void PlaneTransfrom(Vector4f& planeout, const Vector4f& planein, const Matrix4& mat)
    {
        Matrix4 matInvTranspose = Matrix4Inverse(mat);
        matInvTranspose = matInvTranspose.Transpose();

        planeout = VectorMultiMatrix( planein, matInvTranspose);
    }

    // 平面反射矩阵
    inline void Matrix4Reflect( Matrix4& outMat, const Plane& plane )
    {
        Plane P(plane);

        P.Normalize();

        outMat = Matrix4(-2 * P._vNormal.x * P._vNormal.x + 1,  -2 * P._vNormal.y * P._vNormal.x,      -2 * P._vNormal.z * P._vNormal.x,        0,
                            -2 * P._vNormal.x * P._vNormal.y,      -2 * P._vNormal.y * P._vNormal.y + 1,  -2 * P._vNormal.z * P._vNormal.y,        0,
                            -2 * P._vNormal.x * P._vNormal.z,      -2 * P._vNormal.y * P._vNormal.z,      -2 * P._vNormal.z * P._vNormal.z + 1,    0,
                            -2 * P._vNormal.x * P._d,              -2 * P._vNormal.y * P._d,              -2 * P._vNormal.z * P._d,                1 );


    }
		

	//* Matrix4 to quaternion
	inline void Matrix4ToQuaternion( Quaternion& q, const Matrix4& m )
	{
		// Determine which of s, x, y, or z has the largest absolute value
		float fourSSquaredMinus1 = m._00 + m._11 + m._22;
		float fourXSquaredMinus1 = m._00 - m._11 - m._22;
		float fourYSquaredMinus1 = m._11 - m._00 - m._22;
		float fourZSquaredMinus1 = m._22 - m._00 - m._11;

		int biggestIndex = 0;

		float fourBiggestSquaredMinus1 = fourSSquaredMinus1;
		if (fourXSquaredMinus1 > fourBiggestSquaredMinus1) 
		{
			fourBiggestSquaredMinus1 = fourXSquaredMinus1;
			biggestIndex = 1;
		}
		if (fourYSquaredMinus1 > fourBiggestSquaredMinus1) 
		{
			fourBiggestSquaredMinus1 = fourYSquaredMinus1;
			biggestIndex = 2;
		}
		if (fourZSquaredMinus1 > fourBiggestSquaredMinus1) 
		{
			fourBiggestSquaredMinus1 = fourZSquaredMinus1;
			biggestIndex = 3;
		}

		// Perform square root and division
		float biggestVal = std::sqrt(fourBiggestSquaredMinus1 + 1.0f) * 0.5f;
		float mult = 0.25f / biggestVal;

		// Apply table to compute quaternion values
		switch (biggestIndex) 
		{
		case 0:
			q.s = biggestVal;
			q.x = (m._12 - m._21) * mult;
			q.y = (m._20 - m._02) * mult;
			q.z = (m._01 - m._10) * mult;
			break;
		case 1:
			q.x = biggestVal;
			q.s = (m._12 - m._21) * mult;
			q.y = (m._01 + m._10) * mult;
			q.z = (m._20 + m._02) * mult;
			break;
		case 2:
			q.y = biggestVal;
			q.s = (m._20 - m._02) * mult;
			q.x = (m._01 + m._10) * mult;
			q.z = (m._12 + m._21) * mult;
			break;
		case 3:
			q.z = biggestVal;
			q.s = (m._01 - m._10) * mult;
			q.x = (m._20 + m._02) * mult;
			q.y = (m._12 + m._21) * mult;
			break;
		}
	}

	//* quaternion to Matrix
	inline void QuaternionToMatrix4( Matrix4& m, const Quaternion& q )
	{
		float twoxx = 2.0f * q.x * q.x;
		float twoyy = 2.0f * q.y * q.y;
		float twozz = 2.0f * q.z * q.z;
		float twoxy = 2.0f * q.x * q.y;
		float twoyz = 2.0f * q.y * q.z;
		float twoxz = 2.0f * q.x * q.z;
		float twosx = 2.0f * q.s * q.x;
		float twosy = 2.0f * q.s * q.y;
		float twosz = 2.0f * q.s * q.z;

		m.Identify();

		m._00 = 1.0f - twoyy - twozz; 	m._01 = twoxy + twosz; 	        m._02 = twoxz - twosy;
		m._10 = twoxy - twosz;			m._11 = 1.0f - twoxx - twozz;   m._12 = twoyz + twosx;
		m._20 = twoxz + twosy;          m._21 = twoyz - twosx;          m._22 = 1.0f - twoxx - twoyy;
	}


	//* quaternion to Matrix
	inline void QuaternionToMatrix4( Matrix4* m, const Quaternion* q ,int num)
	{
		for (int i=0;i<num;++i)
		{
			float twoxx = 2.0f * q[i].x * q[i].x;
			float twoyy = 2.0f * q[i].y * q[i].y;
			float twozz = 2.0f * q[i].z * q[i].z;
			float twoxy = 2.0f * q[i].x * q[i].y;
			float twoyz = 2.0f * q[i].y * q[i].z;
			float twoxz = 2.0f * q[i].x * q[i].z;
			float twosx = 2.0f * q[i].s * q[i].x;
			float twosy = 2.0f * q[i].s * q[i].y;
			float twosz = 2.0f * q[i].s * q[i].z;

			m[i].Identify();

			m[i]._00 = 1.0f - twoyy - twozz; 	m[i]._01 = twoxy + twosz; 	        m[i]._02 = twoxz - twosy;
			m[i]._10 = twoxy - twosz;			m[i]._11 = 1.0f - twoxx - twozz;   m[i]._12 = twoyz + twosx;
			m[i]._20 = twoxz + twosy;          m[i]._21 = twoyz - twosx;          m[i]._22 = 1.0f - twoxx - twoyy;
		}
	}

	//* Quaternion Slerp
	inline Quaternion QuaternionSlerp(const Quaternion &q0, const Quaternion &q1, float t)
	{
		// Check for out-of range parameter and return edge points if so
		if (t <= 0.0f) return q0;
		if (t >= 1.0f) return q1;
		// Compute "cosine of angle between quaternions" using dot product
		float cosOmega = q0.Dot(q1);
		// If negative dot, use –q1. Two quaternions q and –q
		// represent the same rotation, but may produce
		// different slerp. We chose q or –q to rotate using
		// the acute angle.
		float q1s = q1.s;
		float q1x = q1.x;
		float q1y = q1.y;
		float q1z = q1.z;
		if (cosOmega < 0.0f) 
		{
			q1s = -q1s;
			q1x = -q1x;
			q1y = -q1y;
			q1z = -q1z;
			cosOmega = -cosOmega;
		}
		// We should have two unit quaternions, so dot should be <= 1.0
		assert(cosOmega < 1.1f);
		// Compute interpolation fraction, checking for quaternions
		// almost exactly the same
		float k0, k1;
		if (cosOmega > 0.9999f) 
		{
			// Very close - just use linear interpolation,
			// which will protect againt a divide by zero
			k0 = 1.0f - t;
			k1 = t;
		} 
		else 
		{
			// Compute the sin of the angle using the
			// trig identity sin^2(omega) + cos^2(omega) = 1
			float sinOmega = std::sqrt(1.0f - cosOmega*cosOmega);
			// Compute the angle from its sin and cosine
			float omega = std::atan2(sinOmega, cosOmega);
			// Compute inverse of denominator, so we only have
			// to divide once
			float oneOverSinOmega = 1.0f / sinOmega;
			// Compute interpolation parameters
			k0 = sin((1.0f - t) * omega) * oneOverSinOmega;
			k1 = sin(t * omega) * oneOverSinOmega;
		}
		// Interpolate
		Quaternion result;
		result.x = k0*q0.x + k1*q1x;
		result.y = k0*q0.y + k1*q1y;
		result.z = k0*q0.z + k1*q1z;
		result.s = k0*q0.s + k1*q1s;
		// Return it
		return result;			
	}

	//====================================================================
	//* 判断一个点是否在三角形内
	//  v和三角形三个顶点之间的向量按照同一个叉乘，若叉乘结果朝 同一个方向
	//  则表示在里面，否则在外面
	//====================================================================
	inline bool IsPointIntriangle( const Vector3f& v, const Vector3f& v0, const Vector3f& v1, const Vector3f& v2 )
	{
		Vector3f d0 = v0 - v;
		Vector3f d1 = v1 - v;
		Vector3f d2 = v2 - v;

		Vector3f cross0 = d0.Cross(d1);
		Vector3f cross1 = d1.Cross(d2);
		Vector3f cross2 = d2.Cross(d0);

		cross0.Normalize();
		cross1.Normalize();
		cross2.Normalize();

		if( fabs(cross0.x - cross1.x) < 0.0001f && fabs(cross0.y - cross1.y) < 0.0001f && fabs(cross0.z - cross1.z) < 0.0001f &&
			fabs(cross2.x - cross1.x) < 0.0001f && fabs(cross2.y - cross1.y) < 0.0001f && fabs(cross2.z - cross1.z) < 0.0001f )
			return true;
		else
			return false;
	}


	//* 点和平面距离
	inline float LengthPointToPlane( const Vector3f& vPoint, const Plane& Plane )
	{
		// 要保证_vNormal是单位化的
		return Plane._vNormal.Dot(vPoint) + Plane._d;
	}

	//* 两个平面是否平行
	inline bool IsParallelOf2Plane( const Plane& P1, const Plane& P2 )
	{
		return IsZero( P1._vNormal.Dot( P2._vNormal ) - 1.0f );
	}

	//* 两个(平行)平面间距离
	inline float LengthBetween2Plane( const Plane& P1, const Plane& P2 )
	{
		if( !IsParallelOf2Plane(P1, P2) )
			return 0.0f;
		else
		{
			if( !IsZero(P1._vNormal.z) )
				return P2._d - P2._vNormal.z*P1._d/P1._vNormal.z;
			else if( !IsZero(P1._vNormal.y) )
				return P2._d - P2._vNormal.y*P1._d/P1._vNormal.y;
			else if( !IsZero(P1._vNormal.x) )
				return P2._d - P2._vNormal.x*P1._d/P1._vNormal.x;
			else
				return 0.0f;
		}
	}

	//* 判断一个点在一个平面的方位
	inline Plane::PlaneDirection CalRelationInPointPlane( const Vector3f& vPoint, const Plane& Plane )
	{
		float dot = vPoint.Dot(Plane._vNormal);
		if( IsZero(dot) )
			return Plane::Plane_In;
		else if( dot < 0 )
			return Plane::Plane_Back;
		else
			return Plane::Plane_Front;
	}

	//====================================================================
	//*根据屏幕坐标判断一个包围盒是否被选中
	// 算包围盒在屏幕上的区域，判断屏幕上一点是否在这个区域里面
	// vOffset ：是模型偏离boundbox中心的大小
	//====================================================================
	inline bool PickUp( int xScreen, int yScreen, const Vector3f& vBoundBox, const Vector3f& vOffset, 
			        int ViewPortX, int ViewPortY,  int ViewPortWidth, int ViewPortHeight,  
					const Matrix4& matWorld, const Matrix4& matViewProject )
	{
		//              6__________4
		//             /|        /|
		//           0/_|______2/ |
		//            | |       | | 
		//            | |       | |
		//            | |7______|_|5
		//            | /       | /
		//           1|/________|/3


		float x = vBoundBox.x / 2.0f;
		float y = vBoundBox.y / 2.0f;
		float z = vBoundBox.z / 2.0f;

		static Vector3f av[8];
		av[0].SetValue( -x + vOffset.x,  y + vOffset.y, -z + vOffset.z );
		av[1].SetValue( -x + vOffset.x, -y + vOffset.y, -z + vOffset.z );
		av[2].SetValue(  x + vOffset.x,  y + vOffset.y, -z + vOffset.z );
		av[3].SetValue(  x + vOffset.x, -y + vOffset.y, -z + vOffset.z );
		av[4].SetValue(  x + vOffset.x,  y + vOffset.y,  z + vOffset.z );
		av[5].SetValue(  x + vOffset.x, -y + vOffset.y,  z + vOffset.z );
		av[6].SetValue( -x + vOffset.x,  y + vOffset.y,  z + vOffset.z );
		av[7].SetValue( -x + vOffset.x, -y + vOffset.y,  z + vOffset.z );

		Matrix4 mat = matWorld * matViewProject;

		//转成屏幕坐标
		for( int i = 0; i < 8; i++ )
		{
			av[i] = VectorMultiMatrix( av[i],  mat );
			av[i].x = av[i].x*ViewPortWidth/2.0f + ViewPortX + ViewPortWidth/2.0f;
			av[i].y = av[i].y*ViewPortHeight/(-2.0f) + ViewPortY + ViewPortHeight/2.0f;
			av[i].z = 0.0f;
		}

		Vector3f vPt( (float)xScreen, (float)yScreen, 0.0f );

		////计算是否相交
		//for( int i = 0; i < 8; i++ )
		//{
		//    //判断一个点是否在一个三角形内
		//    if( IsPointIntriangle( vPt, av[i % 8], av[ (i+1) % 8 ], av[ (i+2) % 8 ] ) )
		//        return true;
		//}

		//if( IsPointIntriangle( vPt, av[1], av[3], av[7]) )
		//    return true; 

		//if( IsPointIntriangle( vPt, av[1], av[5], av[7]) )
		//    return true;

		//if( IsPointIntriangle( vPt, av[0], av[2], av[4]) )
		//    return true;

		//if( IsPointIntriangle( vPt, av[0], av[4], av[6]) )
		//    return true;
		//////////////////////////////////////////////////////////////////////////
		// By Kedazhao
		// 因为z值都为零，所以不必要进行完全的叉乘，而且z的正负已经足以判断是否选中，不必单位化
		static INT32 Index[12][3] =  
		{
			{ 0, 1, 2 },{ 1, 2, 3 },{ 2, 3, 4 },{ 3, 4, 5 },
			{ 4, 5, 6 },{ 5, 6, 7 },{ 6, 7, 0 },{ 7, 0, 1 },
			{ 1, 3, 7 },{ 3, 5, 7 },{ 0, 2, 4 },{ 0, 4, 6 },
		};

		//计算是否相交
		for( int i = 0; i < 12; i++ )
		{
			//判断一个点是否在一个三角形内
			Vector3f d0 = av[Index[i][0]] - vPt;
			Vector3f d1 = av[Index[i][1]] - vPt;
			Vector3f d2 = av[Index[i][2]] - vPt;

			float z1 = d0.x*d1.y - d0.y*d1.x;
			float z2 = d1.x*d2.y - d1.y*d2.x;
			float z3 = d2.x*d0.y - d2.y*d0.x;

			if( z1 > 0 && ( z2 < 0 || z3 < 0 ) )
				continue; 
			if( z1 < 0 && ( z2 > 0 || z3 > 0 ) )
				continue;

			return true;
		}

		return false;
	}

	inline float SceneToScreen( int& xScreen, int& yScreen, const Vector3f& vScene, 
			int ViewPortX, int ViewPortY,  int ViewPortWidth, int ViewPortHeight, 
			const Matrix4& matView, const Matrix4& matProj )
	{
		/*CMatrix matVP;
		matVP.Identity();
		matVP._11 = viewPort.Width / 2.0f;
		matVP._22 = -1.0f*viewPort.Height / 2.0f;
		matVP._33 = viewPort.MaxZ - viewPort.MinZ;
		matVP._41 = viewPort.X + viewPort.Width / 2.0f;
		matVP._42 = viewPort.Y + viewPort.Height / 2.0f;
		matVP._43 = viewPort.MinZ;*/

		Vector3f vProj = VectorMultiMatrix( VectorMultiMatrix( vScene,  matView),  matProj);

		/*CMatrix matViewProj = matView * matProj;
		CVector3f vProj = vScene*matViewProj;//.FastMultiply( matViewProj );
		float w = vScene.x*matViewProj._14 + vScene.y*matViewProj._24 + vScene.z*matViewProj._34 + matViewProj._44;

		vProj.x /= w;
		vProj.y /= w;
		vProj.z /= w;*/

		xScreen = (int)( vProj.x*ViewPortWidth/2.0f + ViewPortX + ViewPortWidth/2.0f+ 0.5f );
		yScreen = (int)( vProj.y*ViewPortHeight/(-2.0f) + ViewPortY + ViewPortHeight/2.0f + 0.5f );

		return vProj.z;
	}

	/*inline bool ScreenToScene( Vector3f& vOut, int xScreen, int yScreen,  
			                    int ViewPortX, int ViewPortY,  int ViewPortWidth, int ViewPortHeight, 
			                    const Matrix4& matView, const Matrix4& matProj )
	{

	}*/

	//******************************************************************************
	/*! \fun     T TriInterpolate( T& T0, T& T1, T& T2, T& T3, float x, float y )
	*   \brief  在T0~T3之间进行三角形插值
	*
	*   \param  T& T0, T& T1, T& T2, T& T3		插值参数		T1 _____________ T3 ( x = 1.0f, y = 1.0f )
	*			float x, float y				插值位置		| \	    	   |
	*															|   \	       |
	*															|	  \        |
	*															|	    \ 	   |
	*															|	      \    |
	*															|           \  |
	*															T0 ___________\| T2
	*                                                        ( x = 0, y = 0 )
	*															
	*
	*   \return T								插值结果
	*******************************************************************************/
	template< class T >
		T TriInterpolate( const T& T0, const T& T1, const T& T2, const T& T3, float x, float y )
	{
		// 在下面三角形
		if( y < (T)1 - x )
		{
			return (T)( ( T2 - T0 )*x + ( T1 - T0 )*y + T0 );
				
		}
		// 在上面三角形
		else
		{
			return (T)( (  T1 - T3 ) * ( 1.0f - x ) + ( T2 - T3 ) * ( 1.0f - y ) + T3 );
		}
	}

	/* 
		*  射线和平面交点, 需要保证射线的方向u和平面的法线n是单位化的
		*  return = p0 + [ -d - (n dot p0) ] / (n dot u) * u
		*/
	inline Vector3f RayPlaneIntersect( const Ray& ray, const Plane& plane )
	{
		return ray._vOrign + ( -plane._d - plane._vNormal.Dot( ray._vOrign) ) / plane._vNormal.Dot(ray._vDir) * ray._vDir;
	}

	/* 
		* Ray-triangle intersection test.
		*/  
	inline bool RayTriangleIntersect(  Vector3f& p,               // outputIntersectPoint   
			                    const Ray& ray,            // ray with Normalized normal
			                    const Vector3f &p0,        // triangle vertices
								const Vector3f &p1,        // triangle vertices
								const Vector3f &p2         // triangle vertices
			                    )
	{
		// We'll return this huge number if no intersection is detected
		const bool kNoIntersection = false;

		// Compute clockwise edge vectors.
		Vector3f e1 = p1 - p0;
		Vector3f e2 = p2 - p1;

		// Compute surface normal. (Unnormalized)
		Vector3f n = e1.Cross(e2);

		// 直线方向和平面法线的点积
		float dot = n.Dot( ray._vDir );
 
		// 平行,无交点 
		if( IsZero(dot) )
			return kNoIntersection;

		// 平面(法线被单位化了)
		Plane plane(n, p0);

		// 沿着射线所在的直线判断射线源点到平面的距离
		float t = ( -plane._d - plane._vNormal.Dot( ray._vOrign ) ) / plane._vNormal.Dot( ray._vDir );

		// t < 0,无交点
		if( t < 0.0f )
			return kNoIntersection;

		// t > 0, 求交点
		p = ray._vOrign + ray._vDir * t;

		// Find dominant axis to select which plane
		// to project onto, and compute u's and v's
		float u0, u1, u2;
		float v0, v1, v2;
		if (fabs(n.x) > fabs(n.y)) 
		{
			if (fabs(n.x) > fabs(n.z))
			{
				u0 = p.y  - p0.y;
				u1 = p1.y - p0.y;
				u2 = p2.y - p0.y;
				v0 = p.z  - p0.z;
				v1 = p1.z - p0.z;
				v2 = p2.z - p0.z;
			} 
			else 
			{
				u0 = p.x  - p0.x;
				u1 = p1.x - p0.x;
				u2 = p2.x - p0.x;
				v0 = p.y  - p0.y;
				v1 = p1.y - p0.y;
				v2 = p2.y - p0.y;
			}
		}
		else 
		{
			if (fabs(n.y) > fabs(n.z)) 
			{
				u0 = p.x  - p0.x;
				u1 = p1.x - p0.x;
				u2 = p2.x - p0.x;
				v0 = p.z  - p0.z;
				v1 = p1.z - p0.z;
				v2 = p2.z - p0.z;
			} 
			else
			{
				u0 = p.x  - p0.x;
				u1 = p1.x - p0.x;
				u2 = p2.x - p0.x;
				v0 = p.y  - p0.y;
				v1 = p1.y - p0.y;
				v2 = p2.y - p0.y;
			}
		}

		// Compute denominator, check for invalid
		float temp = u1 * v2 - v1 * u2;
		if (!(temp != 0.0f)) 
		{
			return kNoIntersection;
		}
		temp = 1.0f / temp;

		// Compute barycentric coords, checking for out-of-range
		// at each step
		float alpha = (u0 * v2 - v0 * u2) * temp;
		if ( !(alpha >= 0.0f) ) 
		{
			return kNoIntersection;
		}

		float beta = (u1 * v0 - v1 * u0) * temp;
		if (!(beta >= 0.0f)) 
		{
			return kNoIntersection;
		}

		float gamma = 1.0f - alpha - beta;
		if (!(gamma >= 0.0f)) 
		{
			return kNoIntersection;
		}

		return true;
	}

	#define XX 0
	#define YY 1
	#define ZZ 2

	#define EPSILON 0.000001
	#define CROSS(dest,v1,v2) \
			dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
			dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
			dest[2]=v1[0]*v2[1]-v1[1]*v2[0]; 

	#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

	#define SUB(dest,v1,v2) \
			dest[0]=v1[0]-v2[0]; \
			dest[1]=v1[1]-v2[1]; \
			dest[2]=v1[2]-v2[2]; 

	#define FINDMINMAX(x0,x1,x2,min,max) \
	min = max = x0;   \
	if(x1<min) min=x1;\
	if(x1>max) max=x1;\
	if(x2<min) min=x2;\
	if(x2>max) max=x2;

	inline bool RayTriangleIntersect2(float orig[3],float dir[3],float vert0[3],float vert1[3],float vert2[3],float *t,float *u,float *v)
	{
		float edge1[3],edge2[3],tvec[3],pvec[3],qvec[3];
		float det,inv_det;

		/* find vectors for two edges sharing vert0 */
		SUB(edge1, vert1, vert0);
		SUB(edge2, vert2, vert0);

		/* begin calculating determinant - also used to calculate U parameter */
		CROSS(pvec, dir, edge2);

		/* if determinant is near zero, ray lies in plane of triangle */
		det = DOT(edge1, pvec);

#ifdef TEST_CULL
		/* define TEST_CULL if culling is desired */
		if (det < EPSILON)
			return false;

		/* calculate distance from vert0 to ray origin */
		SUB(tvec, orig, vert0);

		/* calculate U parameter and test bounds */
		*u = DOT(tvec, pvec)
		if (*u < 0.0 || *u > det)
			return false;

		/* prepare to test V parameter */
		CROSS(qvec, tvec, edge1);

		/* calculate V parameter and test bounds */
		*v = DOT(dir, qvec);
		if (*v < 0.0 || *u + *v > det);
			return false;

		/* calculate t, scale parameters, ray intersects triangle */
		*t = DOT(edge2, qvec);
		inv_det = 1.0 / det;
		*t *= inv_det;
		*u *= inv_det;
		*v *= inv_det;
#else
		/* the non-culling branch */
		if (det > -EPSILON && det < EPSILON)
			return false;

		inv_det = 1.0f / det;

		/* calculate distance from vert0 to ray origin */
		SUB(tvec, orig, vert0);

		/* calculate U parameter and test bounds */
		*u = DOT(tvec, pvec) * inv_det;
		if (*u < 0.0 || *u > 1.0)
			return false;

		/* prepare to test V parameter */
		CROSS(qvec, tvec, edge1);

		/* calculate V parameter and test bounds */
		*v = DOT(dir, qvec) * inv_det;
		if (*v < 0.0 || *u + *v > 1.0)
			return false;

		/* calculate t, ray intersects triangle */
		*t = DOT(edge2, qvec) * inv_det;
#endif
		return true;
	}

	inline int planeBoxOverlap(float normal[3], float vert[3], float maxbox[3])	// -NJMP-
	{
		int q;
		float vmin[3],vmax[3],v;

		for(q=XX;q<=ZZ;q++)
		{
			v=vert[q];					// -NJMP-
			if(normal[q]>0.0f)
			{
				vmin[q]=-maxbox[q] - v;	// -NJMP-
				vmax[q]= maxbox[q] - v;	// -NJMP-
			}
			else
			{
				vmin[q]= maxbox[q] - v;	// -NJMP-
				vmax[q]=-maxbox[q] - v;	// -NJMP-
			}
		}

		if(DOT(normal,vmin)>0.0f) return 0;	// -NJMP-
		if(DOT(normal,vmax)>=0.0f) return 1;	// -NJMP-
			
		return 0;
	}

	inline bool Rect2Rect(float r1x1, float r1y1, float r1x2, float r1y2, float r2x1, float r2y1, float r2x2, float r2y2)
	{
		if (r2x1 >= r1x2 || r2y1 >= r1y2 || r2x2 <= r1x1 || r2y2 <= r1y1) 
		{
			return false;
		} 
		else 
		{
			return true;
		}
	}

	/*======================== X-tests ========================*/

	#define AXISTEST_X01(a, b, fa, fb)			   \
		p0 = a*v0[YY] - b*v0[ZZ];			       	   \
		p2 = a*v2[YY] - b*v2[ZZ];			       	   \
		if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
		rad = fa * boxhalfsize[YY] + fb * boxhalfsize[ZZ];   \
		if(min>rad || max<-rad) return 0;

	#define AXISTEST_X2(a, b, fa, fb)			   \
		p0 = a*v0[YY] - b*v0[ZZ];			           \
		p1 = a*v1[YY] - b*v1[ZZ];			       	   \
		if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
		rad = fa * boxhalfsize[YY] + fb * boxhalfsize[ZZ];   \
		if(min>rad || max<-rad) return 0;

	/*======================== Y-tests ========================*/

	#define AXISTEST_Y02(a, b, fa, fb)			   \
		p0 = -a*v0[XX] + b*v0[ZZ];		      	   \
		p2 = -a*v2[XX] + b*v2[ZZ];	       	       	   \
		if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
		rad = fa * boxhalfsize[XX] + fb * boxhalfsize[ZZ];   \
		if(min>rad || max<-rad) return 0;

	#define AXISTEST_Y1(a, b, fa, fb)			   \
		p0 = -a*v0[XX] + b*v0[ZZ];		      	   \
		p1 = -a*v1[XX] + b*v1[ZZ];	     	       	   \
		if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
		rad = fa * boxhalfsize[XX] + fb * boxhalfsize[ZZ];   \
		if(min>rad || max<-rad) return 0;

	/*======================== Z-tests ========================*/

	#define AXISTEST_Z12(a, b, fa, fb)			   \
		p1 = a*v1[XX] - b*v1[YY];			           \
		p2 = a*v2[XX] - b*v2[YY];			       	   \
		if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
		rad = fa * boxhalfsize[XX] + fb * boxhalfsize[YY];   \
		if(min>rad || max<-rad) return 0;

	#define AXISTEST_Z0(a, b, fa, fb)			   \
		p0 = a*v0[XX] - b*v0[YY];				   \
		p1 = a*v1[XX] - b*v1[YY];			           \
		if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
		rad = fa * boxhalfsize[XX] + fb * boxhalfsize[YY];   \
		if(min>rad || max<-rad) return 0;



	inline bool TriangleAABBIntersect( float boxcenter[3],float boxhalfsize[3],float triverts[3][3] )
	{
		/*    use separating axis theorem to test overlap between triangle and box */
		/*    need to test for overlap in these directions: */
		/*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
		/*       we do not even need to test these) */
		/*    2) normal of the triangle */
		/*    3) crossproduct(edge from tri, {x,y,z}-directin) */
		/*       this gives 3x3=9 more tests */

		float v0[3],v1[3],v2[3];

		//   float axis[3];

		float min,max,p0,p1,p2,rad,fex,fey,fez;		// -NJMP- "d" local variable removed

		float normal[3],e0[3],e1[3],e2[3];

		/* This is the fastest branch on Sun */
		/* move everything so that the boxcenter is in (0,0,0) */

		SUB(v0,triverts[0],boxcenter);
		SUB(v1,triverts[1],boxcenter);
		SUB(v2,triverts[2],boxcenter);

		/* compute triangle edges */
		SUB(e0,v1,v0);      /* tri edge 0 */
		SUB(e1,v2,v1);      /* tri edge 1 */
		SUB(e2,v0,v2);      /* tri edge 2 */

		/* Bullet 3:  */
		/*  test the 9 tests first (this was faster) */
		fex = fabsf(e0[XX]);
		fey = fabsf(e0[YY]);
		fez = fabsf(e0[ZZ]);

		AXISTEST_X01(e0[ZZ], e0[YY], fez, fey);
		AXISTEST_Y02(e0[ZZ], e0[XX], fez, fex);
		AXISTEST_Z12(e0[YY], e0[XX], fey, fex);


		fex = fabsf(e1[XX]);
		fey = fabsf(e1[YY]);
		fez = fabsf(e1[ZZ]);

		AXISTEST_X01(e1[ZZ], e1[YY], fez, fey);
		AXISTEST_Y02(e1[ZZ], e1[XX], fez, fex);
		AXISTEST_Z0(e1[YY], e1[XX], fey, fex);


		fex = fabsf(e2[XX]);
		fey = fabsf(e2[YY]);
		fez = fabsf(e2[ZZ]);

		AXISTEST_X2(e2[ZZ], e2[YY], fez, fey);
		AXISTEST_Y1(e2[ZZ], e2[XX], fez, fex);
		AXISTEST_Z12(e2[YY], e2[XX], fey, fex);

		/* Bullet 1: */
		/*  first test overlap in the {x,y,z}-directions */
		/*  find min, max of the triangle each direction, and test for overlap in */
		/*  that direction -- this is equivalent to testing a minimal AABB around */
		/*  the triangle against the AABB */

		/* test in X-direction */

		FINDMINMAX(v0[XX],v1[XX],v2[XX],min,max);

		if(min>boxhalfsize[XX] || max<-boxhalfsize[XX]) return 0;

		/* test in Y-direction */

		FINDMINMAX(v0[YY],v1[YY],v2[YY],min,max);

		if(min>boxhalfsize[YY] || max<-boxhalfsize[YY]) return 0;

		/* test in Z-direction */

		FINDMINMAX(v0[ZZ],v1[ZZ],v2[ZZ],min,max);

		if(min>boxhalfsize[ZZ] || max<-boxhalfsize[ZZ]) return 0;

		/* Bullet 2: */
		/*  test if the box intersects the plane of the triangle */
		/*  compute plane equation of triangle: normal*x+d=0 */

		CROSS(normal,e0,e1);

		// -NJMP- (line removed here)

		if(!planeBoxOverlap(normal,v0,boxhalfsize)) return 0;	// -NJMP-

		return 1;   /* box and triangle overlaps */
	};

	/*
	//  AABB和Frustum的香蕉检测，用于包围层次结构
	*/
	inline IntersectionResult AxisAlignedBoundingBox2Frustum(AxisAlignedBoundingBox* const pAxisAlignedBoundingBox,  const OptFrustum* const pFrustum, UINT32 inMask, UINT32& outMask )
	{
		if( !pAxisAlignedBoundingBox || !pFrustum )
			return IR_Outside;

		float m, n;
		UINT32 k = 1 << pAxisAlignedBoundingBox->_startID;
		outMask = 0;
		IntersectionResult result = IR_Inside;

		OptPlane* sp = (OptPlane*)pFrustum->_planes[pAxisAlignedBoundingBox->_startID];

		if( k & inMask )
		{
			// if P_Vertex is outside then N_vertex is also outside, the result is outside
			// P dot Plane.n + d, Plane.n has been normolized
			m = sp->_vNormal.x * pAxisAlignedBoundingBox->_vMinMax[ 1 - sp->_NPVer[0] ].x +
				sp->_vNormal.y * pAxisAlignedBoundingBox->_vMinMax[ 1 - sp->_NPVer[1] ].y +
				sp->_vNormal.z * pAxisAlignedBoundingBox->_vMinMax[ 1 - sp->_NPVer[2] ].z + sp->_d;

			if( m < 0.0f )
				return IR_Outside;

			// if P_Vertex is Inside then test N_Vertex
			// if N_Vertex is outside then the result is intersect, else the result is inside
			n = sp->_vNormal.x * pAxisAlignedBoundingBox->_vMinMax[ sp->_NPVer[0] ].x +
				sp->_vNormal.y * pAxisAlignedBoundingBox->_vMinMax[ sp->_NPVer[1] ].y +
				sp->_vNormal.z * pAxisAlignedBoundingBox->_vMinMax[ sp->_NPVer[2] ].z + sp->_d;

			if( n < 0.0f )
			{
				outMask |= k;
				pAxisAlignedBoundingBox->SetPlaneMask( (ClipMask)outMask );
				result = IR_Intersect;
			}
		}

		for (UINT32 i = 0, k = 1; k <= inMask; i++, k += k )
		{
			if( (i != pAxisAlignedBoundingBox->_startID) && (k & inMask) )
			{
				sp = (OptPlane*)pFrustum->_planes[i];

				// if P_Vertex is outside then N_vertex is also outside, the result is outside
				// P dot Plane.n + d, Plane.n has been normolized
				m = sp->_vNormal.x * pAxisAlignedBoundingBox->_vMinMax[ 1 - sp->_NPVer[0] ].x +
					sp->_vNormal.y * pAxisAlignedBoundingBox->_vMinMax[ 1 - sp->_NPVer[1] ].y +
					sp->_vNormal.z * pAxisAlignedBoundingBox->_vMinMax[ 1 - sp->_NPVer[2] ].z + sp->_d;

				if( m < 0.0f )
				{
					pAxisAlignedBoundingBox->_startID = i;
					return IR_Outside;
				}

				// if P_Vertex is Inside then test N_Vertex
				// if N_Vertex is outside then the result is intersect, else the result is inside
				n = sp->_vNormal.x * pAxisAlignedBoundingBox->_vMinMax[ sp->_NPVer[0] ].x +
					sp->_vNormal.y * pAxisAlignedBoundingBox->_vMinMax[ sp->_NPVer[1] ].y +
					sp->_vNormal.z * pAxisAlignedBoundingBox->_vMinMax[ sp->_NPVer[2] ].z + sp->_d;

				if( n < 0.0f )
				{
					outMask |= k;
					pAxisAlignedBoundingBox->SetPlaneMask( (ClipMask)outMask );
					result = IR_Intersect;
				}
			}
		}

		return result;
	}

	/*
	//  AABB和Frustum的相交检测
	*/
	inline IntersectionResult AxisAlignedBoundingBox2Frustum(AxisAlignedBoundingBox* const pAxisAlignedBoundingBox,  const OptFrustum* const pFrustum )
	{
		assert(pAxisAlignedBoundingBox && pFrustum );

		IntersectionResult result = IR_Inside;

		float m, n;
		OptPlane* sp = (OptPlane*)pFrustum->_planes[pAxisAlignedBoundingBox->_startID];

		// Check k
		// if P_Vertex is outside then N_vertex is also outside, the result is outside
		// P dot Plane.n + d, Plane.n has been normolized
		m = sp->_vNormal.x * pAxisAlignedBoundingBox->_vMinMax[ 1 - sp->_NPVer[0] ].x +
			sp->_vNormal.y * pAxisAlignedBoundingBox->_vMinMax[ 1 - sp->_NPVer[1] ].y +
			sp->_vNormal.z * pAxisAlignedBoundingBox->_vMinMax[ 1 - sp->_NPVer[2] ].z + sp->_d;

		if( m < 0.0f )
			return IR_Outside;

		// if P_Vertex is Inside then test N_Vertex
		// if N_Vertex is outside then the result is intersect, else the result is inside
		n = sp->_vNormal.x * pAxisAlignedBoundingBox->_vMinMax[ sp->_NPVer[0] ].x +
			sp->_vNormal.y * pAxisAlignedBoundingBox->_vMinMax[ sp->_NPVer[1] ].y +
			sp->_vNormal.z * pAxisAlignedBoundingBox->_vMinMax[ sp->_NPVer[2] ].z + sp->_d;

		if( n < 0.0f )
			result = IR_Intersect;

		for ( int i = 0; i < 6 ; i++ )
		{
			if (i== pAxisAlignedBoundingBox->_startID)
				continue;

			sp = (OptPlane*)pFrustum->_planes[i];

			// if P_Vertex is outside then N_vertex is also outside, the result is outside
			// P dot Plane.n + d, Plane.n has been normolized
			m = sp->_vNormal.x * pAxisAlignedBoundingBox->_vMinMax[ 1 - sp->_NPVer[0] ].x +
				sp->_vNormal.y * pAxisAlignedBoundingBox->_vMinMax[ 1 - sp->_NPVer[1] ].y +
				sp->_vNormal.z * pAxisAlignedBoundingBox->_vMinMax[ 1 - sp->_NPVer[2] ].z + sp->_d;

			if( m < 0.0f )
			{
				pAxisAlignedBoundingBox->_startID = i;
				return IR_Outside;
			}

			// if P_Vertex is Inside then test N_Vertex
			// if N_Vertex is outside then the result is intersect, else the result is inside
			n = sp->_vNormal.x * pAxisAlignedBoundingBox->_vMinMax[ sp->_NPVer[0] ].x +
				sp->_vNormal.y * pAxisAlignedBoundingBox->_vMinMax[ sp->_NPVer[1] ].y +
				sp->_vNormal.z * pAxisAlignedBoundingBox->_vMinMax[ sp->_NPVer[2] ].z + sp->_d;

			if( n < 0.0f )
				result = IR_Intersect;
		}

		return result;
	}

	/*
	//  AABB和Frustum的相交检测
	*/
	inline IntersectionResult AxisAlignedBoundingBox2(AxisAlignedBoundingBox* const pAxisAlignedBoundingBox1, AxisAlignedBoundingBox* const pAxisAlignedBoundingBox2)
	{
		assert(pAxisAlignedBoundingBox1 && pAxisAlignedBoundingBox2);

		Vector3f t_length = pAxisAlignedBoundingBox1->Center();
		t_length -= pAxisAlignedBoundingBox2->Center();

		if ((2.0f*fabs(t_length.x)<=(pAxisAlignedBoundingBox1->XSize()+ pAxisAlignedBoundingBox2->XSize()))&&(2.0f*fabs(t_length.y)<=(pAxisAlignedBoundingBox1->YSize()+ pAxisAlignedBoundingBox2->YSize()))&&(2.0f*fabs(t_length.z)<=(pAxisAlignedBoundingBox1->ZSize()+ pAxisAlignedBoundingBox2->ZSize())))
			return IR_Intersect;

		return IR_Outside;
	}

	inline bool AxisAlignedBoundingBoxIsInclude(AxisAlignedBoundingBox* const pAxisAlignedBoundingBoxBeInclude, AxisAlignedBoundingBox* const pAxisAlignedBoundingBoxInclude)
	{
		assert(pAxisAlignedBoundingBoxBeInclude && pAxisAlignedBoundingBoxInclude);
		
		Vector3f vMin1 = pAxisAlignedBoundingBoxBeInclude->_vMinMax[AxisAlignedBoundingBox::Ver_Min];
		Vector3f vMax1 = pAxisAlignedBoundingBoxBeInclude->_vMinMax[AxisAlignedBoundingBox::Ver_Max];
		Vector3f vMin2 = pAxisAlignedBoundingBoxInclude->_vMinMax[AxisAlignedBoundingBox::Ver_Min];
		Vector3f vMax2 = pAxisAlignedBoundingBoxInclude->_vMinMax[AxisAlignedBoundingBox::Ver_Max];

		//
		if ((vMin1.x > vMin2.x) && (vMin1.y > vMin2.y) && (vMin1.z > vMin2.z) && (vMax1.x < vMax2.x) && (vMax1.y < vMax2.y) && (vMax1.z < vMax2.z))
		{
			return true;
		}
		return false;
	}

	/*
	//  AABB和Sphere的相交检测
	*/
	inline IntersectionResult AxisAlignedBoundingBox2Sphere( const AxisAlignedBoundingBox* const pAxisAlignedBoundingBox, const Vector4f* const pCenter, float fRadius )
	{
		assert(pAxisAlignedBoundingBox && pCenter );

		float s, d = 0;
		if (pCenter->x<pAxisAlignedBoundingBox->_vMinMax[AxisAlignedBoundingBox::Ver_Min].x)
		{
			s	= pCenter->x- pAxisAlignedBoundingBox->_vMinMax[AxisAlignedBoundingBox::Ver_Min].x;
			d  += s*s;
		}
		else if (pCenter->x>pAxisAlignedBoundingBox->_vMinMax[AxisAlignedBoundingBox::Ver_Max].x)
		{
			s	= pCenter->x- pAxisAlignedBoundingBox->_vMinMax[AxisAlignedBoundingBox::Ver_Max].x;
			d  += s*s;
		}

		if (pCenter->y<pAxisAlignedBoundingBox->_vMinMax[AxisAlignedBoundingBox::Ver_Min].y)
		{
			s	= pCenter->y- pAxisAlignedBoundingBox->_vMinMax[AxisAlignedBoundingBox::Ver_Min].y;
			d  += s*s;
		}
		else if (pCenter->y>pAxisAlignedBoundingBox->_vMinMax[AxisAlignedBoundingBox::Ver_Max].y)
		{
			s	= pCenter->y- pAxisAlignedBoundingBox->_vMinMax[AxisAlignedBoundingBox::Ver_Max].y;
			d  += s*s;
		}

		if (pCenter->z<pAxisAlignedBoundingBox->_vMinMax[AxisAlignedBoundingBox::Ver_Min].z)
		{
			s	= pCenter->z- pAxisAlignedBoundingBox->_vMinMax[AxisAlignedBoundingBox::Ver_Min].z;
			d  += s*s;
		}
		else if (pCenter->z>pAxisAlignedBoundingBox->_vMinMax[AxisAlignedBoundingBox::Ver_Max].z)
		{
			s	= pCenter->z- pAxisAlignedBoundingBox->_vMinMax[AxisAlignedBoundingBox::Ver_Max].z;
			d  += s*s;
		}

		if (d <= fRadius*fRadius)
			return IR_Intersect;
		else
			return IR_Outside;
	}

    /*
	//---------------------------------------------------------------------------
	//  Ray to AABB
	//  return:  vNear -- near IntersectPoint, vFar -- far IntersectPoint
	inline bool Ray2AABB( bool& bIntersectIsNear, Vector3f& vNear, Vector3f& vFar, const Ray& ray, const BoundingAABB& AABB )
	{
		const float kBigNumber = 1e37f;
		float tMin = -kBigNumber;
		float tMax = kBigNumber;

		float Mins[3], Maxs[3];
		Mins[0] = AABB._vMinMax[BoundingAABB::Ver_Min].x + 1.0f;
		Mins[1] = AABB._vMinMax[BoundingAABB::Ver_Min].y + 1.0f;
		Mins[2] = AABB._vMinMax[BoundingAABB::Ver_Min].z + 1.0f;

		Maxs[0] = AABB._vMinMax[BoundingAABB::Ver_Max].x - 1.0f;
		Maxs[1] = AABB._vMinMax[BoundingAABB::Ver_Max].y - 1.0f;
		Maxs[2] = AABB._vMinMax[BoundingAABB::Ver_Max].z - 1.0f;

		bool bIntersect = false;
		float bMin, bMax, t;
		for( int i = 0; i < 3; i++ )
		{
			if( IsZero( ray._vDir._v[i] ) )
				continue;

			// 根据射线方向确定远近区间
			if( ray._vDir._v[i] > 0 )
			{
				bMin = Mins[i];
				bMax = Maxs[i];

				if( ray._vOrign._v[i] > bMax )
					continue;
			}
			else
			{
				bMin = Maxs[i];
				bMax = Mins[i];

				if( ray._vOrign._v[i] < bMax )
					continue;
			}

				
			// 求参数
			t = (bMax - ray._vOrign._v[i]) / ray._vDir._v[i];

			if( t < tMax )
			{
				//if( t < tMin )
				//	continue;

				tMax = t;
				bIntersect = true;
			}

			t = (bMin - ray._vOrign._v[i])/ray._vDir._v[i];

			if( t > 0.0f )
			{
				if( t > tMin )
				{
					//if( t > tMax ) 
					//	continue;
					tMin = t;
					bIntersect = true;
				}	
			}

							
		}

		for( int i = 0; i < 3; i++ )
		{
			vNear._v[i] = ray._vOrign._v[i] + tMin * ray._vDir._v[i];
			vFar._v[i] = ray._vOrign._v[i] + tMax * ray._vDir._v[i];
		}

		if( bIntersect )
        {
            if( AABB.Contains(vNear) )
            {
                bIntersectIsNear = true;
                return true;
            }
            if( AABB.Contains(vFar) )
            {
                bIntersectIsNear = false;
                return true;
            }
            else
                return false;
        }
				

		return bIntersect;
	}
    */

    //---------------------------------------------------------------------------
    //  Ray to AABB
    //  return:  vNear -- near IntersectPoint, vFar -- far IntersectPoint
    inline bool Ray2AxisAlignedBoundingBox( Vector3f& vIntersectIsNear, const Ray& ray, const AxisAlignedBoundingBox& rAxisAlignedBoundingBox )
    {
        const float kBigNumber = 1e37f;
        float tMin = -kBigNumber;
        float tMax = kBigNumber;

        float Mins[3], Maxs[3];
        Mins[0] = rAxisAlignedBoundingBox._vMinMax[AxisAlignedBoundingBox::Ver_Min].x + 1.0f;
        Mins[1] = rAxisAlignedBoundingBox._vMinMax[AxisAlignedBoundingBox::Ver_Min].y + 1.0f;
        Mins[2] = rAxisAlignedBoundingBox._vMinMax[AxisAlignedBoundingBox::Ver_Min].z + 1.0f;

        Maxs[0] = rAxisAlignedBoundingBox._vMinMax[AxisAlignedBoundingBox::Ver_Max].x - 1.0f;
        Maxs[1] = rAxisAlignedBoundingBox._vMinMax[AxisAlignedBoundingBox::Ver_Max].y - 1.0f;
        Maxs[2] = rAxisAlignedBoundingBox._vMinMax[AxisAlignedBoundingBox::Ver_Max].z - 1.0f;

        bool bIntersect = false;
        float bMin, bMax, t;
        for( int i = 0; i < 3; i++ )
        {
            if( IsZero( ray._vDir._v[i] ) )
                continue;

            // 根据射线方向确定远近区间
            if( ray._vDir._v[i] > 0 )
            {
                bMin = Mins[i];
                bMax = Maxs[i];

                if( ray._vOrign._v[i] > bMax )
                    continue;
            }
            else
            {
                bMin = Maxs[i];
                bMax = Mins[i];

                if( ray._vOrign._v[i] < bMax )
                    continue;
            }


            // 求参数
            t = (bMax - ray._vOrign._v[i]) / ray._vDir._v[i];

            if( t < tMax )
            {
                //if( t < tMin )
                //	continue;

                tMax = t;
                bIntersect = true;
            }

            t = (bMin - ray._vOrign._v[i])/ray._vDir._v[i];

            if( t > 0.0f )
            {
                if( t > tMin )
                {
                    //if( t > tMax ) 
                    //	continue;
                    tMin = t;
                    bIntersect = true;
                }	
            }


        }

        Vector3f vNear, vFar;
        for( int i = 0; i < 3; i++ )
        {
            vNear._v[i] = ray._vOrign._v[i] + tMin * ray._vDir._v[i];
            vFar._v[i] = ray._vOrign._v[i] + tMax * ray._vDir._v[i];
        }

        if( bIntersect )
        {
            if(rAxisAlignedBoundingBox.Contains(vNear) )
            {
                vIntersectIsNear = vNear;
                return true;
            }
            if(rAxisAlignedBoundingBox.Contains(vFar) )
            {
                vIntersectIsNear = vFar;
                return true;
            }
            else
                return false;
        }


        return bIntersect;
    }
}
using namespace Math;