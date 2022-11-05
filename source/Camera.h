#pragma once
#include <cassert>
#include <iostream>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle },
			fov{ tanf(_fovAngle * TO_RADIANS / 2.f) }
		{
		}

		Vector3 origin{};
		float fovAngle{ 90.f };
		float fov{ tanf(45.f * TO_RADIANS / 2.f) };

		// Settings
		float degreesPerSecond{ 90.f };
		float moveSpeedPerSecond{ 10.f };
		float turnSpeedPerSecond{ 10.f };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		Matrix cameraToWorld{};

		Matrix CalculateCameraToWorld()
		{
			// inverse pitch to make the pitch variable itself more reasonable
			const Matrix rotation{ Matrix::CreateRotationX(-totalPitch * TO_RADIANS) * Matrix::CreateRotationY(totalYaw * TO_RADIANS) };

			forward = rotation.TransformVector(Vector3::UnitZ).Normalized();
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

		/*	forward = rotation.GetAxisZ();
			right = rotation.GetTranslation();
			up = rotation.GetAxisY();*/

			cameraToWorld = {
				Vector4{right, 0},
				Vector4{up, 0},
				Vector4{forward, 0},
				Vector4{origin, 1},
			};

			return cameraToWorld;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			// Movement
			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += forward * moveSpeedPerSecond * deltaTime;
			}

			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= forward * moveSpeedPerSecond * deltaTime;
			}

			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= right * moveSpeedPerSecond * deltaTime;
			}

			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += right * moveSpeedPerSecond * deltaTime;
			}

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if ((mouseState & SDL_BUTTON_RMASK) != 0)
			{
				if (!(mouseState & SDL_BUTTON_LMASK) != 0)
				{
					totalYaw += mouseX;
					totalPitch = Clamp(-60.f, 30.f, totalPitch + mouseY);
					
					// Recalculate view matrix
					CalculateCameraToWorld();
				}
			}

			if ((mouseState & SDL_BUTTON_LMASK) != 0 && (mouseState & SDL_BUTTON_RMASK) != 0)
			{
				origin += up * moveSpeedPerSecond * deltaTime * mouseY;
			}
			else if ((mouseState & SDL_BUTTON_LMASK) != 0)
			{
				if (mouseY > 0)
				{
					origin -= forward * moveSpeedPerSecond * deltaTime;
				}
				else if (mouseY < 0)
				{
					origin += forward * moveSpeedPerSecond * deltaTime;
				}

				totalYaw += mouseX;
			}
		}
	};
}
