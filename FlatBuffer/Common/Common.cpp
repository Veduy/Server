#include "Common.h"

int RecvPacket(SOCKET Socket, char* Buffer)
{
	//Header를 다 받을때까지 기다림
	int PacketSize = 0;
	int RecvBytes = recv(Socket, (char*)&PacketSize, sizeof(PacketSize), MSG_WAITALL);

	if (RecvBytes <= 0)
	{
		goto End;
	}

	PacketSize = ntohl(PacketSize);

End:
	return RecvBytes = recv(Socket, Buffer, PacketSize, MSG_WAITALL);
}

int SendPacket(SOCKET Socket, const flatbuffers::FlatBufferBuilder& Builder)
{
	int PacketSize = (int)Builder.GetSize();
	int NetPacketSize = htonl(PacketSize);

	int SentBytes = send(Socket, (char*)&NetPacketSize, sizeof(PacketSize), 0);
	if (SentBytes <= 0)
	{
		goto SentEnd;
	}

	SentBytes = send(Socket, (char*)Builder.GetBufferPointer(), (int)Builder.GetSize(), 0);

SentEnd:

	return SentBytes;
}