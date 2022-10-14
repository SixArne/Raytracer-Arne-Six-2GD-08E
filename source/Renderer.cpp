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

using namespace dae;

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
	const Camera& camera = pScene->GetCamera();

	// Light ray constants
	constexpr float minLightRay{ 0.001f };

	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const float ar{ (static_cast<float>(m_Width) / static_cast<float>(m_Height)) };

	for (int px{}; px < m_Width; ++px)
	{
		const float cx = ((2 * (static_cast<float>(px) + 0.5f)) / static_cast<float>(m_Width) - 1) * ar * camera.fov;

		for (int py{}; py < m_Height; ++py)
		{
			const float cy = (1 - (2 * (static_cast<float>(py) + 0.5f)) / static_cast<float>(m_Height)) * camera.fov;

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
				const Vector3 displacedHitOrigin = closestHit.origin + closestHit.normal * minLightRay;

				for (const Light& light : lights)
				{
					// Hard shadow calculations
					Vector3 directionToLight = LightUtils::GetDirectionToLight(light, displacedHitOrigin);
					const float distance = directionToLight.Normalize();
					Ray invLightRay = Ray{ displacedHitOrigin, directionToLight, minLightRay, distance };

					// If a shadow needs to be rendered it skips it
					if (pScene->DoesHit(invLightRay) && m_CanRenderShadow)
						continue;

					// for every light
					ColorRGB BRDFrgb = materials[closestHit.materialIndex]->Shade(closestHit, directionToLight, -rayDirection);

					// Lambert shading
					const float lambertCosine = Vector3::Dot(closestHit.normal, (directionToLight));

					if (lambertCosine <= 0)
						continue;

					switch (m_CurrentLightingMode)
					{
					case LightingMode::ObservedArea:
						finalColor += ColorRGB{1.f,1.f,1.f} * lambertCosine;
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
	}

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
