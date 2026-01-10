#include "Common.h"

bool SendPacket(SOCKET Socket, const flatbuffers::FlatBufferBuilder& Builder)
{
    // [size(4Byte-int)][FlatBuffer Payload(~)]
    const int TotalSize = (int)Builder.GetSize() + sizeof(int);
    int NetSize = htonl(TotalSize);

    char Buffer[4096] = { 0 };
    memcpy(Buffer, &NetSize, sizeof(int));
    memcpy(Buffer + sizeof(int), Builder.GetBufferPointer(), (int)Builder.GetSize());

    int SentTotal = 0;
    while(SentTotal < TotalSize)
    {
        int Sent = send(Socket, Buffer + SentTotal, TotalSize - SentTotal, 0);

        if (Sent == SOCKET_ERROR || Sent == 0)
        {
            return false;
        }
    }

    return true;
}

int RecvPacket(SOCKET Socket, char* Buffer)
{
    int TotalRecv = 0;
    int PacketSize = sizeof(int);
    int PayloadSize = 0;
    bool bSizeReceived = false;

    while (TotalRecv < PacketSize)
    { 
        int Recv = recv(Socket, Buffer + TotalRecv, PacketSize - TotalRecv, 0);
        if (Recv <= 0)
        {
            return -1;
        }
        TotalRecv += Recv;

        // 4바이트를 받게되면 -> Payload의 Size를 알게되면
        if (!bSizeReceived && TotalRecv >= sizeof(int))
        {
            memmove(&PayloadSize, Buffer, sizeof(int));
            PayloadSize = ntohl(PayloadSize);

            PacketSize = sizeof(int) + PayloadSize;
            bSizeReceived = true;
        }
    }

    return PacketSize;
}