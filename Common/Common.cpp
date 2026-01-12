#include "Common.h"

int SendPacket(SOCKET Socket, flatbuffers::FlatBufferBuilder& Builder)
{
    // [size(4Byte-int)][FlatBuffer Payload(~)]
    const int PayloadSize = (int)Builder.GetSize();
    int NetPayloadSize = htonl(PayloadSize);

    int Sent = send(Socket, (char*)(&NetPayloadSize), sizeof(int), 0);
    if (Sent != sizeof(int))
    {
        return SOCKET_ERROR;
    }

    const char* Payload = (const char*)(Builder.GetBufferPointer());

 /*   char Buffer[4096] = { 0 };
    memcpy(Buffer + sizeof(int), Builder.GetBufferPointer(), (int)Builder.GetSize());*/

    int TotalSent = 0;
    while(TotalSent < PayloadSize)
    {
        int Sent = send(Socket, Payload + TotalSent, PayloadSize - TotalSent, 0);

        if (Sent == SOCKET_ERROR || Sent == 0)
        {
            return SOCKET_ERROR;
        }

        TotalSent += Sent;
    }

    return TotalSent;
}

int RecvPacket(SOCKET Socket, char* Buffer)
{
    int PayloadSize = 0;

    // 1. payload size 수신 (4 bytes)
    int Received = 0;
    while (Received < sizeof(int))
    {
        int Recvbytes = recv(Socket, (char*)(&PayloadSize) + Received, sizeof(int) - Received, 0);

        if (Recvbytes <= 0)
        {
            return -1;
        }

        Received += Recvbytes;
    }

    PayloadSize = ntohl(PayloadSize);

    // 2?. payload 수신
    Received = 0;
    while (Received < PayloadSize)
    {
        int Recvbytes = recv(Socket, Buffer + Received, PayloadSize - Received, 0);

        if (Recvbytes <= 0)
        {
            return -1;
        }

        Received += Recvbytes;
    }

    // 성공 시 payload 크기 반환
    return PayloadSize;
}