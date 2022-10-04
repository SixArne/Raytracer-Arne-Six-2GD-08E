#pragma once

#include <cstdint>

#include "Vector3.h"

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

	private:
		enum class LightingMode
		{
			ObservedArea,
			Radiance,
			BRDF,
			Combined
		};

		LightingMode m_CurrentLightingMode{LightingMode::Combined};

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		Vector3 GetRayDirection(int px, int py, float ar, float fov) const;

		bool m_CanRenderShadow{};

		int m_Width{};
		int m_Height{};
	};
}
