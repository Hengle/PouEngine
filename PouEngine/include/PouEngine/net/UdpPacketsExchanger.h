#ifndef UDPPACKETSEXCHANGER_H
#define UDPPACKETSEXCHANGER_H

#include "PouEngine/Types.h"
#include "PouEngine/net/NetAddress.h"
#include "PouEngine/net/UdpPacketTypes.h"
#include "PouEngine/net/UdpSocket.h"

#include <vector>
#include <map>
#include <cstdint>

namespace pou
{

class UdpPacketsExchanger;

struct UdpBuffer
{
    UdpBuffer();

    NetAddress  address;
    std::vector<uint8_t> buffer;
};

struct FragmentedPacket
{
    FragmentedPacket();

    bool needToBeCleaned;
    uint16_t sequence;
    int nbr_frags;
    float birthday;

    std::vector< std::vector<uint8_t> > fragmentBuffers;
    std::vector< bool > receivedFrags;
    int nbr_receivedFrags;
};

class UdpPacketsExchanger
{
    public:
        UdpPacketsExchanger();
        virtual ~UdpPacketsExchanger();

        bool createSocket(unsigned short port = 0);
        void destroy();

        virtual void update(const pou::Time &elapsedTime);

        virtual void sendPacket(NetAddress &address, UdpPacket &packet, bool forceNonFragSend = false);
        virtual void sendPacket(UdpBuffer &packetBuffer, bool forceNonFragSend = false);
        virtual void receivePackets(std::vector<UdpBuffer> &packetBuffers);


        void generatePacketHeader(UdpPacket &packet, PacketType packetType);
        PacketType readPacketType(UdpBuffer &packetBuffer);
        bool readPacket(UdpPacket &packet, UdpBuffer &packetBuffer);
        unsigned short getPort() const;

    protected:
        void fragmentPacket(UdpBuffer &packetBuffer);
        bool reassemblePacket(UdpBuffer &fragBuffer, UdpBuffer &destBuffer);

        uint32_t hashPacket(std::vector<uint8_t> *data = nullptr);
        bool verifyPacketIntegrity(UdpPacket &packet);

        int getMaxPacketSize();

    private:
        UdpSocket m_socket;

        uint16_t    m_curSequence;
        float       m_curLocalTime;
        std::map< NetAddress, std::pair<float, std::vector<FragmentedPacket> > > m_fragPacketsBuffer;

    public:
        static const int MAX_FRAGBUFFER_ENTRIES;
        static const float MAX_FRAGPACKET_LIFESPAN;
        static const float MAX_KEEPFRAGPACKETSPERCLIENT_TIME;

};

}

#endif // UDPPACKETSEXCHANGER_H