//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"

#include <iostream>

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
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const float ar{ (static_cast<float>(m_Width) / static_cast<float>(m_Height)) };
	camera.CalculateCameraToWorld();

	for (int px{}; px < m_Width; ++px)
	{
		const float cx = ((2 * (px + 0.5f)) / (float)m_Width - 1) * ar * camera.fov;

		for (int py{}; py < m_Height; ++py)
		{
			const float cy = (1 - (2 * (py + 0.5f)) / (float)m_Height) * camera.fov;

			const Vector3 rayDirection = camera.cameraToWorld.TransformVector(Vector3(cx, cy, 1.f)).Normalized();
			const Ray hitRay = Ray{ camera.origin, rayDirection };


			// Color to write to the color buffer
			ColorRGB finalColor{};

			// HitRecord containing info about hit
			HitRecord closestHit{};

			pScene->GetClosestHit(hitRay, closestHit);

			if (closestHit.didHit)
			{
				finalColor = materials[closestHit.materialIndex]->Shade();
				constexpr float minLightRay{ 0.0001f };
				constexpr float shadowMultiplier{0.5f};

				const Vector3 displacedHitOrigin = closestHit.origin + closestHit.normal * minLightRay;

				for (const Light& light : lights)
				{
					Vector3 directionToLight = LightUtils::GetDirectionToLight(light, displacedHitOrigin);
					const float distance = directionToLight.Normalize();
					const Ray invLightRay = Ray{ displacedHitOrigin, directionToLight, minLightRay, distance};

					if (pScene->DoesHit(invLightRay))
					{
						finalColor *= shadowMultiplier;
					}
				}
			}

			//Update Color in Buffer
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
