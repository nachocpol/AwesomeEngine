#include "Math.h"

using namespace Math;

void Math::ExtractPlanesFromProjection(Plane* planes, float aspect, float fovY, float nearDist, float farDist)
{
	float halfVFov = glm::radians(fovY * 0.5f);
	float tanHalfVFov = glm::tan(halfVFov);

	// Near plane
	float halfHeightNear = tanHalfVFov * nearDist;
	float halfWidthNear = aspect * halfHeightNear;
	glm::vec3 TRNear = glm::vec3(  halfWidthNear,  halfHeightNear, nearDist);
	glm::vec3 BRNear = glm::vec3(  halfWidthNear, -halfHeightNear, nearDist);
	glm::vec3 BLNear = glm::vec3( -halfWidthNear, -halfHeightNear, nearDist);
	glm::vec3 TLNear = glm::vec3( -halfWidthNear,  halfHeightNear, nearDist);

	// Far plane
	float halfHeightFar = tanHalfVFov * farDist;
	float halfWidthFar = aspect * halfHeightFar;
	glm::vec3 TRFar = glm::vec3(  halfWidthFar,  halfHeightFar, farDist);
	glm::vec3 BRFar = glm::vec3(  halfWidthFar, -halfHeightFar, farDist);
	glm::vec3 BLFar = glm::vec3( -halfWidthFar, -halfHeightFar, farDist);
	glm::vec3 TLFar = glm::vec3( -halfWidthFar,  halfHeightFar, farDist);

	// Remember, we use a left handed coordinate system (+y is up)

	// Top plane
	planes[0].Point = TRNear;
	planes[0].Normal = glm::normalize(glm::cross(TRFar - TRNear, TLNear - TRNear));

	// Right plane
	planes[1].Point = TRNear;
	planes[1].Normal = glm::normalize(glm::cross(BRNear - TRNear, TRFar - TRNear));

	// Bot plane
	planes[2].Point = BRNear;
	planes[2].Normal = glm::normalize(glm::cross(BLNear - BRNear, BRFar - BRNear));

	// Left plane
	planes[3].Point = BLNear;
	planes[3].Normal = glm::normalize(glm::cross(TLNear - BLNear, BLFar - BLNear));

	// Far plane
	planes[4].Point = glm::vec3(0.0f, 0.0f, farDist);
	planes[4].Normal = glm::vec3(0.0f, 0.0f, -1.0f);

	// Near plane
	planes[5].Point = glm::vec3(0.0f, 0.0f, nearDist);
	planes[5].Normal = glm::vec3(0.0f, 0.0f, 1.0f);
}


IntersectionResult::T Math::PlaneSphereIntersection(const Plane& plane, const BSData& sphere)
{
	glm::vec3 PC = plane.Point - sphere.Center;
	float dist = glm::dot(PC, -plane.Normal);

	if (glm::abs(dist) <= sphere.Radius)
	{
		return IntersectionResult::Touching;
	}

	float delta = dist - sphere.Radius;
	if (delta > 0.0f)
	{
		return IntersectionResult::Inside;
	}
	return IntersectionResult::Outside;
}
