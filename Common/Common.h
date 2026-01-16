#pragma once

#define NOMINMAX

#include <WinSock2.h>
#include <Windows.h>

#include "UserEvents_generated.h"
#include "flatbuffers/flatbuffers.h"

#pragma comment(lib, "ws2_32")

int SendPacket(SOCKET Socket, flatbuffers::FlatBufferBuilder& Builder);
int RecvPacket(SOCKET Socket, char* Buffer);