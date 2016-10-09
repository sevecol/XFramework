
#pragma once
#include "Vector3.h"

namespace Math
{
	#define Laxis 0
	#define Saxis 1
	#define Taxis 2

	struct Ray
	{
		Vector3f _vOrign;
		Vector3f _vDir;
	};

	struct RayEx : public Ray
	{
		Vector3f		m_axis[3];
		float			m_intercept[3][3],m_increment[3][3];

		RayEx(const Ray& ray)
		{
			_vOrign		= ray._vOrign;
			_vDir			= ray._vDir;

			if (fabs(ray._vDir.x)>fabs(ray._vDir.y))
			{
				if (fabs(ray._vDir.x)>fabs(ray._vDir.z))
				{
					m_axis[Laxis] = Vector3f(1,0,0);
					if (fabs(ray._vDir.y)>fabs(ray._vDir.z))
					{
						m_axis[Saxis] = Vector3f(0,1,0);
						m_axis[Taxis] = Vector3f(0,0,1);
					}
					else
					{
						m_axis[Saxis] = Vector3f(0,0,1);
						m_axis[Taxis] = Vector3f(0,1,0);
					}
				}
				else
				{
					m_axis[Laxis] = Vector3f(0,0,1);
					m_axis[Saxis] = Vector3f(1,0,0);
					m_axis[Taxis] = Vector3f(0,1,0);
				}
			}
			else
			{
				if (fabs(ray._vDir.x)>fabs(ray._vDir.z))
				{
					m_axis[Laxis] = Vector3f(0,1,0);
					m_axis[Saxis] = Vector3f(1,0,0);
					m_axis[Taxis] = Vector3f(0,0,1);
				}
				else
				{
					m_axis[Taxis] = Vector3f(1,0,0);
					if (fabs(ray._vDir.y)>fabs(ray._vDir.z))
					{
						m_axis[Laxis] = Vector3f(0,1,0);
						m_axis[Saxis] = Vector3f(0,0,1);
					}
					else
					{
						m_axis[Laxis] = Vector3f(0,0,1);
						m_axis[Saxis] = Vector3f(0,1,0);
					}
				}
			}

			Vector3f t_vector1 = ray._vOrign;
			Vector3f t_vector2 = ray._vOrign + 10* ray._vDir;
				
			// LS,LT
			float x1 = t_vector1.Dot(m_axis[Laxis]);
			float x2 = t_vector2.Dot(m_axis[Laxis]);

			float y1 = t_vector1.Dot(m_axis[Saxis]);
			float y2 = t_vector2.Dot(m_axis[Saxis]);

			m_intercept[Laxis][Saxis] = (y1*x2-y2*x1)/(x2-x1);
			m_increment[Laxis][Saxis] = (y2-y1)/(x2-x1);

			y1 = t_vector1.Dot(m_axis[Taxis]);
			y2 = t_vector2.Dot(m_axis[Taxis]);

			m_intercept[Laxis][Taxis] = (y1*x2-y2*x1)/(x2-x1);
			m_increment[Laxis][Taxis] = (y2-y1)/(x2-x1);
				
			// SL,ST
			x1 = t_vector1.Dot(m_axis[Saxis]);
			x2 = t_vector2.Dot(m_axis[Saxis]);

			y1 = t_vector1.Dot(m_axis[Laxis]);
			y2 = t_vector2.Dot(m_axis[Laxis]);

			m_intercept[Saxis][Laxis] = (y1*x2-y2*x1)/(x2-x1);
			m_increment[Saxis][Laxis] = (y2-y1)/(x2-x1);

			y1 = t_vector1.Dot(m_axis[Taxis]);
			y2 = t_vector2.Dot(m_axis[Taxis]);

			m_intercept[Saxis][Taxis] = (y1*x2-y2*x1)/(x2-x1);
			m_increment[Saxis][Taxis] = (y2-y1)/(x2-x1);

			// TL,TS
			x1 = t_vector1.Dot(m_axis[Taxis]);
			x2 = t_vector2.Dot(m_axis[Taxis]);

			y1 = t_vector1.Dot(m_axis[Laxis]);
			y2 = t_vector2.Dot(m_axis[Laxis]);

			m_intercept[Taxis][Laxis] = (y1*x2-y2*x1)/(x2-x1);
			m_increment[Taxis][Laxis] = (y2-y1)/(x2-x1);

			y1 = t_vector1.Dot(m_axis[Saxis]);
			y2 = t_vector2.Dot(m_axis[Saxis]);

			m_intercept[Taxis][Saxis] = (y1*x2-y2*x1)/(x2-x1);
			m_increment[Taxis][Saxis] = (y2-y1)/(x2-x1);

		};
	};
}
using namespace Math;