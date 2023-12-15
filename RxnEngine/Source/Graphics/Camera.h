#pragma once

namespace Rxn::Graphics
{
    class Camera
    {
    public:
        Camera();

        void Init(DirectX::XMFLOAT3 position);
        void Update(float32 elapsedSeconds);
        DirectX::XMMATRIX GetViewMatrix();
        DirectX::XMMATRIX GetProjectionMatrix(float32 fov, float32 aspectRatio, float32 nearPlane = 1.0f, float32 farPlane = 1000.0f);
        void SetMoveSpeed(float32 unitsPerSecond);
        void SetTurnSpeed(float32 radiansPerSecond);

        void OnKeyDown(WPARAM key);
        void OnKeyUp(WPARAM key);

    private:
        void Reset();

        struct KeysPressed
        {
            bool w;
            bool a;
            bool s;
            bool d;

            bool left;
            bool right;
            bool up;
            bool down;
        };

        DirectX::XMFLOAT3 m_initialPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        DirectX::XMFLOAT3 m_position;
        float32 m_yaw = DirectX::XM_PI;                // Relative to the +z axis.
        float32 m_pitch = 0.0f;                // Relative to the xz plane.
        DirectX::XMFLOAT3 m_lookDirection = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);
        DirectX::XMFLOAT3 m_upDirection = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
        float32 m_moveSpeed = 20.0f;            // Speed at which the camera moves, in units per second.
        float32 m_turnSpeed = DirectX::XM_PIDIV2;            // Speed at which the camera turns, in radians per second.

        KeysPressed m_keysPressed = {};
    };
}
