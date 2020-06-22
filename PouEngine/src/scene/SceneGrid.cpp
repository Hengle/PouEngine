#include "PouEngine/scene/SceneGrid.h"

namespace pou
{

SceneGrid::SceneGrid() :
    m_quadSize(1),
    m_minPos(0),
    m_gridSize(0)
{
    //ctor
}

SceneGrid::~SceneGrid()
{
    //dtor
}


void SceneGrid::addChildNode(std::shared_ptr<SimpleNode> childNode)
{
    auto childPos = childNode->getGlobalXYPosition();
    this->enlargeForPosition(childPos);

    auto gridPos = this->getGridPos(childPos);

    if(this->containsChildNode(childNode.get(),gridPos))
        return;

    m_grid[gridPos.y][gridPos.x].push_back(childNode);
    childNode->setParent(this);
    this->startListeningTo(childNode.get());
}

bool SceneGrid::removeChildNode(SimpleNode *childNode)
{
    auto childPos = childNode->getGlobalXYPosition();
    auto gridPos = this->getGridPos(childPos);
    return this->removeChildNode(childNode, gridPos);
}

bool SceneGrid::removeChildNode(SimpleNode *childNode, glm::ivec2 gridPos)
{
    if(gridPos.x < 0 || gridPos.y < 0 || gridPos.x >= m_gridSize.x || gridPos.y >= m_gridSize.y)
        return (false);

    for(auto childIt = m_grid[gridPos.y][gridPos.x].begin() ;
        childIt != m_grid[gridPos.y][gridPos.x].end() ; ++childIt)
    {
        if(childIt->get() == childNode)
        {
            childNode->setParent(nullptr);
            this->stopListeningTo(childNode);
            m_grid[gridPos.y][gridPos.x].erase(childIt);
            return (true);
        }
    }
    return (false);
}

void SceneGrid::removeAllChilds()
{
    /* do something */
}

bool SceneGrid::containsChildNode(SimpleNode *childNode, glm::ivec2 gridPos)
{
    if(gridPos.x < 0 || gridPos.y < 0 || gridPos.x >= m_gridSize.x || gridPos.y >= m_gridSize.y)
        return (false);

    for(auto childIt = m_grid[gridPos.y][gridPos.x].begin() ;
        childIt != m_grid[gridPos.y][gridPos.x].end() ; ++childIt)
    {
        if(childIt->get() == childNode)
            return (true);
    }
    return (false);
}

std::shared_ptr<SimpleNode> SceneGrid::extractChildNode(SimpleNode *childNode)
{
    auto gridPos = this->getGridPos(childNode->getGlobalXYPosition());
    return this->extractChildNode(childNode, gridPos);
}

std::shared_ptr<SimpleNode> SceneGrid::extractChildNode(SimpleNode *childNode, glm::ivec2 gridPos)
{
    if(gridPos.x < 0 || gridPos.y < 0 || gridPos.x >= m_gridSize.x || gridPos.y >= m_gridSize.y)
        return (nullptr);

    for(auto childIt = m_grid[gridPos.y][gridPos.x].begin() ;
        childIt != m_grid[gridPos.y][gridPos.x].end() ; ++childIt)
    {
        if(childIt->get() == childNode)
        {
            std::shared_ptr<SimpleNode> rNode;
            rNode = *childIt;
            rNode->setParent(nullptr);
            this->stopListeningTo(rNode.get());
            m_grid[gridPos.y][gridPos.x].erase(childIt);
            return (rNode);
        }
    }
    return (nullptr);
}


void SceneGrid::setQuadSize(float s)
{
    if(s <= 0)
        return;

    m_quadSize = s;
}

void SceneGrid::resizeQuad(glm::ivec2 minPos, glm::ivec2 gridSize)
{
    std::vector< std::vector< std::vector< std::shared_ptr<SimpleNode> > > >
            newGrid;

    newGrid.resize(gridSize.y);
    int y = minPos.y;
    for(auto it = newGrid.begin() ; it != newGrid.end() ; ++it, ++y)
    {
        int relY = y - minPos.y;
        if(relY >= 0 && relY < m_gridSize.y)
        {
            int relX = std::min(0, minPos.x - m_minPos.x);
            int newX = std::max(0, m_minPos.x - minPos.x);
            it->insert(it->begin()+newX,m_grid[relY].begin()+relX, m_grid[relY].end());
        }

        it->resize(gridSize.x);
    }

    m_grid.swap(newGrid);

    m_minPos = minPos;
    m_gridSize = gridSize;
}

void SceneGrid::enlargeForPosition(glm::vec2 pos)
{
    auto minPos = m_minPos;
    auto gridSize = m_gridSize;
    bool enlarge = false;

    if(pos.x < m_minPos.x * m_quadSize)
    {
        minPos.x = glm::floor(pos.x/m_quadSize);
        enlarge = true;
    }

    if(pos.y < m_minPos.y * m_quadSize )
    {
        minPos.y = glm::floor(pos.y/m_quadSize);
        enlarge = true;
    }

    if(pos.x >= m_quadSize * (m_minPos.x + m_gridSize.x))
    {
        gridSize.x = glm::ceil((m_minPos.x * m_quadSize + pos.x)/m_quadSize);
        enlarge = true;
    }

    if(pos.y >= m_quadSize * (m_minPos.y + m_gridSize.y))
    {
        gridSize.y = glm::ceil((m_minPos.y * m_quadSize + pos.y)/m_quadSize);
        enlarge = true;
    }

    if(enlarge)
        this->resizeQuad(minPos, gridSize);
}

void SceneGrid::addUpdateProbe(std::shared_ptr<SimpleNode> node, float radius)
{
    GridProbe probe;
    probe.node = node;
    probe.radius = radius;
    m_updateProbes.push_back(probe);
}

void SceneGrid::removeUpdateProbe(SimpleNode *node)
{
    for(auto it = m_updateProbes.begin() ; it != m_updateProbes.end() ; ++it)
        if(it->node.get() == node)
        {
            m_updateProbes.erase(it);
            return;
        }
}

void SceneGrid::setRenderProbe(std::shared_ptr<SimpleNode> node, float radius)
{
    m_renderProbe.node      = node;
    m_renderProbe.radius    = radius;
}

void SceneGrid::probesZones(std::set< std::vector<std::shared_ptr<SimpleNode> > *> &zonesToUpdate, GridProbe &probe)
{
    auto probeGridPos = this->getGridPos(probe.node->getGlobalXYPosition()-probe.radius*glm::vec2(1));
    auto probeGridSize = glm::ceil(probe.radius*2/m_quadSize);

    for(auto y = 0 ; y < (int)probeGridSize ; ++y)
    for(auto x = 0 ; x < (int)probeGridSize ; ++x)
    if(probeGridPos.y + y >= 0 && probeGridPos.x + x >= 0
    && probeGridPos.y + y < m_gridSize.y && probeGridPos.x + x < m_gridSize.x)
        zonesToUpdate.insert(&m_grid[probeGridPos.y + y][probeGridPos.x + x]);
}


/*void SceneGrid::probesZones(std::set< std::shared_ptr<SimpleNode> > &nodesToUpdate, GridProbe &probe)
{
    auto probeGridPos = this->getGridPos(probe.node->getGlobalXYPosition()-probe.radius*glm::vec2(1));
    auto probeGridSize = glm::ceil(probe.radius*2/m_quadSize);

    //std::cout<<probeGridPos.x<<" "<<probeGridPos.y<<std::endl;
    //std::cout<<probeGridSize<<std::endl;

    for(auto y = 0 ; y < (int)probeGridSize ; ++y)
    for(auto x = 0 ; x < (int)probeGridSize ; ++x)
    if(probeGridPos.y + y >= 0 && probeGridPos.x + x >= 0
    && probeGridPos.y + y < m_gridSize.y && probeGridPos.x + x < m_gridSize.x)
        nodesToUpdate.insert(m_grid[probeGridPos.y + y][probeGridPos.x + x].begin(),
                             m_grid[probeGridPos.y + y][probeGridPos.x + x].end());
}*/

void SceneGrid::update(const Time &elapsedTime, uint32_t localTime)
{
    SceneNode::update(elapsedTime, localTime);

    std::set< std::vector<std::shared_ptr<SimpleNode> > *> zonesToUpdate;
    //std::set< std::shared_ptr<SimpleNode> > nodesToUpdate;

    for(auto &probe : m_updateProbes)
        this->probesZones(zonesToUpdate, probe);
       // this->probesZones(nodesToUpdate, probe);

    for(auto* zone : zonesToUpdate)
    for(auto node : *zone)
    //for(auto node : nodesToUpdate)
        node->update(elapsedTime, localTime);

    for(auto &nodeToMove : m_nodesToMove)
    {
        auto ptrNode = this->extractChildNode(nodeToMove.second, nodeToMove.first);
        this->addChildNode(ptrNode);
    }

    m_nodesToMove.clear();
}

void SceneGrid::generateRenderingData(SceneRenderingInstance *renderingInstance, bool propagateToChilds)
{
    SceneNode::generateRenderingData(renderingInstance, false);

    if(!propagateToChilds)
        return;

    std::set< std::vector<std::shared_ptr<SimpleNode> > *> zonesToUpdate;
    //std::set< std::shared_ptr<SimpleNode> > nodesToUpdate;
    this->probesZones(zonesToUpdate, m_renderProbe);
    //this->probesZones(nodesToUpdate, m_renderProbe);

    for(auto* zone : zonesToUpdate)
    for(auto node : *zone)
    //for(auto node : nodesToUpdate)
        std::dynamic_pointer_cast<SceneNode>(node)->generateRenderingData(renderingInstance, propagateToChilds);
}

glm::ivec2 SceneGrid::getGridPos(glm::vec2 worldPos)
{
    return glm::floor((worldPos - glm::vec2(m_minPos) * m_quadSize)/m_quadSize);
}

void SceneGrid::notify(NotificationSender* sender, int notificationType, void* data)
{
    SceneNode::notify(sender, notificationType, data);

    if(notificationType == NotificationType_NodeMoved)
    {
        auto node = static_cast<SceneNode*>(sender);
        auto oldPos = *((glm::vec3*)data);

        auto oldGridPos = this->getGridPos(glm::vec2(oldPos));
        auto newGridPos = this->getGridPos(node->getGlobalXYPosition());

        if(oldGridPos != newGridPos)
        if(this->containsChildNode(node, oldGridPos))
        {
            m_nodesToMove.push_back({oldGridPos, node});
            //auto ptrNode = this->extractChildNode(node, oldGridPos);
            //this->addChildNode(ptrNode);
        }
    }

    if(notificationType == NotificationType_SenderDestroyed)
        this->removeUpdateProbe((pou::SimpleNode*)sender);
}

}
