#pragma once

#include <cstdint>

#include "Vector3.h"
#include "Camera.h"
#include <vector>
#include "DataTypes.h"
#include "Material.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		bool SaveBufferToImage() const;
		void CycleLightingMode();
		void ToggleShadows() { m_CanRenderShadow = !m_CanRenderShadow; }
		void PerPixel(Scene* pScene, uint32_t pixelIndex, float fov, float as, const Camera& camera, const std::vector<Light>& lights, const std::vector<Material*>& materials) const;

	private:
		enum class LightingMode
		{
			ObservedArea,
			Radiance,
			BRDF,
			Combined
		};

		LightingMode m_CurrentLightingMode{ LightingMode::Combined };

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		bool m_CanRenderShadow{ true };

		int m_Width{};
		int m_Height{};
		int m_HitCounter{};
	};
}
