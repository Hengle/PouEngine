#ifndef GAMECLIENT_H
#define GAMECLIENT_H

#include "PouEngine/Types.h"
#include "PouEngine/net/NetEngine.h"
#include "PouEngine/renderers/RenderWindow.h"

#include "world/GameWorld.h"

class GameClient
{
    public:
        GameClient();
        virtual ~GameClient();

        bool create(unsigned short port = 0);

        bool connectToServer(const pou::NetAddress &address);
        bool disconnectFromServer();

        void update(const pou::Time &elapsedTime);
        void render(pou::RenderWindow *renderWindow);

        //const pou::NetAddress &getAddress() const;
        unsigned short getPort() const;

        void sendMsgTest(bool reliable, bool forceSend);

    protected:
        void cleanup();

        void processMessage(std::shared_ptr<pou::NetMessage> msg);

        void updateWorld(const pou::Time &elapsedTime);

    private:
        std::unique_ptr<pou::AbstractClient> m_client;

        GameWorld   m_world;
        uint16_t    m_curWorldId;
        bool        m_isWaitingForWorldSync;
        //pou::Timer  m_syncWorldTimer;

        //static const float CLIENTWORLD_SYNC_DELAY;
};

#endif // GAMECLIENT_H
