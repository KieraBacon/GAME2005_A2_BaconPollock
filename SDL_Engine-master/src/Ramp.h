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
	glm::vec4 colour;
	glm::vec2 startPosition;
	glm::vec2 endPosition;
	float length;
	glm::vec2 slope;
	float angle;

	void calcPositions();
	void calcTrig();
	void clampPositions();
	void setStartOnTop();

	bool painted[2];
	unsigned char painting;
	int paintStyle;
private:
	bool clampBounds(glm::vec2& position, const glm::vec2& slope, const glm::vec4& screenBounds);
};

#endif /* defined (__RAMP__) */