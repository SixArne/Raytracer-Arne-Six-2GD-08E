#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

#define Moller

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			// We calculate the vector between the sphere and camera (defaulted at 0,0,0)
			const Vector3 rayOriginToCameraSphere{sphere.origin - ray.origin };

			// This will be our hypotenuse
			const float cameraSphereLengthSquared = rayOriginToCameraSphere.SqrMagnitude();
			// We get the length of projecting the sphere center on our ray
			const float projectedSphereCenterLength{Vector3::Dot(rayOriginToCameraSphere, ray.direction)};

			const float distanceToProjectedPointSquared = cameraSphereLengthSquared - projectedSphereCenterLength * projectedSphereCenterLength;

			// If our projected length is bigger than the sphere we can conclude it didn't hit.
			if (distanceToProjectedPointSquared > sphere.radius * sphere.radius)
			{
				return false;
			}

			const float distanceToIntersection = std::sqrtf( sphere.radius * sphere.radius - distanceToProjectedPointSquared);
			const float t = projectedSphereCenterLength - distanceToIntersection;

			if (t < ray.min || t > ray.max)
			{
				return false;
			}

			hitRecord.didHit = true;

			if (ignoreHitRecord)
				return true;
			
			hitRecord.materialIndex = sphere.materialIndex;
			hitRecord.t = t;
			hitRecord.origin = ray.origin + t * ray.direction;
			hitRecord.normal = Vector3(sphere.origin, hitRecord.origin).Normalized();
			return true;
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
				if (ignoreHitRecord)
					return true;

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
//#pragma region Old
//				float normalViewDot = Vector3::Dot(triangle.normal, ray.direction);
//
//#pragma region Culling and early exits
//				// if our ray is perpendicular to the normal it will never hit.
//				if (normalViewDot == 0)
//					return false;
//
//				// Skip calculations depending on cull mode
//				if (triangle.cullMode == TriangleCullMode::FrontFaceCulling && normalViewDot < 0)
//					return false;
//
//				// Skip calculations depending on cull mode
//				if (triangle.cullMode == TriangleCullMode::BackFaceCulling && normalViewDot > 0)
//					return false;
//
//#pragma endregion
//
//				const Vector3 inBetween = triangle.v0 + triangle.v1 + triangle.v2;
//				Vector3 center = inBetween / 3.f;
//				Vector3 L = center - ray.origin;
//				float t = Vector3::Dot(L, triangle.normal) / Vector3::Dot(ray.direction, triangle.normal);
//
//				if (t < ray.min || t > ray.max)
//					return false;
//
//				Vector3 p = ray.origin + t * ray.direction;
//
//#pragma region IsContainedWithinTriangle
//				Vector3 edgeA = triangle.v1 - triangle.v0;
//				Vector3 pointToSideA = p - triangle.v0;
//
//				if (Vector3::Dot(triangle.normal, Vector3::Cross(edgeA, pointToSideA)) < 0)
//					return false;
//
//				Vector3 edgeB = triangle.v2 - triangle.v0;
//				Vector3 pointToSideB = p - triangle.v0;
//
//				if (Vector3::Dot(triangle.normal, Vector3::Cross(pointToSideB, edgeB)) < 0)
//					return false;
//
//				Vector3 edgeC = triangle.v1 - triangle.v2;
//				Vector3 pointToSideC = p - triangle.v2;
//
//				if (Vector3::Dot(triangle.normal, Vector3::Cross(pointToSideC, edgeC)) < 0)
//					return false;
//#pragma endregion
//
//				if (ignoreHitRecord)
//					return true;
//
//				hitRecord.origin = ray.origin + (t * ray.direction);
//				hitRecord.normal = triangle.normal;
//				hitRecord.didHit = true;
//				hitRecord.materialIndex = triangle.materialIndex;
//				hitRecord.t = t;
//
//				return true;
//#pragma endregion
		
#pragma region Mine
				constexpr float EPSILON = 0.0000001f;

				float normalViewDot = Vector3::Dot(triangle.normal, ray.direction);

				// if our ray is perpendicular to the normal it will never hit.
				if (normalViewDot == 0)
					return false;

				// Skip calculations depending on cull mode
				if (triangle.cullMode == TriangleCullMode::FrontFaceCulling && normalViewDot < 0)
					return false;

				// Skip calculations depending on cull mode
				if (triangle.cullMode == TriangleCullMode::BackFaceCulling && normalViewDot > 0)
					return false;

				Vector3 vertex0 = triangle.v0;
				Vector3 vertex1 = triangle.v1;
				Vector3 vertex2 = triangle.v2;
				Vector3 edge1{}, edge2{}, h{}, s{}, q{};

				float a{}, f{}, u{}, v{};
				edge1 = vertex1 - vertex0;
				edge2 = vertex2 - vertex0;
				h = Vector3::Cross(ray.direction, edge2);
				a = Vector3::Dot(edge1, h);

				if (a > -EPSILON && a < EPSILON)
					return false; // parallel to triangle

				f = 1.0f / a;
				s = ray.origin - vertex0;
				u = f * Vector3::Dot(s, h);

				if (u < 0.0f || u > 1.0f)
					return false;

				q = Vector3::Cross(s, edge1);
				v = f * Vector3::Dot(ray.direction, q);

				if (v < 0.0f || (u + v) > 1.0f)
					return false;

				float t = f * Vector3::Dot(edge2, q);

				if (t < ray.min || t > ray.max)
					return false;

				if (t > EPSILON)
				{
					if (ignoreHitRecord)
						return true;

					hitRecord.origin = ray.origin + (t * ray.direction);
					hitRecord.normal = triangle.normal;
					hitRecord.didHit = true;
					hitRecord.materialIndex = triangle.materialIndex;
					hitRecord.t = t;

					return true;
				}
				else
					return false;
#pragma endregion
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest

		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			float tx1 = (mesh.transformedMinAABB.x - ray.origin.x) / ray.direction.x;
			float tx2 = (mesh.transformedMaxAABB.x - ray.origin.x) / ray.direction.x;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			float ty1 = (mesh.transformedMinAABB.y - ray.origin.y) / ray.direction.y;
			float ty2 = (mesh.transformedMaxAABB.y - ray.origin.y) / ray.direction.y;

			tmin = std::max(tmin, std::min(ty1, ty2));
			tmax = std::min(tmax, std::max(ty1, ty2));

			float tz1 = (mesh.transformedMinAABB.z - ray.origin.z) / ray.direction.z;
			float tz2 = (mesh.transformedMaxAABB.z - ray.origin.z) / ray.direction.z;

			tmin = std::max(tmin, std::min(tz1, tz2));
			tmax = std::min(tmax, std::max(tz1, tz2));

			return tmax > 0 && tmax >= tmin;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false, bool usingMoller = true)
		{
			if (!SlabTest_TriangleMesh(mesh, ray))
			{
				return false;
			}

			Triangle triangle{};
			int normalCounter{};
			HitRecord record{};

			if (mesh.indices.size() % 3 != 0)
				throw std::runtime_error("Mesh no multiple of 3");

			int vertexCount{};
			int normalCount{};

			for (int index{}; index < mesh.indices.size(); index++)
			{
				vertexCount++;

				if (vertexCount == 3)
				{
					triangle.normal = mesh.transformedNormals[normalCount];
					triangle.v0 = mesh.transformedPositions[mesh.indices[index - 2]];
					triangle.v1 = mesh.transformedPositions[mesh.indices[index - 1]];
					triangle.v2 = mesh.transformedPositions[mesh.indices[index]];

					triangle.cullMode = mesh.cullMode;
					triangle.materialIndex = mesh.materialIndex;

					vertexCount = 0;
					normalCount++;

					bool hasHit = HitTest_Triangle(triangle, ray, hitRecord);

					if (hasHit && ignoreHitRecord)
					{
						return true;
					}


					if (hitRecord.t < record.t && hasHit)
					{
						record = hitRecord;
					}
				}
			}

			if (record.didHit)
			{
				hitRecord = record;
				return true;
			}

			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, bool usingMoller = true)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true, usingMoller);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			Vector3 originLight = light.origin - origin;

			return originLight;
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			// Light intensity
			//ColorRGB colorRgb = light.color * (light.intensity / (light.origin - target).SqrMagnitude());
			ColorRGB colorRgb = light.color * (light.intensity / (light.origin - target).SqrMagnitude());
			return colorRgb;
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