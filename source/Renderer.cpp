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

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			// Calculate the hitRay from the camera
			Vector3 rayDirection{GetRayDirection(px, py, ar)};
			Ray hitRay{ camera.origin, rayDirection };

			// Color to write to the color buffer
			ColorRGB finalColor{};

			// HitRecord containing info about hit
			HitRecord closestHit{};

			pScene->GetClosestHit(hitRay, closestHit); 

			if (closestHit.didHit)
			{
				const float scaled_t = 1.f- (closestHit.t / 175.f);
				finalColor = materials[closestHit.materialIndex]->Shade() * ColorRGB{ scaled_t, scaled_t, scaled_t };
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

Vector3 Renderer::GetRayDirection(int px, int py, float ar) const
{
	float cx{
		(2 * (static_cast<float>(px) + 0.5f) / static_cast<float>(m_Width) - 1.f) * ar
	};

	float cy{
		(1.f - (2 * static_cast<float>(py)) / static_cast<float>(m_Height))
	};

	Vector3 rayDirection{ cx, cy, 1.f };
	rayDirection.Normalize();

	return rayDirection;
}
