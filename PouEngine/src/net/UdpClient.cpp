#include "PouEngine/net/UdpClient.h"

#include "PouEngine/net/UdpPacketTypes.h"
#include "PouEngine/utils/Logger.h"
#include "PouEngine/utils/Hasher.h"


#include "PouEngine/core/VApp.h"

#include <vector>
#include <glm/gtc/random.hpp>

namespace pou
{

const float UdpClient::CONNECTING_ATTEMPT_DELAY = 1.0f;
const float UdpClient::CONNECTING_MAX_TIME = 10.0f;
const float UdpClient::DEFAULT_PINGDELAY = .5f;
const float UdpClient::DEFAULT_DECONNECTIONPINGDELAY = 5.0f;


UdpClient::UdpClient() :
    m_pingDelay(UdpClient::DEFAULT_PINGDELAY),
    m_deconnectionPingDelay(UdpClient::DEFAULT_DECONNECTIONPINGDELAY)
{
    //ctor
}

UdpClient::~UdpClient()
{
    //dtor
}


bool UdpClient::create(unsigned short port)
{
    if(!m_packetsExchanger.createSocket(port))
        return (false);

    m_port = m_packetsExchanger.getPort();

    return (true);
}

bool UdpClient::destroy()
{
    m_packetsExchanger.destroy();

    return (true);
}

bool UdpClient::connectToServer(const NetAddress &serverAddress)
{
    m_connectionStatus  = ConnectionStatus_Connecting;
    m_serverAddress     = serverAddress;
    m_salt              = glm::linearRand(0, (int)pow(2,SALT_SIZE));

    m_connectingTimer.reset(CONNECTING_MAX_TIME);

    return (true);
}

bool UdpClient::disconnectFromServer()
{
    if(m_connectionStatus != ConnectionStatus_Disconnected)
    {
        Logger::write("Disconnected from server: "+m_serverAddress.getAddressString());
        for(auto i = 0 ; i < 5 ; ++i)
            this->sendConnectionMsg(m_serverAddress, ConnectionMessage_Disconnection);
    }

    m_connectionStatus = ConnectionStatus_Disconnected;

    return (true);
}


void UdpClient::update(const Time &elapsedTime)
{

    if(m_connectionStatus != ConnectionStatus_Connected
    && m_connectingTimer.update(elapsedTime))
    {
        m_connectionStatus = ConnectionStatus_Disconnected;
        Logger::warning("Could not connect to: "+m_serverAddress.getAddressString());
    }

    if(m_connectionStatus == ConnectionStatus_Connecting)
    {
        if(m_connectingAttemptTimer.update(elapsedTime)
        || !m_connectingAttemptTimer.isActive())
        {
            this->tryToConnect();
            m_connectingAttemptTimer.reset(CONNECTING_ATTEMPT_DELAY);
        }
    }

    if(m_connectionStatus == ConnectionStatus_Challenging)
    {
        if(m_connectingAttemptTimer.update(elapsedTime)
        || !m_connectingAttemptTimer.isActive())
        {
            this->tryToChallenge();
            m_connectingAttemptTimer.reset(CONNECTING_ATTEMPT_DELAY);
        }
    }

    if(m_connectionStatus == ConnectionStatus_Connected)
        if(m_lastServerAnswerPingTime + m_deconnectionPingDelay < m_curLocalTime)
        {
            Logger::write("Server connection timed out");
            this->disconnectFromServer();
        }

    if(m_connectionStatus != ConnectionStatus_Disconnected
    && m_lastServerPingTime + m_pingDelay < m_curLocalTime)
        this->sendConnectionMsg(m_serverAddress, ConnectionMessage_Ping);

    m_packetsExchanger.update(elapsedTime);

    AbstractClient::update(elapsedTime);
}


void UdpClient::receivePackets()
{
    /*NetAddress sender;
    std::vector<uint8_t> buffer(DEFAULT_MAX_PACKETSIZE);

    while(true)
    {
        int bytes_read = m_socket.receive(sender, buffer.data(), buffer.size());
        if (!bytes_read)
            break;

        std::cout<<"Server received packet from"<<sender.getAddressString()<<std::endl;
    }*/

    std::vector<UdpBuffer> packet_buffers;
    m_packetsExchanger.receivePackets(packet_buffers);
    for(auto &buffer : packet_buffers)
        this->processMessages(buffer);
}

void UdpClient::processMessages(UdpBuffer &buffer)
{
    PacketType packetType = m_packetsExchanger.readPacketType(buffer);

    if(m_connectionStatus != ConnectionStatus_Disconnected
    && buffer.address == m_serverAddress)
        m_lastServerAnswerPingTime = m_curLocalTime;

    if(packetType == PacketType_ConnectionMsg)
        this->processConnectionMessages(buffer);
}


void UdpClient::processConnectionMessages(UdpBuffer &buffer)
{
    UdpPacket_ConnectionMsg packet;
    if(!m_packetsExchanger.readPacket(packet, buffer))
        return;

    if(packet.connectionMessage == ConnectionMessage_ConnectionAccepted)
    {
        if(m_connectionStatus != ConnectionStatus_Connected)
            Logger::write("Connected to server: "+buffer.address.getAddressString());
        m_connectionStatus = ConnectionStatus_Connected;
        return;
    }
    if(packet.connectionMessage == ConnectionMessage_Challenge)
    {
        m_connectionStatus = ConnectionStatus_Challenging;
        m_serverSalt       = packet.salt;
        m_connectingTimer.reset(CONNECTING_MAX_TIME);
    }
    if(packet.connectionMessage == ConnectionMessage_ConnectionDenied
    && m_connectionStatus != ConnectionStatus_Connected)
    {
        Logger::write("Connection denied to server: "+buffer.address.getAddressString());
        m_connectionStatus = ConnectionStatus_Disconnected;
        return;
    }
    if(packet.connectionMessage == ConnectionMessage_Disconnection)
    {
        this->disconnectFromServer();
        //Logger::write("Disconnected from server: "+buffer.address.getAddressString());
        //m_connectionStatus = ConnectionStatus_Disconnected;
        return;
    }
}

void UdpClient::sendConnectionMsg(NetAddress &address, ConnectionMessage msg)
{
    UdpPacket_ConnectionMsg packet;
    packet.type = PacketType_ConnectionMsg;
    packet.connectionMessage = msg;
    packet.salt = m_salt ^ m_serverSalt;

    if(msg == ConnectionMessage_ConnectionRequest)
        packet.salt = m_salt;

    m_packetsExchanger.sendPacket(address,packet);

    m_lastServerPingTime = m_curLocalTime;
}

void UdpClient::tryToConnect()
{
    Logger::write("Attempting to connect to: "+m_serverAddress.getAddressString());

    UdpPacket_ConnectionMsg connectionPacket;
    connectionPacket.type = PacketType_ConnectionMsg;
    connectionPacket.connectionMessage = ConnectionMessage_ConnectionRequest;
    connectionPacket.salt = m_salt;

    m_packetsExchanger.sendPacket(m_serverAddress, connectionPacket);
}


void UdpClient::tryToChallenge()
{
    Logger::write("Attempting to challenge: "+m_serverAddress.getAddressString());

    UdpPacket_ConnectionMsg connectionPacket;
    connectionPacket.type = PacketType_ConnectionMsg;
    connectionPacket.connectionMessage = ConnectionMessage_Challenge;
    connectionPacket.salt = m_salt ^ m_serverSalt;

    m_packetsExchanger.sendPacket(m_serverAddress, connectionPacket);
}


}