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

		// configs
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
			/*Matrix rotation{Matrix::CreateRotationY(totalYaw * TO_RADIANS)};
			forward = rotation.GetAxisZ();
			right = rotation.GetAxisX();
			up = rotation.GetAxisY();

			cameraToWorld = {
				Vector4{right, 0},
				Vector4{up, 0},
				Vector4{forward, 0},
				Vector4{origin, 1},
			};

			return cameraToWorld;*/

			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

			cameraToWorld = {
				Vector4{right, 0},
				Vector4{up, 0},
				Vector4{forward, 0},
				Vector4{origin, 1},
			};

			return cameraToWorld;

			/*const Vector3 rightV = Vector3::Cross(Vector3::UnitY, forward);
			const Vector3 upV = Vector3::Cross(forward, right);

			cameraToWorld = {
				Vector4{rightV.x, rightV.y, rightV.z, 0},
				Vector4{upV.x, upV.y, upV.z, 0},
				Vector4{forward.x, forward.y, forward.z, 0},
				Vector4{origin, 1},
			};

			return cameraToWorld;*/
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			if (pKeyboardState[SDL_SCANCODE_O])
			{
				fovAngle -= degreesPerSecond * deltaTime;

				if (fovAngle <= 0.f)
				{
					fovAngle = 0.f;
				}

				fov = tanf(fovAngle * TO_RADIANS / 2.f);
			}
			else if (pKeyboardState[SDL_SCANCODE_P])
			{
				fovAngle += degreesPerSecond * deltaTime;

				if (fovAngle >= 140.f)
				{
					fovAngle = 140.f;
				}

				fov = tanf(fovAngle * TO_RADIANS / 2.f);
			}

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

			// Debug
			if (pKeyboardState[SDL_SCANCODE_F])
			{
				std::cout << "FOV: " << fovAngle << "\n";
			}

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if ((mouseState & SDL_BUTTON_RMASK) != 0)
			{
				if (!(mouseState & SDL_BUTTON_LMASK) != 0)
				{
					//rotate yaw
					totalYaw += mouseX;
					//rotate pitch
					totalPitch += mouseY;

					Matrix rotation{ Matrix::CreateRotationX(totalPitch * TO_RADIANS) * Matrix::CreateRotationY(totalYaw * TO_RADIANS) };
					forward = rotation.TransformVector(Vector3::UnitZ).Normalized();
				}
			}
		}
	};
}
