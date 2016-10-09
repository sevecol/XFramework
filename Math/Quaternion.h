
#pragma once
#include "MathComm.h"
#include "Vector3.h"

namespace Math
{
	class Quaternion
	{
	public:
		float x, y, z, s;

		//构造，析构

		Quaternion(void) : s(1.0f), x(0.0f), y(0.0f), z(0.0f){};
		Quaternion(float fs, float fx, float fy, float fz):s(fs), x(fx), y(fy), z(fz) {};
		Quaternion( const Quaternion& quat ) : s(quat.s), x(quat.x), y(quat.y), z(quat.z) {};


		//运算符重载

		inline Quaternion& operator = (const Quaternion& quat)
		{
			s = quat.s;
			x = quat.x;
			y = quat.y;
			z = quat.z;

			return *this;
		}

		inline Quaternion operator + (const Quaternion& quat)
		{
			Quaternion q;

			q.s = s + quat.s;
			q.x = x + quat.x;
			q.y = y + quat.y;
			q.z = z + quat.z;

			return q;
		}

		inline Quaternion operator * (const Quaternion& quat)
		{
			Quaternion q;

			q.s = s*quat.s - x*quat.x - y*quat.y - z*quat.z;
			q.x = s*quat.x + x*quat.s + y*quat.z - z*quat.y;
			q.y = s*quat.y + y*quat.s + z*quat.x - x*quat.z;
			q.z = s*quat.z + z*quat.s + x*quat.y - y*quat.x;

			return q;
		}

		inline Quaternion& operator += ( const Quaternion& quat )
		{
			s += quat.s;
			x += quat.x;
			y += quat.y;
			z += quat.z;

			return *this;
		}

		inline Quaternion& operator *= (const Quaternion& quat)
		{
			*this = *this * quat;
				
			return *this;
		}

		//归一化四元数
		void Normalize()
		{
			float fSqrt = std::sqrt((s*s + x*x + y*y + z*z));

			if( !IsZero(fSqrt) )
			{
				float fReci = 1.0f / fSqrt;

				s *= fReci;
				x *= fReci;
				y *= fReci;
				z *= fReci;
			}
		}

		//单位四元数
		void LoadIdentify()
		{
			s = 1.0f;
			x = y = z = 0.0f;
		}

		// Setup the quaternion to a specific rotation
		void FromRotateX( float fAngle )
		{
			// Compute the half angle
			float halfAngle = fAngle * 0.5f;
			// Set the values
			s = std::cos(halfAngle);
			x = std::sin(halfAngle);
			y = 0.0f;
			z = 0.0f;
		}

		void FromRotateY( float fAngle )
		{
			// Compute the half angle
			float halfAngle = fAngle * 0.5f;
			// Set the values
			s = std::cos(halfAngle);
			x = 0.0f;
			y = std::sin(halfAngle);
			z = 0.0f;
		}

		void FromRotateZ(float fAngle)
		{
			// Compute the half angle
			float halfAngle = fAngle * 0.5f;
			// Set the values
			s = std::cos(halfAngle);
			x = 0.0f;
			y = 0.0f;
			z = std::sin(halfAngle);
		}

		void FromRotateAxisAngle(const Vector3f &axis, float fAngle)
		{
			// The axis of rotation must be normalized
			Vector3f v (axis);
			v.Normalize();
		
			// Compute the half angle and its sin
			float halfAngle = fAngle * 0.5f;
			float sinHalfAngle = std::sin(halfAngle);
			// Set the values
			s = std::cos(halfAngle);
			x = v.x * sinHalfAngle;
			y = v.y * sinHalfAngle;
			z = v.z * sinHalfAngle;
		}
			

		// Extract and return the rotation angle and axis.
		float GetRotationAngle() const
		{
			// s = cos(theta / 2)
			float sClamp = Clamp( s, -1.0f, 1.0f );
			float thetaOver2 = std::acos(sClamp);
            return thetaOver2 * 2.0f;
		}

		Vector3f GetRotationAxis() const
		{
			// Compute sin^2(theta/2). Remember that s = cos(theta/2),
			// and sin^2(x) + cos^2(x) = 1

			float sinThetaOver2Sq = 1.0f - s*s;
			// Protect against numerical imprecision
			if (sinThetaOver2Sq <= 0.0f) 
			{
				// Identity quaternion, or numerical imprecision. Just
				// return any valid vector, since it doesn't matter
				return Vector3f(1.0f, 0.0f, 0.0f);
			}
			// Compute 1 / sin(theta/2)
			float oneOverSinThetaOver2 = 1.0f / sqrt(sinThetaOver2Sq);
			// Return axis of rotation
			return Vector3f(
				x * oneOverSinThetaOver2,
				y * oneOverSinThetaOver2,
				z * oneOverSinThetaOver2
				);
		}

		//从欧拉角创建四元数
		void FromEular(float aAngle, float bAngle, float cAngle)
		{
			float angle;
			double sr, sp, sy;
			double cr, cp, cy;

			angle = cAngle*0.5f;
			sy = sin(angle);
			cy = cos(angle);

			angle = bAngle*0.5f;
			sp = sin(angle);
			cp = cos(angle);

			angle = aAngle*0.5f;
			sr = sin(angle);
			cr = cos(angle);

			double crcp = cr*cp;
			double srsp = sr*sp;

			s = (float)(crcp*cy - srsp*sy);
			x = (float)(sr*cp*cy + cr*sp*sy);
			y = (float)(cr*sp*cy - sr*cp*sy);
			z = (float)(crcp*sy + srsp*cy); 
		}

		// Dot Product
		inline float Dot( const Quaternion& q ) const
		{
			return s*q.s + x*q.x + y*q.y + z*q.z;
		}

		/////////////////////////////////////////////////
		//从四元数得到矩阵，
		//fMat保存返回的矩阵，列优先
		/////////////////////////////////////////////////
		void ToMatrix(float *fMat)
		{
			float twoxx = 2.0f * x * x;
			float twoyy = 2.0f * y * y;
			float twozz = 2.0f * z * z;
			float twoxy = 2.0f * x * y;
			float twoyz = 2.0f * y * z;
			float twoxz = 2.0f * x * z;
			float twosx = 2.0f * s * x;
			float twosy = 2.0f * s * y;
			float twosz = 2.0f * s * z;

			memset( (void*)fMat, 0, 16*sizeof(float) );
			fMat[15] = 1.0f;

			fMat[0] = 1.0f - twoyy - twozz; 	fMat[1] = twoxy + twosz; 	        fMat[2] = twoxz - twosy;
			fMat[4] = twoxy - twosz;			fMat[5] = 1.0f - twoxx - twozz;     fMat[6] = twoyz + twosx;
			fMat[8] = twoxz + twosy;            fMat[9] = twoyz - twosx;            fMat[10] = 1.0f - twoxx - twoyy;
		}

		//////////////////////////////////////////////////
		//球形线性插值插值
		//q1,q2:要插值的两个四元数
		//t    :插值参数
		//////////////////////////////////////////////////
		bool Slerp(const Quaternion& q1, const Quaternion& q2, float t)
		{
			if(t < 0.0f)
				return false;
			if( IsZero(t) )
			{
				*this = q1;
				return true;
			}
			else if( IsZero(1.0f-t) )
			{
				*this = q2;
				return true;
			}

			// 检查q1和q2是否相等，如果相等，则直接把q1作为插值的结果
			if(q1.x == q2.x && q1.y == q2.y && q1.z == q2.z && q1.s == q2.s) 
			{
				*this = q1;
				return false;
			}

			// 用来存储临时插值结果的四元组
			Quaternion q2Bk = q2;

			// Following the (b.a) part of the equation, we do a dot product between q1 and q2.
			// We can do a dot product because the same math applied for a 3D vector as a 4D vector.
			float cosTheta = (q1.x * q2.x) + (q1.y * q2.y) + (q1.z * q2.z) + (q1.s * q2.s);

			// If the dot product is less than 0, the angle is greater than 90 degrees
			if(cosTheta < 0.0f)
			{
				// Negate the second quaternion and the result of the dot product
				q2Bk = Quaternion(-q2.s, -q2.x, -q2.y, -q2.z);
				cosTheta = -cosTheta;
			}

			// Check if they are very close together to protect
			// against divide-by-zero
			float scale0, scale1;
			if (cosTheta > 0.9999f) 
			{
				// Very close - just use linear interpolation
				scale0 = 1.0f - t;
				scale1 = t;
			}
			else
			{
				// Compute the sin of the angle using the
				// trig identity sin^2(theta) + cos^2(theta) = 1
				float sinTheta = sqrt(1.0f - cosTheta*cosTheta);
				// Compute the angle from its sin and cosine
				float theta = atan2(sinTheta, cosTheta);
				// Compute inverse of denominator, so we only have
				// to divide once
				float oneOverSinTheta = 1.0f / sinTheta;
				// Compute interpolation parameters
				scale0 = sin((1.0f - t) * theta) * oneOverSinTheta;
				scale1 = sin(t * theta) * oneOverSinTheta;
			}

			// Calculate the x, y, z and w values for the quaternion by using a special
			// form of linear interpolation for quaternions.
			x = (scale0 * q1.x) + (scale1 * q2Bk.x);
			y = (scale0 * q1.y) + (scale1 * q2Bk.y);
			z = (scale0 * q1.z) + (scale1 * q2Bk.z);
			s = (scale0 * q1.s) + (scale1 * q2Bk.s);

			// Return the interpolated quaternion
			return true;
		}
	};
}
using namespace Math;