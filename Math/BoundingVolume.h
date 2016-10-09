
#pragma once

namespace Math
{
	class BoundingVolume
	{
	public:
		enum VolumeType
		{
			VT_AABB,
			VT_OBB,
			VT_SPHERE
		};
		BoundingVolume() {};
		virtual ~BoundingVolume() {};
		virtual void Add( BoundingVolume* pBV ) = 0;
		virtual BoundingVolume::VolumeType GetVolumeType() { return _type; }
        virtual void Empty() = 0;

	protected:
		BoundingVolume( BoundingVolume::VolumeType type ) : _type(type) {};
		BoundingVolume::VolumeType _type;
	};
}
using namespace Math;