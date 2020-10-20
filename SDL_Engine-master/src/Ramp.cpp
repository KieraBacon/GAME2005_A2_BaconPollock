#include "Ramp.h"
#include "Util.h"
#include "glm/trigonometric.hpp"
#include "Renderer.h"
#include "Game.h"

Ramp::Ramp()
{
	slopeColour = { 0.0f, 0.0f, 0.0f, 1.0f };
	boundColour = { 0.0f, 0.0f, 0.0f, 0.5f };
	fillColour = { 0.25f, 0.25f, 0.25f, 0.25f };
	getTransform()->position = glm::vec2(0.0f, 0.0f);
	getRigidBody()->velocity = glm::vec2(0.0f, 0.0f);
	getRigidBody()->acceleration = glm::vec2(0.0f, 0.0f);
	getRigidBody()->isColliding = false;
	setWidth(50);
	setType(TARGET);

	painted[0] = painted[1] = true;
	painting = 0;
	paintStyle = 0;
	coefficientOfFriction = 0.0f;
	shapeChanged = true;
	m_pfillTexture = nullptr;
}

Ramp::~Ramp() = default;

void Ramp::draw()
{
	const auto renderer = Renderer::Instance()->getRenderer();

	int xmin = (startPosition.x <= endPosition.x ? startPosition.x : endPosition.x);
	int ymin = (startPosition.y <= endPosition.y ? startPosition.y : endPosition.y);
	int xmax = (startPosition.x >= endPosition.x ? startPosition.x : endPosition.x);
	int ymax = (startPosition.y >= endPosition.y ? startPosition.y : endPosition.y);
	int w = (xmax - xmin);
	int h = (ymax - ymin);
	if(shapeChanged && w > 0 && h > 0)
	{
		if(m_pfillTexture != nullptr)
		{
			SDL_DestroyTexture(m_pfillTexture);
			m_pfillTexture = nullptr;
		}

		SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);

		for(int x = 0; x < surface->w; x++)
		{
			for(int y = 0; y < surface->h; y++)
			{
				bool pos = ((slope.x > 0.0f && slope.y > 0.0f) || (slope.x < 0.0f && slope.y < 0.0f));
				
				// Get y position at x on slope
				int yOfLine = x * slope.y / slope.x;
				if(!pos)
					yOfLine += h;

				if(y > yOfLine)
				{
					//Convert the pixels to 32 bit
					Uint32* pixels = (Uint32*)surface->pixels;

					//Set the pixel
					SDL_Colour c = { fillColour.r * 255.0f, fillColour.g * 255.0f, fillColour.b * 255.0f, fillColour.a * 255.0f };
					pixels[(y * surface->w) + x] = (Uint32)((c.a << 24) + (c.b << 16) + (c.g << 8) + (c.r << 0));
				}
			}
		}

		//SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		//SDL_SetRenderDrawColor(renderer, fillColour.x * 255.0f, fillColour.y * 255.0f, fillColour.z * 255.0f, fillColour.w * 255.0f);
		//for(int y = ymin; y <= ymax; ++y)
		//{
		//	for(int x = xmin; x <= xmax; ++x)
		//	{
		//		auto A = (startPosition.x >= endPosition.x ? endPosition : startPosition);
		//		auto B = (startPosition.x >= endPosition.x ? startPosition : endPosition);
		//		if(((B.x - A.x) * (y - A.y) - (B.y - A.y) * (x - A.x)) > 0.0f) // Credit to users Regexident and Eric Bainville from https://stackoverflow.com/questions/1560492/how-to-tell-whether-a-point-is-to-the-right-or-left-side-of-a-line.
		//			SDL_RenderDrawPoint(renderer, x, y);
		//	}
		//}

		m_pfillTexture = SDL_CreateTextureFromSurface(renderer, surface);
		if(m_pfillTexture == nullptr)
		{
			std::cout << "Unable to create texture from surface!";
			m_pfillTexture = nullptr;
		}

		if(surface != nullptr)
		{
			SDL_FreeSurface(surface);
			surface = nullptr;
		}

		shapeChanged = false;
	}

	SDL_Rect src = { 0, 0, (xmax - xmin), (ymax - ymin) };
	SDL_Rect dst = { xmin, ymin, (xmax - xmin), (ymax - ymin) };
	SDL_RenderCopy(renderer, m_pfillTexture, &src, &dst);

	Util::DrawLine(startPosition, endPosition, slopeColour);
	if(startPosition.y >= endPosition.y)
	{
		Util::DrawLine(endPosition, glm::vec2(endPosition.x, startPosition.y), boundColour);
		Util::DrawLine(glm::vec2(endPosition.x, startPosition.y), startPosition, boundColour);
	}
	else
	{
		Util::DrawLine(startPosition, glm::vec2(startPosition.x, endPosition.y), boundColour);
		Util::DrawLine(glm::vec2(startPosition.x, endPosition.y), endPosition, boundColour);
	}
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
	shapeChanged = true;
}

void Ramp::calcTrig()
{
	length = Util::distance(startPosition, endPosition);
	if(startPosition.y >= endPosition.y)
		slope = Util::normalize(glm::vec2((endPosition.x - startPosition.x), (endPosition.y - startPosition.y)));
	else
		slope = Util::normalize(glm::vec2((startPosition.x - endPosition.x), (startPosition.y - endPosition.y)));
	angle = glm::degrees(glm::atan(-slope.y / slope.x));
	if(angle < 0.0f)
		angle += 180.0f;

	getTransform()->position.x = startPosition.x + (endPosition.x - startPosition.x) * 0.5f;
	getTransform()->position.y = startPosition.y + (endPosition.y - startPosition.y) * 0.5f;

	shapeChanged = true;
}

void Ramp::clampPositions()
{
	glm::vec4 screen = { 0.0f, 0.0f, 800.0f, 600.0f };
	if(clampBounds(startPosition, slope, screen) &&
		clampBounds(endPosition, slope, screen))
		calcTrig();
}

void Ramp::setStartOnTop()
{
	if(painting == 0 && startPosition.y > endPosition.y)
		swapPositions();
}

void Ramp::swapPositions()
{
	glm::vec2 temp = endPosition;
	endPosition = startPosition;
	startPosition = temp;
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