#include "Rxn.h"
#include "Camera.h"

namespace Rxn::Graphics
{
    Camera::Camera() 
        : m_position(m_initialPosition)
    {
    }

    void Camera::Init(DirectX::XMFLOAT3 position)
    {
        m_initialPosition = position;
        Reset();
    }

    void Camera::SetMoveSpeed(float32 unitsPerSecond)
    {
        m_moveSpeed = unitsPerSecond;
    }

    void Camera::SetTurnSpeed(float32 radiansPerSecond)
    {
        m_turnSpeed = radiansPerSecond;
    }

    void Camera::Reset()
    {
        m_position = m_initialPosition;
        m_yaw = DirectX::XM_PI;
        m_pitch = 0.0f;
        m_lookDirection = { 0.0f, 0.0f, -1.0f };
    }

    void Camera::Update(float64 elapsedSeconds)
    {
        // Calculate the move vector in camera space.
        DirectX::XMFLOAT3 move(0, 0, 0);

        if (m_keysPressed.a)
            move.x -= 1.0f;
        if (m_keysPressed.d)
            move.x += 1.0f;
        if (m_keysPressed.w)
            move.z -= 1.0f;
        if (m_keysPressed.s)
            move.z += 1.0f;

        if (fabs(move.x) > 0.1f && fabs(move.z) > 0.1f)
        {
            DirectX::XMVECTOR vector = DirectX::XMVector3Normalize(XMLoadFloat3(&move));
            move.x = DirectX::XMVectorGetX(vector);
            move.z = DirectX::XMVectorGetZ(vector);
        }

        float32 moveInterval = m_moveSpeed * static_cast<float32>(elapsedSeconds);
        float32 rotateInterval = m_turnSpeed * static_cast<float32>(elapsedSeconds);

        if (m_keysPressed.left)
            m_yaw += rotateInterval;
        if (m_keysPressed.right)
            m_yaw -= rotateInterval;
        if (m_keysPressed.up)
            m_pitch += rotateInterval;
        if (m_keysPressed.down)
            m_pitch -= rotateInterval;

        // Prevent looking too far up or down.
        m_pitch = std::min(m_pitch, DirectX::XM_PIDIV4);
        m_pitch = std::max(-DirectX::XM_PIDIV4, m_pitch);

        // Move the camera in model space.
        float32 x = move.x * -cosf(m_yaw) - move.z * sinf(m_yaw);
        float32 z = move.x * sinf(m_yaw) - move.z * cosf(m_yaw);
        m_position.x += x * moveInterval;
        m_position.z += z * moveInterval;

        // Determine the look direction.
        float32 r = cosf(m_pitch);
        m_lookDirection.x = r * sinf(m_yaw);
        m_lookDirection.y = sinf(m_pitch);
        m_lookDirection.z = r * cosf(m_yaw);
    }

    // TODO - find out why camera is not updating via controls. I think it's just caused by the constant buffer using the wrong view matrix.
    DirectX::XMMATRIX Camera::GetViewMatrix()
    {
        return DirectX::XMMatrixLookToRH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_lookDirection), XMLoadFloat3(&m_upDirection));
    }

    DirectX::XMMATRIX Camera::GetProjectionMatrix(float32 fov, float32 aspectRatio, float32 nearPlane, float32 farPlane)
    {
        return DirectX::XMMatrixPerspectiveFovRH(fov, aspectRatio, nearPlane, farPlane);
    }

    void Camera::OnKeyDown(WPARAM key)
    {
        switch (key)
        {
        case 'W':
            m_keysPressed.w = true;
            break;
        case 'A':
            m_keysPressed.a = true;
            break;
        case 'S':
            m_keysPressed.s = true;
            break;
        case 'D':
            m_keysPressed.d = true;
            break;
        case VK_LEFT:
            m_keysPressed.left = true;
            break;
        case VK_RIGHT:
            m_keysPressed.right = true;
            break;
        case VK_UP:
            m_keysPressed.up = true;
            break;
        case VK_DOWN:
            m_keysPressed.down = true;
            break;
        case VK_ESCAPE:
            Reset();
            break;
        default:
            break;
        }
    }

    void Camera::OnKeyUp(WPARAM key)
    {
        switch (key)
        {
        case 'W':
            m_keysPressed.w = false;
            break;
        case 'A':
            m_keysPressed.a = false;
            break;
        case 'S':
            m_keysPressed.s = false;
            break;
        case 'D':
            m_keysPressed.d = false;
            break;
        case VK_LEFT:
            m_keysPressed.left = false;
            break;
        case VK_RIGHT:
            m_keysPressed.right = false;
            break;
        case VK_UP:
            m_keysPressed.up = false;
            break;
        case VK_DOWN:
            m_keysPressed.down = false;
            break;
        default:
            break;
        }
    }
}
