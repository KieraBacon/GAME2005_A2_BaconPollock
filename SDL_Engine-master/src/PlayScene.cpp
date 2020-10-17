#include "PlayScene.h"
#include "Game.h"
#include "EventManager.h"

// required for IMGUI
#include "imgui.h"
#include "imgui_sdl.h"
#include "Renderer.h"
#include <sstream>
#include "CollisionManager.h"

PlayScene::PlayScene()
{
	PlayScene::start();
}

PlayScene::~PlayScene()
= default;

void PlayScene::draw()
{
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
	static bool dragging = false;

	EventManager::Instance().update();
	if(m_pFirstRamp->painting > 0)
	{
		if(dragging)
		{
			if(m_pFirstRamp->painting == 1)
			{
				m_pFirstRamp->startPosition = EventManager::Instance().getMousePosition();
			}
			else if(m_pFirstRamp->painting == 2)
			{
				m_pFirstRamp->endPosition = EventManager::Instance().getMousePosition();
			}

			m_pFirstRamp->calcTrig();
			setBoxToStartingPosition();

			if(EventManager::Instance().getMouseButtonUp(0))
			{
				dragging = false;
				if(m_pFirstRamp->painting == 1)
				{
					m_pFirstRamp->painting = 2;
				}
				else if(m_pFirstRamp->painting == 2)
				{
					m_pFirstRamp->painting = 0;
				}
			}
		}
		else if(EventManager::Instance().getMouseButtonDown(0))
		{
			dragging = true;
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
	gravity = { 0.0f, 9.8f };

	// Set GUI Title
	m_guiTitle = "Play Scene";

	// Ramp
	m_pFirstRamp = new Ramp();
	addChild(m_pFirstRamp);
	m_pFirstRamp->getTransform()->position = { 400.0f, 300.0f };
	m_pFirstRamp->getTransform()->position.y += m_pFirstRamp->getWidth();
	{
		float rise = -30;
		float run = 40;
		m_pFirstRamp->startPosition = { m_pFirstRamp->getTransform()->position.x - (run * 0.5f), m_pFirstRamp->getTransform()->position.y + (rise * 0.5f) };
		m_pFirstRamp->endPosition = { m_pFirstRamp->getTransform()->position.x + (run * 0.5f), m_pFirstRamp->getTransform()->position.y - (rise * 0.5f) };
		m_pFirstRamp->calcTrig();
	}

	// Box
	m_pBox = new Box();
	addChild(m_pBox);
	m_pBox->setScale(0.75f);
	m_pBox->getFreeBody().setScale(0.5f);
	m_pBox->getFreeBody().setArrowScale(10.0f);
	setBoxToStartingPosition();

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

void PlayScene::setBoxToStartingPosition() const
{
	m_pBox->runningSim = false;
	m_pBox->getTransform()->position = { m_pFirstRamp->startPosition.x + m_pBox->getWidth() * 0.0f, m_pFirstRamp->startPosition.y - m_pBox->getHeight() * 0.0f };
	m_pBox->getTransform()->rotation = m_pFirstRamp->slope;
	m_pBox->getRigidBody()->acceleration = { 0.0f, 0.0f };
	m_pBox->getRigidBody()->velocity = { 0.0f, 0.0f };
	m_pBox->getRigidBody()->mass = 12.8f;
	m_pBox->getRigidBody()->isColliding = false;
	m_pBox->getFreeBody().clean();
}

#include "Util.h"
void PlayScene::checkCollisions()
{
	if(true)
	{
		m_pBox->getFreeBody().clean();
		if(CollisionManager::lineAngledRectCheck(
			m_pFirstRamp->startPosition,
			m_pFirstRamp->endPosition,
			m_pBox->getTransform()->position,
			m_pBox->getWidth(),
			m_pBox->getHeight(),
			m_pBox->getAngle()
		))
		{
			m_pBox->getFreeBody().addForce("g", gravity * m_pBox->getRigidBody()->mass);
			auto m = m_pBox->getRigidBody()->mass;
			// Find the components of gravity which are parallel and perpendicular to the slope of the ramp
			float sa = glm::atan(m_pFirstRamp->slope.y / m_pFirstRamp->slope.x);
			float na = glm::atan(m_pFirstRamp->slope.x / -m_pFirstRamp->slope.y); // This gives the correct values only in the cases of -180 > -90 or 0 > 90.
			if(na < 0)
				na += glm::pi<float>(); // This hack ensures that the perpendicular line is always facing downward.
			float FgparaMag = gravity.y * m_pBox->getRigidBody()->mass * glm::sin(sa); // The magnitude of the component of gravity which is parallel to the incline plane
			float FgperpMag = gravity.y * m_pBox->getRigidBody()->mass * glm::cos(sa); // The magnitude of the component of gravity which is perpendicular to the incline plane
			glm::vec2 FgparaVec = { FgparaMag * glm::cos(sa), FgparaMag * glm::sin(sa) };
			glm::vec2 FgperpVec = { FgperpMag * glm::cos(na), FgperpMag * glm::sin(na) };
			m_pBox->getFreeBody().addForceComponent("g", FgparaVec);
			m_pBox->getFreeBody().addForceComponent("g", FgperpVec);

			// Find the normal force, which is equal and opposite to the component of gravity which is perpendicular to the slope of the ramp
			na -= glm::pi<float>();
			glm::vec2 normal = { FgperpMag * glm::cos(na), FgperpMag * glm::sin(na) };
			m_pBox->getFreeBody().addForce("n", normal);
			m_pBox->getRigidBody()->isColliding = true;
		}
		else
		{
			m_pBox->getFreeBody().addForce("g", gravity);
			m_pBox->getRigidBody()->isColliding = false;
		}
	}
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
		setBoxToStartingPosition();
		m_pBox->runningSim = true;
	}
	ImGui::SameLine();
	
	static ImVec4 paintButtonColour = ImGui::GetStyleColorVec4(ImGuiCol_Button);
	//colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	//colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	//colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	if(m_pFirstRamp->painting == 0)
		paintButtonColour = ImGui::GetStyleColorVec4(ImGuiCol_Button);
	if(m_pFirstRamp->painting == 1)
	{
		paintButtonColour = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
		paintButtonColour.x = (paintButtonColour.x + 1.0f) * 0.5f;
		paintButtonColour.y = (paintButtonColour.y + 0.0f) * 0.5f;
		paintButtonColour.z = (paintButtonColour.z + 0.0f) * 0.5f;
	}
	if(m_pFirstRamp->painting == 2)
	{
		paintButtonColour = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
		paintButtonColour.x = (paintButtonColour.x + 2.0f) * 0.33f;
		paintButtonColour.y = (paintButtonColour.y + 0.0f) * 0.33f;
		paintButtonColour.z = (paintButtonColour.z + 0.0f) * 0.33f;
	}
	ImGui::PushStyleColor(ImGuiCol_Button, paintButtonColour);
	if(ImGui::Button("Paint Incline"))
	{
		m_pFirstRamp->painting = 1;
	}

	ImGui::PopStyleColor();

	ImGui::Separator();

	// Display Checkboxes
	if(ImGui::Checkbox("Forces##showForcesCheckbox", &m_pBox->getFreeBody().showForces)) {}
	ImGui::SameLine();
	if(ImGui::Checkbox("Components##showComponentsCheckbox", &m_pBox->getFreeBody().showComponents)) {}
	ImGui::SameLine();
	if(ImGui::Checkbox("Net Force##showNetForceCheckbox", &m_pBox->getFreeBody().showNetForce)) {}

	// Visual Parameters

	if(ImGui::CollapsingHeader("Visual Parameters"))
	{
		static float boxScale = m_pBox->getScale();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
		if(ImGui::SliderFloat("Box Scale##BoxScale", &boxScale, 0.25f, 2.5f))
		{
			m_pBox->setScale(boxScale);
			setBoxToStartingPosition();
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

		ImGui::Separator();
	}

	if(ImGui::CollapsingHeader("Simulation Parameters", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.35f);
		if(ImGui::SliderFloat("##Length", &m_pFirstRamp->length, 0.0f, 1000.0f))
		{
			m_pFirstRamp->calcPositions();
			setBoxToStartingPosition();
		}
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.35f);
		ImGui::SameLine();
		if(ImGui::SliderFloat("##Angle", &m_pFirstRamp->angle, -180.0f, 180.0f))
		{
			m_pFirstRamp->calcPositions();
			setBoxToStartingPosition();
		}

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.4f);
		if(ImGui::SliderFloat("x##startX", &m_pFirstRamp->startPosition.x, 0.0f, 800.0f))
		{
			m_pFirstRamp->calcTrig();
			setBoxToStartingPosition();
		}
		ImGui::SameLine();
		if(ImGui::SliderFloat("y##startY", &m_pFirstRamp->startPosition.y, 0.0f, 600.0f))
		{
			m_pFirstRamp->calcTrig();
			setBoxToStartingPosition();
		}

		if(ImGui::SliderFloat("x##endX", &m_pFirstRamp->endPosition.x, 0.0f, 800.0f))
		{
			m_pFirstRamp->calcTrig();
			setBoxToStartingPosition();
		}
		ImGui::SameLine();
		if(ImGui::SliderFloat("y##endY", &m_pFirstRamp->endPosition.y, 0.0f, 600.0f))
		{
			m_pFirstRamp->calcTrig();
			setBoxToStartingPosition();
		}
	}

	ImGui::End();

	// Don't Remove this
	ImGui::Render();
	ImGuiSDL::Render(ImGui::GetDrawData());
	ImGui::StyleColorsDark();
}
