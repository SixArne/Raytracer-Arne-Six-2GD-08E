#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			hitRecord.didHit = false;

			// We calculate the Camera ray.
			const Vector3 cameraRayUnit{ ray.direction };

			// We calculate the vector between the sphere and camera (defaulted at 0,0,0)
			const Vector3 cameraSphere{sphere.origin - ray.origin };

			// We get the length of projecting the sphere center on our ray
			const float projectedSphereCenterLength{Vector3::Dot(cameraSphere, cameraRayUnit)};

			// We get the point of our projection
			const Vector3 projectedSphereCenterPoint{cameraRayUnit * projectedSphereCenterLength};

			// If our projected length is bigger than the sphere we can conclude it didn't hit.
			if ((projectedSphereCenterPoint - cameraSphere).Magnitude() > sphere.radius)
			{
				hitRecord.didHit = false;
				return false;
			}

			// We calculate the distance from the circle center to the projected center on the ray
			// through pythagorean theorem
			const float projectedCircleCenterToCircleCenterLength{sqrtf(Square(cameraSphere.Magnitude()) - Square(projectedSphereCenterLength))};

			// We calculate the difference between the hit and the projected center.
			const float sphereBorderToSphereCenterLength{sqrtf(Square(sphere.radius) - Square(projectedCircleCenterToCircleCenterLength))};

			// We get the distance from the camera to the hit.
			const float hitCircleLength{ projectedSphereCenterLength - sphereBorderToSphereCenterLength };

			// We get our hit position by multiplying the unit ray vector with the length.
			const Vector3 hitPosition{ cameraRayUnit * hitCircleLength + ray.origin };

			hitRecord.didHit = true;
			hitRecord.t = hitCircleLength;
			hitRecord.origin = hitPosition;
			hitRecord.normal = hitPosition.Normalized();
			hitRecord.materialIndex = sphere.materialIndex;

			return hitRecord.didHit;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			const float t = Vector3::Dot(Vector3{ ray.origin, plane.origin }, plane.normal) / Vector3::Dot(ray.direction, plane.normal);

			if (t >= ray.min && t <= ray.max)
			{
				hitRecord.didHit = true;
				hitRecord.t = t;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.normal = plane.normal;
				hitRecord.origin = ray.origin + t * ray.direction;
				return true;
			}

			hitRecord.didHit = false;
			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			//todo W3
			assert(false && "No Implemented Yet!");
			return {};
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3
			assert(false && "No Implemented Yet!");
			return {};
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}