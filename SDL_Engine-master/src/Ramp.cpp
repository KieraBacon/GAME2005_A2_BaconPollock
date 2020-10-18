#include "Ramp.h"
#include "Util.h"
#include "glm/trigonometric.hpp"

Ramp::Ramp()
{
	colour = { 0.0f, 0.0f, 0.0f, 1.0f };
	getTransform()->position = glm::vec2(0.0f, 0.0f);
	getRigidBody()->velocity = glm::vec2(0.0f, 0.0f);
	getRigidBody()->acceleration = glm::vec2(0.0f, 0.0f);
	getRigidBody()->isColliding = false;
	setWidth(50);
	setType(TARGET);

	painting = 0;
	paintStyle = 0;
}

Ramp::~Ramp() = default;

void Ramp::draw()
{
	Util::DrawLine(startPosition, endPosition, colour);
}

void Ramp::update()
{
}

void Ramp::clean()
{
}

void Ramp::calcPositions()
{
	slope = { glm::cos(glm::radians(-angle)), glm::sin(glm::radians(-angle)) };
	startPosition = endPosition = getTransform()->position;
	startPosition -= slope * (length * 0.5f);
	endPosition += slope * (length * 0.5f);

	clampPositions();
}

void Ramp::calcTrig()
{
	length = Util::distance(startPosition, endPosition);
	slope = Util::normalize(glm::vec2((endPosition.x - startPosition.x), (endPosition.y - startPosition.y)));
	angle = glm::degrees(glm::atan(-slope.y / slope.x));

	getTransform()->position.x = startPosition.x + (endPosition.x - startPosition.x) * 0.5f;
	getTransform()->position.y = startPosition.y + (endPosition.y - startPosition.y) * 0.5f;
}

void Ramp::clampPositions()
{
	glm::vec4 screen = { 0.0f, 0.0f, 800.0f, 600.0f };
	if(clampBounds(startPosition, slope, screen) &&
		clampBounds(endPosition, slope, screen))
		calcTrig();
}

bool Ramp::clampBounds(glm::vec2 & position, const glm::vec2 & slope, const glm::vec4 & screenBounds)
{
	// Currently works correctly only in the upper left corner.

	bool ret = false;
	if(position.x < screenBounds.x) // Horizontal minimum bound
	{
		position += glm::vec2((screenBounds.x - position.x), slope.y * ((screenBounds.x - position.x) / slope.x));
		ret = true;
	}
	if(position.x > screenBounds.z) // Horizontal maximum bound
	{
		position += glm::vec2((screenBounds.z - position.x), slope.y * ((screenBounds.z - position.x) / slope.x));
		ret = true;
	}
	if(position.y < screenBounds.y) // Vertical minimum bound
	{
		position += glm::vec2(slope.x * ((screenBounds.y - position.y) / slope.y), (screenBounds.y - position.y));
		ret = true;
	}
	if(position.y > screenBounds.w) // Vertical maximum bound
	{
		position += glm::vec2(slope.x * ((position.y - screenBounds.w) / slope.y), (position.y - screenBounds.w));
		ret = true;
	}
	return ret;
}