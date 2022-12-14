#include <net/ipv4.hpp>
#include <debug.h>

#include "../kernel.h"

namespace NetworkIPv4
{
    IPv4::IPv4(NetworkARP::ARP *ARP, NetworkEthernet::Ethernet *Ethernet) : NetworkEthernet::EthernetEvents(NetworkEthernet::TYPE_IPV4)
    {
        this->ARP = ARP;
        this->Ethernet = Ethernet;
    }

    IPv4::~IPv4()
    {
    }

    void IPv4::Send(uint8_t *Data, uint64_t Length, uint8_t Protocol, InternetProtocol DestinationIP)
    {
        netdbg("Sending %ld bytes to %s", Length, DestinationIP.v4.ToStringLittleEndian());
        IPv4Packet *Packet = (IPv4Packet *)kmalloc(Length + sizeof(IPv4Header));

        /* This part is the most confusing one for me. */
        Packet->Header.Version = b8(4);
        Packet->Header.IHL = b8(sizeof(IPv4Header) / 4);
        Packet->Header.TypeOfService = b8(0);
        /* We don't byteswap. */
        Packet->Header.TotalLength = Length + sizeof(IPv4Header);
        Packet->Header.TotalLength = ((Packet->Header.TotalLength & 0xFF00) >> 8) | ((Packet->Header.TotalLength & 0x00FF) << 8);

        Packet->Header.Identification = b16(0x0000);
        Packet->Header.Flags = b8(0x0);
        Packet->Header.FragmentOffset = b8(0x0);
        Packet->Header.TimeToLive = b8(64);
        Packet->Header.Protocol = b8(Protocol);
        Packet->Header.DestinationIP = b32(DestinationIP.v4.ToHex());
        Packet->Header.SourceIP = b32(Ethernet->GetInterface()->IP.v4.ToHex());
        Packet->Header.HeaderChecksum = b16(0x0);
        Packet->Header.HeaderChecksum = CalculateChecksum((uint16_t *)Packet, sizeof(IPv4Header));

        memcpy(Packet->Data, Data, Length);
        InternetProtocol DestinationRoute = DestinationIP;
        if ((DestinationIP.v4.ToHex() & SubNetworkMaskIP.v4.ToHex()) != (b32(Packet->Header.SourceIP) & SubNetworkMaskIP.v4.ToHex()))
            DestinationRoute = SubNetworkMaskIP;

        Ethernet->Send(MediaAccessControl().FromHex(ARP->Resolve(DestinationRoute)), this->GetFrameType(), (uint8_t *)Packet, Length + sizeof(IPv4Header));
        kfree(Packet);
    }

    Vector<IPv4Events *> RegisteredEvents;

    bool IPv4::OnEthernetPacketReceived(uint8_t *Data, uint64_t Length)
    {
        IPv4Packet *Packet = (IPv4Packet *)Data;
        netdbg("Received %d bytes [Protocol %ld]", Length, Packet->Header.Protocol);
        if (Length < sizeof(IPv4Header))
        {
            warn("Packet too short");
            return false;
        }

        bool Reply = false;

        if (b32(Packet->Header.DestinationIP) == Ethernet->GetInterface()->IP.v4.ToHex() || b32(Packet->Header.DestinationIP) == 0xFFFFFFFF || Ethernet->GetInterface()->IP.v4.ToHex() == 0)
        {
            uint64_t TotalLength = Packet->Header.TotalLength;
            if (TotalLength > Length)
                TotalLength = Length;

            foreach (auto Event in RegisteredEvents)
                if (Packet->Header.Protocol == Event->GetProtocol())
                {
                    InternetProtocol SourceIP;
                    InternetProtocol DestinationIP;

                    SourceIP.v4 = SourceIP.v4.FromHex(b32(Packet->Header.SourceIP));
                    DestinationIP.v4 = DestinationIP.v4.FromHex(b32(Packet->Header.DestinationIP));

                    if (Event->OnIPv4PacketReceived(SourceIP, DestinationIP, (uint8_t *)((uint64_t)Data + 4 * Packet->Header.IHL), TotalLength - 4 * Packet->Header.IHL))
                        Reply = true;
                }
        }
        else
        {
            netdbg("Not for us. We are %s but this is for %s",
                   Ethernet->GetInterface()->IP.v4.ToHex(),
                   Packet->Header.DestinationIP);
        }

        if (Reply)
        {
            uint32_t SwapIP = Packet->Header.DestinationIP;
            Packet->Header.DestinationIP = Packet->Header.SourceIP;
            Packet->Header.SourceIP = SwapIP;
            Packet->Header.TimeToLive = 0x40;
            Packet->Header.HeaderChecksum = 0x0;
            Packet->Header.HeaderChecksum = CalculateChecksum((uint16_t *)Data, 4 * Packet->Header.TotalLength);
            NIManager->Send(Ethernet->GetInterface(), (uint8_t *)Data, Length);
        }
        return Reply;
    }

    IPv4Events::IPv4Events(IPv4Protocols Protocol)
    {
        this->Protocol = (uint8_t)Protocol;
        RegisteredEvents.push_back(this);
    }

    IPv4Events::~IPv4Events()
    {
        for (uint64_t i = 0; i < RegisteredEvents.size(); i++)
            if (RegisteredEvents[i] == this)
            {
                RegisteredEvents.remove(i);
                break;
            }
    }
}

namespace NetworkIPv6
{

}
