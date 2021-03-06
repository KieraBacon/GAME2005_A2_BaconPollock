#include "FreeBody.h"
#include "Util.h"
#include "glm/trigonometric.hpp"
#include <sstream>

DisplayForce::DisplayForce(std::string name, glm::vec2 head, const glm::vec2& origin, int size, glm::vec4& colour, float& scale, float extender, float& arrowScale, float& arrowHead) :
	name(name), head(head), origin(origin), colour(colour), m_fScale(scale), m_fArrowScale(arrowScale), m_fArrowHead(arrowHead), extender(extender)
{
	SDL_Color col = { colour.r * 255.0f, colour.g * 255.0f, colour.b * 255.0f, colour.a * 255.0f };
	m_pLabel = Label(name, "Consolas", size, std::move(col), origin + head, 0, true);
}

DisplayForce::DisplayForce(const DisplayForce& base) :
	name(base.name), m_pLabel(base.m_pLabel),
	head(base.head), origin(base.origin), colour(base.colour),
	m_fScale(base.m_fScale), m_fArrowScale(base.m_fArrowScale), m_fArrowHead(base.m_fArrowHead), extender(base.extender)
{}

DisplayForce::~DisplayForce()
{
}

void DisplayForce::draw()
{
	float magnitude = Util::magnitude(head);
	if(magnitude >= 0.01f || magnitude <= -0.01f)
	{
		Util::DrawLine(origin, origin + head * m_fScale, colour);
		Util::DrawLine(origin + head * m_fScale, origin + head * m_fScale + Util::normalize((Util::rotate(head, m_fArrowHead))) * m_fArrowScale, colour);
		Util::DrawLine(origin + head * m_fScale, origin + head * m_fScale + Util::normalize((Util::rotate(head, -m_fArrowHead))) * m_fArrowScale, colour);

		if(extender > 0.0f)
			m_pLabel.getTransform()->position = origin + head * m_fScale + Util::normalize(head) * static_cast<float>(m_pLabel.getHeight()) * extender;
		else
			m_pLabel.getTransform()->position = origin + head * m_fScale;
		m_pLabel.getTransform()->position.x += (m_fScale * 0.5);

		std::ostringstream out;
		out.precision(1);
		out << std::fixed << name << ": " << magnitude << "N";
		m_pLabel.setText(out.str());

		m_pLabel.draw();
	}
}

FreeBody::FreeBody()
{
	m_ForceColour = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_ComponentColour = { 0.0f, 0.0f, 1.0f, 0.5f };
	m_NetColour = { 1.0f, 0.0f, 0.0f, 1.0f };
	m_fScale = 5.0f;
	m_fArrowScale = 15.0f;
	m_fArrowHead = 150.0f;
	m_iLabelSize = 14;

	m_pNetForce = new DisplayForce("Net", glm::vec2(0.0f, 0.0f), getTransform()->position,
		m_iLabelSize, m_NetColour, m_fScale, 2.25f, m_fArrowScale, m_fArrowHead);

	showForces = showComponents = showNetForce = true;
}

FreeBody::~FreeBody()
{
	clean();
	delete m_pNetForce;
}

void FreeBody::draw()
{
	const glm::vec2& position = getTransform()->position;
	if(showForces)
	{
		for(auto force : m_vForces)
		{
			force.draw();
		}
	}
	if(showComponents)
	{
		for(auto force : m_vForceComponents)
		{
			force.draw();
		}
	}
	if(showNetForce)
	{
		glm::vec2 net = getNetForce();
		m_pNetForce->head = net;
		m_pNetForce->draw();
	}
}

void FreeBody::update()
{
}

void FreeBody::clean()
{
	m_vForces.clear();
	m_vForceComponents.clear();
}

void FreeBody::addForce(std::string name, glm::vec2 force)
{
	m_vForces.push_back(DisplayForce(name, force, getTransform()->position, m_iLabelSize, m_ForceColour, m_fScale, 1.0f, m_fArrowScale, m_fArrowHead));
}

void FreeBody::addForceComponent(std::string name, glm::vec2 forceComponent)
{
	m_vForceComponents.push_back(DisplayForce(name, forceComponent, getTransform()->position, m_iLabelSize, m_ComponentColour, m_fScale, 1.0f, m_fArrowScale, m_fArrowHead));
}

glm::vec2 FreeBody::getNetForce()
{
	glm::vec2 net = { 0.0f, 0.0f };
	for(auto force : m_vForces)
	{
		net += force.head;
	}
	return net;
}

float FreeBody::getScale()
{
	return m_fScale;
}

void FreeBody::setScale(float scale)
{
	m_fScale = scale;
}

float FreeBody::getArrowScale()
{
	return m_fArrowScale;
}

void FreeBody::setArrowScale(float scale)
{
	m_fArrowScale = scale;
}

float FreeBody::getArrowAngle()
{
	return m_fArrowHead;
}

void FreeBody::setArrowAngle(float angle)
{
	m_fArrowHead = angle;
}

int FreeBody::getLabelSize()
{
	return m_iLabelSize;
}

void FreeBody::setLabelSize(int size)
{
	m_iLabelSize = size;
	for(auto force : m_vForces)
		force.m_pLabel.setSize(m_iLabelSize);
	for(auto force : m_vForceComponents)
		force.m_pLabel.setSize(m_iLabelSize);
	m_pNetForce->m_pLabel.setSize(m_iLabelSize);
}
