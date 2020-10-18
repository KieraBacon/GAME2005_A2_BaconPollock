#pragma once
#ifndef __FREEBODY__
#define __FREEBODY__

#include "DisplayObject.h"
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "Label.h"

struct DisplayForce
{
	DisplayForce(std::string name, glm::vec2 head, const glm::vec2& origin, int size, glm::vec4& colour, float& scale, float extender, float& arrowScale, float& arrowHead);
	DisplayForce(const DisplayForce& base);
	~DisplayForce();
	void draw();

	std::string name;
	glm::vec2 head;

	const glm::vec2& origin;
	const glm::vec4& colour;
	const float& m_fScale;
	const float& m_fArrowScale;
	const float& m_fArrowHead;
	float extender;
	Label m_pLabel;
};


class FreeBody : public DisplayObject
{
public:
	FreeBody();
	virtual ~FreeBody();

	// Inherited via GameObject
	virtual void draw() override;
	virtual void update() override;
	virtual void clean() override;

	virtual void addForce(std::string name, glm::vec2 force);
	virtual void addForceComponent(std::string name, glm::vec2 forceComponent);
	virtual glm::vec2 getNetForce();
	virtual float getScale();
	virtual void setScale(float scale);
	virtual float getArrowScale();
	virtual void setArrowScale(float scale);
	virtual float getArrowAngle();
	virtual void setArrowAngle(float angle);
	virtual int getLabelSize();
	virtual void setLabelSize(int size);
	bool showForces;
	bool showComponents;
	bool showNetForce;
private:
	std::vector<DisplayForce> m_vForces;
	std::vector<DisplayForce> m_vForceComponents;
	DisplayForce* m_pNetForce;
	glm::vec4 m_ForceColour;
	glm::vec4 m_ComponentColour;
	glm::vec4 m_NetColour;
	float m_fScale;
	float m_fArrowScale;
	float m_fArrowHead;
	int m_iLabelSize;
};

#endif /* defined (__FREEBODY__) */
