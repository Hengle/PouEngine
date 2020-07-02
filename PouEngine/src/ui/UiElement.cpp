#include "PouEngine/ui/UiElement.h"

namespace pou
{

UiElement::UiElement(UserInterface *interface) :
    m_interface(interface),
    m_size(0.0f),
    m_isVisible(true),
    m_isMouseHover(false)
{
}


UiElement::~UiElement()
{
    //dtor
}

void UiElement::setSize(glm::vec2 s)
{
    if(s.x >= 0 && s.y >= 0)
        m_size = s;
}

void UiElement::setSize(float x, float y)
{
    this->setSize({x,y});
}

const glm::vec2 &UiElement::getSize()
{
    return m_size;
}


std::shared_ptr<SimpleNode> UiElement::nodeAllocator()
{
    return std::make_shared<UiElement>(m_interface);
}

void UiElement::render(UiRenderer *renderer)
{
    if(!m_isVisible)
        return;

    for(auto node : m_childs)
        std::dynamic_pointer_cast<UiElement>(node)->render(renderer);
}


void UiElement::handleEvents(const EventsManager *eventsManager)
{
    m_isMouseHover = false;

    if(!m_isVisible)
        return;

    auto mousePos = eventsManager->mousePosition();
    auto &globalPos = this->getGlobalPosition();
    auto &size      = this->getSize();
    if(mousePos.x >= globalPos.x && mousePos.x <= globalPos.x + size.x
    && mousePos.y >= globalPos.y && mousePos.y <= globalPos.y + size.y)
        m_isMouseHover = true;

    for(auto node : m_childs)
        std::dynamic_pointer_cast<UiElement>(node)->handleEvents(eventsManager);
}

void UiElement::show()
{
    m_isVisible = true;
}

void UiElement::hide()
{
    m_isVisible = false;
}

bool UiElement::isVisible()
{
    return m_isVisible;
}

bool UiElement::isMouseHover()
{
    return m_isMouseHover;
}




}
