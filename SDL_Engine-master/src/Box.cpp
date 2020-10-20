#include "Box.h"
#include "TextureManager.h"
#include <glm/trigonometric.hpp>

/// <summary>
/// Credit due to rubberduck at https://opengameart.org/node/19027 for the artwork, used here under the public domain license CC0.
/// </summary>
Box::Box()
{
	maintainSpeedOnAngleChange = true;
	runningSim = false;
	alpha = 255;

	TextureManager::Instance()->load("../Assets/textures/LootCrate_48.png", "lootcrate");
	setScale(1.0f);
	getRigidBody()->mass = 1.0f;

	getTransform()->position = glm::vec2(100.0f, 100.0f);
	getRigidBody()->velocity = glm::vec2(0, 0);
	getRigidBody()->isColliding = false;

	setType(TARGET);
	SceneScale = 1.0f;
}

Box::~Box()
{
}

void Box::draw()
{
	// alias for x and y
	const auto x = getTransform()->position.x;
	const auto y = getTransform()->position.y;

	// draw the target
	TextureManager::Instance()->draw("lootcrate", x, y, getAngle(), alpha, true, SDL_FLIP_NONE, m_fScale, m_fScale);
	
	m_FreeBody.draw();
}

void Box::update()
{
	m_checkCollision();
	m_checkBounds();
	m_move();
}

void Box::clean()
{
	m_FreeBody.clean();
}

float Box::getAngle()
{
	return glm::degrees(glm::atan(getTransform()->rotation.y / getTransform()->rotation.x));
}

FreeBody& Box::getFreeBody()
{
	return m_FreeBody;
}

float Box::getScale()
{
	return m_fScale;
}

void Box::setScale(float scale)
{
	m_fScale = scale;
	const auto size = TextureManager::Instance()->getTextureSize("lootcrate");
	setWidth(size.x * m_fScale);
	setHeight(size.y * m_fScale);
}

void Box::m_checkCollision()
{
}

void Box::m_checkBounds()
{
}

void Box::m_move()
{
	float deltaTime = 1.0f / 60.0f;
	m_FreeBody.update();
	if(runningSim)
	{
		getRigidBody()->acceleration = m_FreeBody.getNetForce() / getRigidBody()->mass;
		getRigidBody()->velocity += getRigidBody()->acceleration * deltaTime;
		getTransform()->position += getRigidBody()->velocity * deltaTime * SceneScale;
	}
	m_FreeBody.getTransform()->position = getTransform()->position;
}