#pragma once

#include "glm/glm.hpp"

namespace Math
{
	struct Plane
	{
		glm::vec3 Point;
		glm::vec3 Normal;
	};

	struct BSData
	{
		BSData() : Center(0.0f), Radius(-1.0f) {}
		BSData(glm::vec3 c, float r) : Center(c), Radius(r) {}
		glm::vec3 Center;
		float Radius;
	};

	struct AABBData
	{
		AABBData() : Min(0.0f), Max(0.0f) {}
		glm::vec3 Min;
		glm::vec3 Max;
	};

	struct IntersectionResult
	{
		enum T
		{
			Inside,		// Fully inside
			Outside,	// Fully outside
			Touching,	// Partly in/out
		};
	};

	// Construct the 6 planes of the view frustum.
	// Planes in view space looking at positive z.
	// The normals point inside the frustum.
	// Top,Right,Bot,Left,Far,Near
	void ExtractPlanesFromProjection(Plane* planes, float aspect, float fovY, float nearDist, float farDist);

	IntersectionResult::T PlaneSphereIntersection(const Plane& plane, const BSData& sphere);
}