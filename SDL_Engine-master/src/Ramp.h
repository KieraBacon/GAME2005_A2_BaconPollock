#pragma once
#ifndef __RAMP__
#define __RAMP__

#include "Sprite.h"
#include "glm/vec4.hpp"

class Ramp final : public Sprite
{
public:
	Ramp();
	~Ramp();

	// Life Cycle Functions
	virtual void draw() override;
	virtual void update() override;
	virtual void clean() override;

public:
	glm::vec4 slopeColour;
	glm::vec4 boundColour;
	glm::vec4 fillColour;
	glm::vec2 startPosition;
	glm::vec2 endPosition;
	float length;
	glm::vec2 slope;
	float angle;
	float coefficientOfFriction;

	void calcPositions();
	void calcTrig();
	void clampPositions();
	void setStartOnTop();
	void swapPositions();

	bool painted[2];
	unsigned char painting;
	int paintStyle;
	bool shapeChanged;
	
private:
	bool clampBounds(glm::vec2& position, const glm::vec2& slope, const glm::vec4& screenBounds);
	SDL_Texture* m_pfillTexture;
};

#endif /* defined (__RAMP__) */