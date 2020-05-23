#include "PouEngine/core/GameState.h"

namespace pou
{

GameState::GameState()
{
    //ctor
}

GameState::~GameState()
{
}

void GameState::setManager(StatesManager *manager)
{
    m_manager = manager;
}

void GameState::pause()
{
    m_running = false;
}

void GameState::resume()
{
    m_running = true;
}


}
