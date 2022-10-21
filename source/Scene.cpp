#include "Scene.h"
#include "Utils.h"
#include "Material.h"

namespace dae {

#pragma region Base Scene
	//Initialize Scene with Default Solid Color Material (RED)
	Scene::Scene():
		m_Materials({ new Material_SolidColor({1,0,0})})
	{
		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_Lights.reserve(32);
	}

	Scene::~Scene()
	{
		for(auto& pMaterial : m_Materials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}

		m_Materials.clear();
	}

	void dae::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit) const
	{
		const std::vector<Sphere>& spheres = GetSphereGeometries();
		const std::vector<Plane>& planes = GetPlaneGeometries();

		// We will pass record into the test functions.
		HitRecord record{};

		for (auto& sphere: spheres)
		{
			const bool hasHit = GeometryUtils::HitTest_Sphere(sphere, ray, record);

			// Only update the closest if a new result is closer than the previous one.
			// This will ensure only the closest one is kept.
			if (hasHit && record.t < closestHit.t)
			{
				closestHit = record;
			}
		}

		for (auto& plane: planes)
		{
			const bool hasHit = GeometryUtils::HitTest_Plane(plane, ray, record);

			if (hasHit && record.t < closestHit.t)
			{
				closestHit = record;
			}
		}

		for (auto& triangle : m_TriangleMeshGeometries)
		{
			const bool hasHit = GeometryUtils::HitTest_TriangleMesh(triangle, ray, record);

			if (hasHit && record.t < closestHit.t)
			{
				closestHit = record;
			}
		}
	}

	bool Scene::DoesHit(const Ray& ray) const
	{
		const std::vector<Sphere>& spheres = GetSphereGeometries();
		const std::vector<Plane>& planes = GetPlaneGeometries();

		for (auto& sphere : spheres)
		{
			const bool hasHit = GeometryUtils::HitTest_Sphere(sphere, ray);

			// Only update the closest if a new result is closer than the previous one.
			// This will ensure only the closest one is kept.
			if (hasHit)
			{
				return true;
			}
		}

		for (auto& plane : planes)
		{
			const bool hasHit = GeometryUtils::HitTest_Plane(plane, ray);

			if (hasHit)
			{
				return true;
			}
		}

		for (auto& triangle : m_TriangleMeshGeometries)
		{
			const bool hasHit = GeometryUtils::HitTest_TriangleMesh(triangle, ray);

			if (hasHit)
			{
				return true;
			}
		}

		return false;
	}

#pragma region Scene Helpers
	Sphere* Scene::AddSphere(const Vector3& origin, float radius, unsigned char materialIndex)
	{
		Sphere s;
		s.origin = origin;
		s.radius = radius;
		s.materialIndex = materialIndex;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex)
	{
		Plane p;
		p.origin = origin;
		p.normal = normal;
		p.materialIndex = materialIndex;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	unsigned char Scene::AddMaterial(Material* pMaterial)
	{
		m_Materials.push_back(pMaterial);
		return static_cast<unsigned char>(m_Materials.size() - 1);
	}
#pragma endregion
#pragma endregion

#pragma region SCENE W1
	void Scene_W1::Initialize()
	{
				//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Spheres
		AddSphere({ 25.f, 0.f, 100.f }, 50.f, matId_Solid_Blue);
		AddSphere({ -25.f, 0.f, 100.f }, 50.f, matId_Solid_Red);

		//Plane
		AddPlane({ -75.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 75.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, -75.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 75.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 125.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);
	}
#pragma endregion

#pragma region SCENE W2
	void Scene_W2::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.0f;

		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Plane
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);

		AddPointLight({ 0.f, 5.f, -5.f }, 70.f, colors::White);
	}

#pragma endregion
#pragma region SCENE W3
	void Scene_W3::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.0f;

		const unsigned char matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({.972f, .960f, .915f}, 1.f, 1.f));
		const unsigned char matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .6f));
		const unsigned char matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));
		const unsigned char matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, 1.f));
		const unsigned char matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, .6f));
		const unsigned char matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, .57f, .57f }, 1.f));
		const auto matLamber_GrayRedPhong = AddMaterial(new Material_LambertPhong{ colors::Red, 0.5f, 0.5f, 50.f });
		const auto matLamber_GrayBluePhong = AddMaterial(new Material_LambertPhong{ colors::Blue, 0.5f, 0.5f, 50.f });
		const auto matLamber_GrayYellowPhong = AddMaterial(new Material_LambertPhong{ colors::Yellow, 0.5f, 0.5f, 50.f });

		//Plane
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f,-1.f }, matLambert_GrayBlue);

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);

		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

		AddSphere({ -1.75f, 5.f, 0.f }, .75f, matLamber_GrayYellowPhong);
		AddSphere({ 0.f, 5.f, 0.f }, .75f, matLamber_GrayRedPhong);
		AddSphere({ 1.75f, 5.f, 0.f }, .75f, matLamber_GrayBluePhong);

		AddPointLight({ 0.f, 5.f, 5.f }, 50.f, ColorRGB{1.f, .61f, .45f});
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{1.f, .8f, .45f});
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{.34f, .47f, .68f});
	}
#pragma endregion


	void Scene_W4_ReferenceScene::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.0f;

		const unsigned char matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 1.f));
		const unsigned char matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .6f));
		const unsigned char matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));
		const unsigned char matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, 1.f));
		const unsigned char matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, .6f));
		const unsigned char matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, .57f, .57f }, 1.f));
		const auto matLamber_GrayRedPhong = AddMaterial(new Material_LambertPhong{ colors::Red, 0.5f, 0.5f, 50.f });
		const auto matLamber_GrayBluePhong = AddMaterial(new Material_LambertPhong{ colors::Blue, 0.5f, 0.5f, 50.f });
		const auto matLamber_GrayYellowPhong = AddMaterial(new Material_LambertPhong{ colors::Yellow, 0.5f, 0.5f, 50.f });\

		const auto matLambert_White = AddMaterial(new Material_Lambert{ colors::White, 1.f });

		//Plane
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f,-1.f }, matLambert_GrayBlue);

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);

		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

		// CW winding order
		const Triangle baseTriangle = { Vector3(-.75f, 1.5f, 0.f), Vector3(.75f, 0.f, 0.f), Vector3(-.75f, 0.f, 0.f) };
		
		m_pMeshes[0] = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		m_pMeshes[0]->AppendTriangle(baseTriangle, true);
		m_pMeshes[0]->Translate({ -1.75f, 4.5f, 0.f });
		m_pMeshes[0]->UpdateTransforms();

		m_pMeshes[1] = AddTriangleMesh(TriangleCullMode::FrontFaceCulling, matLambert_White);
		m_pMeshes[1]->AppendTriangle(baseTriangle, true);
		m_pMeshes[1]->Translate({ 0.f, 4.5f, 0.f });
		m_pMeshes[1]->UpdateTransforms();

		m_pMeshes[2] = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		m_pMeshes[2]->AppendTriangle(baseTriangle, true);
		m_pMeshes[2]->Translate({ 1.75f, 4.5f, 0.f });
		m_pMeshes[2]->UpdateTransforms();

		AddPointLight({ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f });
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f });
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });
	}

	void Scene_W4_ReferenceScene::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);
		
		const float yawAngle = (cos(pTimer->GetTotal() + 1.f) / 2.f * PI_DIV_2);
		for (const auto m : m_pMeshes)
		{
			m->RotateY(yawAngle);
			m->UpdateTransforms();
		}
	}

	void Scene_W4_BunnyScene::Initialize()
	{
		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.0f;

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, 0.57f, 0.57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		//Plane
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f,-1.f }, matLambert_GrayBlue);

		pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		Utils::ParseOBJ("Resources/lowpoly_bunny.obj",
			pMesh->positions,
			pMesh->normals,
			pMesh->indices);

		pMesh->UpdateTransforms();
		pMesh->Scale({ 1.f, 1.f, 1.f });

		AddPointLight({ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f });
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f });
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });
	}

	void Scene_W4_BunnyScene::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);

		pMesh->RotateY(PI_DIV_2 * pTimer->GetTotal());
		pMesh->UpdateTransforms();
	}
}




