#include "PlayScene.h"
#include "Game.h"
#include "EventManager.h"

// required for IMGUI
#include "imgui.h"
#include "imgui_sdl.h"
#include "Renderer.h"
#include <sstream>
#include "CollisionManager.h"
#include "Util.h"

PlayScene::PlayScene()
{
	PlayScene::start();
}

PlayScene::~PlayScene()
= default;

void PlayScene::draw()
{
	const auto renderer = Renderer::Instance()->getRenderer();
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_Rect bottomFill = { 0, m_pSecondRamp->endPosition.y, 800, 600 - m_pSecondRamp->endPosition.y };
	SDL_SetRenderDrawColor(renderer, m_pSecondRamp->boundColour.x * 255.0f, m_pSecondRamp->boundColour.y * 255.0f, m_pSecondRamp->boundColour.z * 255.0f, m_pSecondRamp->boundColour.w * 255.0f);
	SDL_RenderDrawRect(renderer, &bottomFill);
	SDL_SetRenderDrawColor(renderer, m_pSecondRamp->fillColour.x * 255.0f, m_pSecondRamp->fillColour.y * 255.0f, m_pSecondRamp->fillColour.z * 255.0f, m_pSecondRamp->fillColour.w * 255.0f);
	SDL_RenderFillRect(renderer, &bottomFill);

	if(EventManager::Instance().isIMGUIActive())
	{
		GUI_Function();
	}

	drawDisplayList();
	SDL_SetRenderDrawColor(Renderer::Instance()->getRenderer(), 255, 255, 255, 255);
}

void PlayScene::update()
{
	checkCollisions();
	updateDisplayList();
}

void PlayScene::clean()
{
	removeAllChildren();
}

void PlayScene::handleEvents()
{
	EventManager::Instance().update();
	if(m_pFirstRamp->painting > 0)
	{
		if(m_pFirstRamp->paintStyle == 0)
		{
			if(EventManager::Instance().getMouseButtonDown(0))
			{
				draggingMouse = true;
				m_pFirstRamp->startPosition = EventManager::Instance().getMousePosition();
				++m_pFirstRamp->painting;
			}
			else if(EventManager::Instance().getMouseButtonUp(0))
			{
				draggingMouse = false;
				m_pFirstRamp->painting = 0;
				m_pFirstRamp->calcTrig();
				setBoxToHigherPosition();
				setSecondRampToLowerPosition();
			}

			if(draggingMouse)
			{
				m_pFirstRamp->endPosition = EventManager::Instance().getMousePosition();
				m_pFirstRamp->calcTrig();
				setBoxToHigherPosition();
				setSecondRampToLowerPosition();
			}
		}
		else if(m_pFirstRamp->paintStyle == 1)
		{
			if(EventManager::Instance().getMouseButtonDown(0))
			{
				draggingMouse = true;
			}
			else if(EventManager::Instance().getMouseButtonUp(0))
			{
				draggingMouse = false;
				if(m_pFirstRamp->painting == 1) // Just finished painting the start position
				{
					m_pFirstRamp->painted[0] = true;
					m_pFirstRamp->painting = (m_pFirstRamp->painted[1] == true ? 0 : 2);
				}
				else if(m_pFirstRamp->painting == 2) // Just finished painting the end position
				{
					m_pFirstRamp->painted[1] = true;
					m_pFirstRamp->painting = (m_pFirstRamp->painted[0] == true ? 0 : 1);
				}
				m_pFirstRamp->calcTrig();
				setBoxToHigherPosition();
				setSecondRampToLowerPosition();
			}

			if(draggingMouse)
			{
				if(m_pFirstRamp->painting == 1)
					m_pFirstRamp->startPosition = EventManager::Instance().getMousePosition();
				else if(m_pFirstRamp->painting == 2)
					m_pFirstRamp->endPosition = EventManager::Instance().getMousePosition();

				m_pFirstRamp->calcTrig();
				setBoxToHigherPosition();
				setSecondRampToLowerPosition();
			}
		}
	}

	// handle player movement with GameController
	if(SDL_NumJoysticks() > 0)
	{
		if(EventManager::Instance().getGameController(0) != nullptr)
		{
			const auto deadZone = 10000;
			if(EventManager::Instance().getGameController(0)->LEFT_STICK_X > deadZone)
			{
				m_pPlayer->setAnimationState(PLAYER_RUN_RIGHT);
				m_playerFacingRight = true;
			}
			else if(EventManager::Instance().getGameController(0)->LEFT_STICK_X < -deadZone)
			{
				m_pPlayer->setAnimationState(PLAYER_RUN_LEFT);
				m_playerFacingRight = false;
			}
			else
			{
				if(m_playerFacingRight)
				{
					m_pPlayer->setAnimationState(PLAYER_IDLE_RIGHT);
				}
				else
				{
					m_pPlayer->setAnimationState(PLAYER_IDLE_LEFT);
				}
			}
		}
	}


	// handle player movement if no Game Controllers found
	if(SDL_NumJoysticks() < 1)
	{
		if(EventManager::Instance().isKeyDown(SDL_SCANCODE_A))
		{
			m_pPlayer->setAnimationState(PLAYER_RUN_LEFT);
			m_playerFacingRight = false;
		}
		else if(EventManager::Instance().isKeyDown(SDL_SCANCODE_D))
		{
			m_pPlayer->setAnimationState(PLAYER_RUN_RIGHT);
			m_playerFacingRight = true;
		}
		else
		{
			if(m_playerFacingRight)
			{
				m_pPlayer->setAnimationState(PLAYER_IDLE_RIGHT);
			}
			else
			{
				m_pPlayer->setAnimationState(PLAYER_IDLE_LEFT);
			}
		}
	}


	if(EventManager::Instance().isKeyDown(SDL_SCANCODE_ESCAPE))
	{
		TheGame::Instance()->quit();
	}

	if(EventManager::Instance().isKeyDown(SDL_SCANCODE_1))
	{
		TheGame::Instance()->changeSceneState(START_SCENE);
	}

	if(EventManager::Instance().isKeyDown(SDL_SCANCODE_2))
	{
		TheGame::Instance()->changeSceneState(END_SCENE);
	}
}

void PlayScene::start()
{
	draggingMouse = false;
	m_bCheckCollisionOfWholeBox = false;

	gravity = { 0.0f, 9.8f };

	// Set GUI Title
	m_guiTitle = "Play Scene";

	// First Ramp
	m_pFirstRamp = new Ramp();
	addChild(m_pFirstRamp);
	m_pFirstRamp->getTransform()->position = { 400.0f, 300.0f };
	{
		float rise = -30;
		float run = 40;
		m_pFirstRamp->startPosition = { m_pFirstRamp->getTransform()->position.x - (run * 0.5f), m_pFirstRamp->getTransform()->position.y + (rise * 0.5f) };
		m_pFirstRamp->endPosition = { m_pFirstRamp->getTransform()->position.x + (run * 0.5f), m_pFirstRamp->getTransform()->position.y - (rise * 0.5f) };
		m_pFirstRamp->calcTrig();
	}

	// Second Ramp
	m_pSecondRamp = new Ramp();
	addChild(m_pSecondRamp);
	setSecondRampToLowerPosition();

	// Box
	m_pBox = new Box();
	addChild(m_pBox);
	m_pBox->setScale(0.75f);
	m_pBox->getFreeBody().setScale(0.5f);
	m_pBox->getFreeBody().setArrowScale(10.0f);
	setBoxToHigherPosition();

	//// Back Button
	//m_pBackButton = new Button("../Assets/textures/backButton.png", "backButton", BACK_BUTTON);
	//m_pBackButton->getTransform()->position = glm::vec2(300.0f, 400.0f);
	//m_pBackButton->addEventListener(CLICK, [&]()-> void
	//{
	//	m_pBackButton->setActive(false);
	//	TheGame::Instance()->changeSceneState(START_SCENE);
	//});

	//m_pBackButton->addEventListener(MOUSE_OVER, [&]()->void
	//{
	//	m_pBackButton->setAlpha(128);
	//});

	//m_pBackButton->addEventListener(MOUSE_OUT, [&]()->void
	//{
	//	m_pBackButton->setAlpha(255);
	//});
	//addChild(m_pBackButton);

	//// Next Button
	//m_pNextButton = new Button("../Assets/textures/nextButton.png", "nextButton", NEXT_BUTTON);
	//m_pNextButton->getTransform()->position = glm::vec2(500.0f, 400.0f);
	//m_pNextButton->addEventListener(CLICK, [&]()-> void
	//{
	//	m_pNextButton->setActive(false);
	//	TheGame::Instance()->changeSceneState(END_SCENE);
	//});

	//m_pNextButton->addEventListener(MOUSE_OVER, [&]()->void
	//{
	//	m_pNextButton->setAlpha(128);
	//});

	//m_pNextButton->addEventListener(MOUSE_OUT, [&]()->void
	//{
	//	m_pNextButton->setAlpha(255);
	//});

	//addChild(m_pNextButton);

	/* Instructions Label */
	m_pInstructionsLabel = new Label("Press the backtick (`) character to toggle Debug View", "Consolas");
	m_pInstructionsLabel->getTransform()->position = glm::vec2(Config::SCREEN_WIDTH * 0.5f, 500.0f);

	addChild(m_pInstructionsLabel);
}

void PlayScene::setBoxToHigherPosition() const
{
	if(m_pFirstRamp->startPosition.y >= m_pFirstRamp->endPosition.y)
	{
		m_pBox->getTransform()->position = { m_pFirstRamp->endPosition.x + m_pBox->getWidth() * 0.0f, m_pFirstRamp->endPosition.y - m_pBox->getHeight() * 0.0f };
		// If start position is lower than end position, then set the alpha to 128 only if end position is the one you're dragging.
		m_pBox->alpha = (draggingMouse && m_pFirstRamp->painting == 2 ? 128 : 255);
	}
	else
	{
		m_pBox->getTransform()->position = { m_pFirstRamp->startPosition.x + m_pBox->getWidth() * 0.0f, m_pFirstRamp->startPosition.y - m_pBox->getHeight() * 0.0f };
		// If start position is higher than end position, then set the alpha to 128 only if start position is the one you're dragging.
		m_pBox->alpha = (draggingMouse && m_pFirstRamp->painting == 1 ? 128 : 255);
	}

	m_pBox->runningSim = false;
	m_pBox->getTransform()->rotation = m_pFirstRamp->slope;
	m_pBox->getRigidBody()->acceleration = { 0.0f, 0.0f };
	m_pBox->getRigidBody()->velocity = { 0.0f, 0.0f };
	m_pBox->getRigidBody()->mass = 12.8f;
	m_pBox->getRigidBody()->isColliding = false;
	m_pBox->getFreeBody().clean();
}

void PlayScene::setSecondRampToLowerPosition() const
{
	if(m_pFirstRamp->startPosition.y >= m_pFirstRamp->endPosition.y) // The first ramp hasn't been properly reangled yet
	{
		m_pSecondRamp->startPosition = m_pFirstRamp->startPosition;
		m_pSecondRamp->endPosition = { (m_pFirstRamp->startPosition.x >= m_pFirstRamp->endPosition.x ? 800.0f : 0.0f), m_pFirstRamp->startPosition.y };
	}
	else // The first ramp is angled downward as it should be
	{
		if(m_pSecondRamp->startPosition != m_pFirstRamp->endPosition)
		{
			m_pSecondRamp->startPosition = m_pFirstRamp->endPosition;
			m_pSecondRamp->endPosition = { (m_pFirstRamp->endPosition.x >= m_pFirstRamp->startPosition.x ? 800.0f : 0.0f), m_pFirstRamp->endPosition.y };
			m_pSecondRamp->calcTrig();
		}
	}
}

void PlayScene::reangleBoxVelocity(const Ramp& from, const Ramp& to) const
{
	m_pBox->getTransform()->rotation = to.slope;

	if(m_pBox->maintainSpeedOnAngleChange)
	{
		//float firstRampAngle = Util::angleOf(from.slope);
		//float secondRampAngle = Util::angleOf(to.slope);
		float boxVelocityAngle = Util::angleOf(m_pBox->getRigidBody()->velocity);

		//std::cout << "First ramp: " << firstRampAngle << " second ramp: " << secondRampAngle << " box: " << boxVelocityAngle << std::endl;

		if(boxVelocityAngle > 0.0f)
		{
			float angleDifference = Util::angle(m_pBox->getRigidBody()->velocity, to.slope);
			m_pBox->getRigidBody()->velocity = Util::rotate(m_pBox->getRigidBody()->velocity, -angleDifference);
		}
		else
		{
			float angleDifference = Util::angle(m_pBox->getRigidBody()->velocity, to.slope);
			m_pBox->getRigidBody()->velocity = Util::rotate(m_pBox->getRigidBody()->velocity, angleDifference);
		}
	}
	else
	{
		m_pBox->getRigidBody()->velocity.y = 0.0f;
	}
}

void PlayScene::checkCollisions()
{
	static Ramp* lastFrameCollider = nullptr; // hooo I don't like this hack!
	static Ramp* collider = nullptr;

	m_pBox->getFreeBody().clean();
	m_pBox->getFreeBody().addForce("g", gravity * m_pBox->getRigidBody()->mass);
	m_pBox->getRigidBody()->isColliding = false;

	bool declineToRight = m_pFirstRamp->startPosition.x <= m_pFirstRamp->endPosition.x;

	if((declineToRight
			&& m_pBox->getTransform()->position.x >= m_pFirstRamp->startPosition.x
			&& m_pBox->getTransform()->position.x <= m_pFirstRamp->endPosition.x) ||
		(!declineToRight
			&& m_pBox->getTransform()->position.x >= m_pFirstRamp->endPosition.x
			&& m_pBox->getTransform()->position.x <= m_pFirstRamp->startPosition.x))
	{
		m_pBox->getRigidBody()->isColliding = true;
		collider = m_pFirstRamp;
	}
	else
	{
		m_pBox->getRigidBody()->isColliding = true;
		collider = m_pSecondRamp;
	}

	if(collider != nullptr && lastFrameCollider != nullptr && collider != lastFrameCollider) // it makes me feel bad
	{
		reangleBoxVelocity(*lastFrameCollider, *collider);
	}

	if(collider != nullptr && m_pBox->getRigidBody()->isColliding)
	{
		// Find the components of gravity which are parallel and perpendicular to the slope of the ramp
		// Step 1: Find the angles which are parallel to and perpendicular to the slope
		float sa = glm::atan(collider->slope.y / collider->slope.x);
		float na = glm::atan(collider->slope.x / -collider->slope.y); // This gives the correct values only in the cases of -180 > -90 or 0 > 90.
		if(na < 0)
			na += glm::pi<float>(); // This hack ensures that the perpendicular line is always facing downward.

		// Step 2: Obtain the magnitudes of the component vectors
		float FgparaMag = gravity.y * m_pBox->getRigidBody()->mass * glm::sin(sa); // The magnitude of the component of gravity which is parallel to the incline plane
		float FgperpMag = gravity.y * m_pBox->getRigidBody()->mass * glm::cos(sa); // The magnitude of the component of gravity which is perpendicular to the incline plane

		// Step 3: Rotate the component magnitudes according to the angles
		glm::vec2 FgparaVec = { FgparaMag * glm::cos(sa), FgparaMag * glm::sin(sa) }; // The vector visual representation to push to the freebody for drawing.
		glm::vec2 FgperpVec = { FgperpMag * glm::cos(na), FgperpMag * glm::sin(na) };
		m_pBox->getFreeBody().addForceComponent("g", FgparaVec);
		m_pBox->getFreeBody().addForceComponent("g", FgperpVec);

		// Step 4: Find the normal force, which is equal and opposite to the component of gravity which is perpendicular to the slope of the ramp
		na -= glm::pi<float>();
		glm::vec2 normal = { FgperpMag * glm::cos(na), FgperpMag * glm::sin(na) };
		m_pBox->getFreeBody().addForce("n", normal);
	}

	lastFrameCollider = collider;
}

void PlayScene::GUI_Function() const
{
	// Always open with a NewFrame
	ImGui::NewFrame();

	// See examples by uncommenting the following - also look at imgui_demo.cpp in the IMGUI filter
	//ImGui::ShowDemoWindow();

	//ImGui::Begin("Your Window Title Goes Here", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);
	ImGui::Begin("Simulation Controls", NULL, ImGuiWindowFlags_MenuBar);
	ImGui::SetWindowSize({ 340, 0 });

	if(ImGui::Button("Start Simulation"))
	{
		setBoxToHigherPosition();
		setSecondRampToLowerPosition();
		m_pBox->runningSim = true;
	}
	ImGui::SameLine();
	
	static ImVec4 paintButtonColour = ImGui::GetStyleColorVec4(ImGuiCol_Button);
	if(m_pFirstRamp->painting == 0)
		paintButtonColour = ImGui::GetStyleColorVec4(ImGuiCol_Button);
	else if(m_pFirstRamp->painted[0] + m_pFirstRamp->painted[1] == 0)
		paintButtonColour = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
	else if(m_pFirstRamp->painted[0] + m_pFirstRamp->painted[1] == 1)
	{
		paintButtonColour = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
		paintButtonColour.x = (paintButtonColour.x + 1.0f) * 0.5f;
		paintButtonColour.y = (paintButtonColour.y + 1.0f) * 0.5f;
		paintButtonColour.z = (paintButtonColour.z + 1.0f) * 0.5f;
	}
	ImGui::PushStyleColor(ImGuiCol_Button, paintButtonColour);
	if(ImGui::Button("Paint Ramp"))
	{
		m_pFirstRamp->painting = 1;
		m_pFirstRamp->painted[0] = false;
		m_pFirstRamp->painted[1] = false;
	}

	ImGui::PopStyleColor();

	ImGui::SameLine(0.0f, 1.0f);
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.1f);
	if(ImGui::SliderInt("##paintStyle", &m_pFirstRamp->paintStyle, 0, 1, ""));

	ImGui::Separator();

	// Simulation Stats
	if(ImGui::CollapsingHeader("Simulation Stats"))
	{
		std::ostringstream out;
		out.precision(2);
		
		// First Ramp Slope
		out.str("");
		out.clear();
		out << "First Ramp Slope: " << std::fixed << m_pFirstRamp->slope.y << "/" << m_pFirstRamp->slope.x;
		ImGui::Text(out.str().c_str());

		// Second Ramp Slope
		out.str("");
		out.clear();
		out << "Second Ramp Slope: " << std::fixed << m_pSecondRamp->slope.y << "/" << m_pSecondRamp->slope.x;
		ImGui::Text(out.str().c_str());

		// Display Checkboxes
		ImGui::Text("Display:");
		ImGui::SameLine();
		if(ImGui::Checkbox("Forces##showForcesCheckbox", &m_pBox->getFreeBody().showForces)) {}
		ImGui::SameLine();
		if(ImGui::Checkbox("Components##showComponentsCheckbox", &m_pBox->getFreeBody().showComponents)) {}
		ImGui::SameLine();
		if(ImGui::Checkbox("Net Force##showNetForceCheckbox", &m_pBox->getFreeBody().showNetForce)) {}
	}

	// Visual Parameters
	if(ImGui::CollapsingHeader("Visual Parameters"))
	{
		static float boxScale = m_pBox->getScale();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
		if(ImGui::SliderFloat("Box Scale##BoxScale", &boxScale, 0.25f, 2.5f))
		{
			m_pBox->setScale(boxScale);
			setBoxToHigherPosition();
			setSecondRampToLowerPosition();
		}
		ImGui::SameLine();
		static float freebodyScale = m_pBox->getFreeBody().getScale();
		if(ImGui::SliderFloat("Arrow Scale##FreebodyScale", &freebodyScale, 0.25f, 5.0f))
		{
			m_pBox->getFreeBody().setScale(freebodyScale);
		}

		static float arrowAngle = m_pBox->getFreeBody().getArrowAngle();
		if(ImGui::SliderFloat("Head Angle##ArrowAngle", &arrowAngle, 0.0f, 180.0f))
		{
			m_pBox->getFreeBody().setArrowAngle(arrowAngle);
		}
		ImGui::SameLine();
		static float arrowScale = m_pBox->getFreeBody().getArrowScale();
		if(ImGui::SliderFloat("Head Scale##ArrowScale", &arrowScale, 0.0f, 100.0f))
		{
			m_pBox->getFreeBody().setArrowScale(arrowScale);
		}

		static int labelSize = m_pBox->getFreeBody().getLabelSize();
		if(ImGui::SliderInt("Label Size##LabelSize", &labelSize, 0, 32))
		{
			m_pBox->getFreeBody().setLabelSize(labelSize);
		}

		ImGui::Separator();
	}

	if(ImGui::CollapsingHeader("Simulation Parameters", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.35f);
		if(ImGui::SliderFloat("Length##Length", &m_pFirstRamp->length, 0.0f, 1000.0f))
		{
			m_pFirstRamp->calcPositions();
			setBoxToHigherPosition();
			setSecondRampToLowerPosition();
		}
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.34f);
		ImGui::SameLine();
		if(ImGui::SliderFloat("Angle##Angle", &m_pFirstRamp->angle, 179.999f, 0.001f))
		{
			m_pFirstRamp->calcPositions();
			setBoxToHigherPosition();
			setSecondRampToLowerPosition();
		}

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.4f);
		if(ImGui::SliderFloat("x##startX", &m_pFirstRamp->startPosition.x, 0.0f, 800.0f))
		{
			m_pFirstRamp->calcTrig();
			setBoxToHigherPosition();
			setSecondRampToLowerPosition();
		}
		ImGui::SameLine();
		if(ImGui::SliderFloat("y##startY", &m_pFirstRamp->startPosition.y, 0.0f, 600.0f))
		{
			if(m_pFirstRamp->startPosition.y >= m_pFirstRamp->endPosition.y)
				m_pFirstRamp->endPosition.y = m_pFirstRamp->startPosition.y + 0.001f;
			m_pFirstRamp->calcTrig();
			setBoxToHigherPosition();
			setSecondRampToLowerPosition();
		}

		if(ImGui::SliderFloat("x##endX", &m_pFirstRamp->endPosition.x, 0.0f, 800.0f))
		{
			m_pFirstRamp->calcTrig();
			setBoxToHigherPosition();
			setSecondRampToLowerPosition();
		}
		ImGui::SameLine();
		if(ImGui::SliderFloat("y##endY", &m_pFirstRamp->endPosition.y, 0.0f, 600.0f))
		{
			if(m_pFirstRamp->startPosition.y >= m_pFirstRamp->endPosition.y)
				m_pFirstRamp->startPosition.y = m_pFirstRamp->endPosition.y - 0.001f;
			m_pFirstRamp->calcTrig();
			setBoxToHigherPosition();
			setSecondRampToLowerPosition();
		}
	}

	ImGui::End();

	// Don't Remove this
	ImGui::Render();
	ImGuiSDL::Render(ImGui::GetDrawData());
	ImGui::StyleColorsDark();
}
