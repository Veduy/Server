#pragma once

#define NOMINMAX

#include <WinSock2.h>

#include "UserEvents_generated.h"
#include "flatbuffers/flatbuffers.h"

#pragma comment(lib, "ws2_32")

bool SendPacket(SOCKET Socket, const flatbuffers::FlatBufferBuilder& Builder);
int RecvPacket(SOCKET Socket, char* Buffer);