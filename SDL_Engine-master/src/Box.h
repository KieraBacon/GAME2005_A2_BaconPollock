#pragma once
#ifndef __SLIDING_BOX__
#define __SLIDING_BOX__
#include "Sprite.h"
#include "FreeBody.h"

class Box : public Sprite
{
public:
	Box();
	~Box();

	// Life Cycle Methods
	virtual void draw() override;
	virtual void update() override;
	virtual void clean() override;

	virtual float getAngle();
	virtual FreeBody& getFreeBody();
	virtual float getScale();
	virtual void setScale(float scale);
	virtual float getSceneScale();
	virtual void setSceneScale(float scale);
public:
	bool runningSim;
	bool maintainSpeedOnAngleChange;
	unsigned char alpha;
	float SceneScale;
private:
	virtual void m_checkCollision();
	virtual void m_checkBounds();
	virtual void m_move();

	FreeBody m_FreeBody;
	float m_fScale;
};

#endif /* defined (__SLIDING_BOX__) */