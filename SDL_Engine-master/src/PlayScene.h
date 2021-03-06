#pragma once
#ifndef __PLAY_SCENE__
#define __PLAY_SCENE__

#include "Scene.h"
#include "Plane.h"
#include "Player.h"
#include "Button.h"
#include "Label.h"
#include "Ramp.h"
#include "Box.h"
#include "FreeBody.h"
#include <glm/vec2.hpp>

class PlayScene : public Scene
{
public:
	PlayScene();
	~PlayScene();

	// Scene LifeCycle Functions
	virtual void draw() override;
	virtual void update() override;
	virtual void clean() override;
	virtual void handleEvents() override;
	virtual void start() override;
	void setBoxToHigherPosition() const;
	void setSecondRampToLowerPosition() const;
private:
	void reangleBoxVelocity(const Ramp& from, const Ramp& to) const;
	void checkCollisions();
	void updateLabels();
	
	// IMGUI Function
	void GUI_Function();
	std::string m_guiTitle;
	
	glm::vec2 m_mousePosition;

	Ramp* m_pFirstRamp;
	Ramp* m_pSecondRamp;
	Plane* m_pPlaneSprite;
	Player* m_pPlayer;
	Box* m_pBox;
	bool m_bCheckCollisionOfWholeBox;
	FreeBody* m_pFreeBody;
	glm::vec2 gravity;
	bool m_playerFacingRight;
	bool draggingMouse;
	

	// UI Items
	Button* m_pBackButton;
	Button* m_pNextButton;
	Label* m_pInstructionsLabel;
	Label* m_pDistanceLabel;
	Label* m_pTimeLabel;
	Label* m_pScreenRatioLabel;

};

#endif /* defined (__PLAY_SCENE__) */