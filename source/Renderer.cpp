//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"

#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"
#include "Camera.h"
#include <future>
#include <ppl.h>

#include <vector>
using namespace dae;

//#define ASYNC
#define PARALLEL_FOR

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	camera.CalculateCameraToWorld();

	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const float as{ (static_cast<float>(m_Width) / static_cast<float>(m_Height)) };

	const uint32_t numPixels = m_Width * m_Height;

#if defined(ASYNC)
	const uint32_t numCores = std::thread::hardware_concurrency();
	std::vector<std::future<void>> async_futures{};

	const uint32_t numPixelsPerTask = numPixels / numCores;
	uint32_t numUnassignedPixels = numPixels % numCores;
	uint32_t currPixelIndex = 0;

	for (uint32_t coreId{ 0 }; coreId < numCores; ++coreId)
	{
		uint32_t taskSize = numPixelsPerTask;

		if (numUnassignedPixels > 0)
		{
			++taskSize;
			--numUnassignedPixels;
		}

		async_futures.push_back(
			std::async(std::launch::async, [=, this]
				{
					const uint32_t pixelIndexEnd = currPixelIndex + taskSize;

					for (uint32_t pixelIndex{ currPixelIndex }; pixelIndex < pixelIndexEnd; ++pixelIndex)
					{
						PerPixel(pScene, pixelIndex, camera.fov, as, camera, lights, materials);
					}
				}
			)
		);

		currPixelIndex += taskSize;
	}

	for (const std::future<void>& f : async_futures)
	{
		f.wait();
	}
#elif defined(PARALLEL_FOR)
	concurrency::parallel_for(0u, numPixels, [=, this](int i) {
		PerPixel(pScene, i, camera.fov, as, camera, lights, materials);
	});
#else
	for (uint32_t p{}; p < numPixels; ++p)
	{
		PerPixel(pScene, p, camera.fov, as, camera, lights, materials);
	}
#endif

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightingMode()
{
	int modeId = static_cast<int>(m_CurrentLightingMode);
	m_CurrentLightingMode = static_cast<LightingMode>((++modeId) % 4);

	switch (m_CurrentLightingMode)
	{
	case LightingMode::ObservedArea:
		std::cout << "CYCLE MODE: ObservedArea" << "\n";
		break;
	case LightingMode::Radiance:
		std::cout << "CYCLE MODE: Radiance" << "\n";
		break;
	case LightingMode::BRDF:
		std::cout << "CYCLE MODE: BRDF" << "\n";
		break;
	case LightingMode::Combined:
		std::cout << "CYCLE MODE: Combined" << "\n";
		break;
	}
}

void Renderer::PerPixel(Scene* pScene, uint32_t pixelIndex, float fov, float as, const Camera& camera, const std::vector<Light>& lights, const std::vector<Material*>& materials) const
{
	// Light ray constants
	constexpr float minLightRay{ 0.001f };

	const int px = pixelIndex % m_Width;
	const int py = pixelIndex / m_Width;

	const float rx = px + 0.5f;
	const float ry = py + 0.5f;

	const float cx = ((2 * (rx)) / static_cast<float>(m_Width) - 1) * (static_cast<float>(m_Width) / static_cast<float>(m_Height)) * camera.fov;
	const float cy = (1 - (2 * (ry)) / static_cast<float>(m_Height)) * camera.fov;

	const Vector3 rayDirection = camera.cameraToWorld.TransformVector(Vector3(cx, cy, 1.f)).Normalized();
	const Ray hitRay = Ray{ camera.origin, rayDirection };

	// Color to write to buffer
	ColorRGB finalColor{};

	// HitRecord containing info about hit
	HitRecord closestHit{};

	pScene->GetClosestHit(hitRay, closestHit);
	if (closestHit.didHit)
	{
		// To shoot our inverse light ray we need to offset it a bit so we don't have self collision.
		const Vector3 offsetHitOrigin = closestHit.origin + closestHit.normal * minLightRay;

		for (const Light& light : lights)
		{
			// Hard shadow calculations
			Vector3 directionToLight = LightUtils::GetDirectionToLight(light, closestHit.origin);
			const float distanceToLight = directionToLight.Normalize();

			// Lambert shading
			const float lambertCosine = Vector3::Dot(closestHit.normal, directionToLight);

			if (lambertCosine <= 0 && m_CurrentLightingMode == LightingMode::Combined
				|| lambertCosine <= 0 && m_CurrentLightingMode == LightingMode::ObservedArea)
			{
				continue;
			}

			// If a shadow needs to be rendered it skips it
			if (m_CanRenderShadow)
			{
				Ray invLightRay = Ray{ offsetHitOrigin, LightUtils::GetDirectionToLight(light, offsetHitOrigin).Normalized(), 0.001f, distanceToLight };
				if (pScene->DoesHit(invLightRay))
				{
					continue;
				}
			}

			// for every light
			ColorRGB BRDFrgb = materials[closestHit.materialIndex]->Shade(closestHit, directionToLight, -rayDirection);
			

			switch (m_CurrentLightingMode)
			{
			case LightingMode::ObservedArea:
				finalColor += ColorRGB{ 1.f,1.f,1.f } * lambertCosine;
				break;
			case LightingMode::Radiance:
				finalColor += LightUtils::GetRadiance(light, closestHit.origin);
				break;
			case LightingMode::BRDF:
				finalColor += BRDFrgb;
				break;
			case LightingMode::Combined:
				finalColor += LightUtils::GetRadiance(light, closestHit.origin) * BRDFrgb * lambertCosine;
				break;
			}
		}
	}
	else
	{
		finalColor = ColorRGB{ 1.f, 1.f, 1.f };
	}

	// Normalizes the color to avoid overflows
	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

