#pragma once

#define NOMINMAX

#include <WinSock2.h>
#include <Windows.h>
#include <string>

#include "UserEvents_generated.h"
#include "MatchingEvents_generated.h"

#include "flatbuffers/flatbuffers.h"

#pragma comment(lib, "ws2_32")

// 서버가 쓰레드를 여러개 가지고 있고, 필요할때마다 꺼내 쓰는 방식으로 설계한다면?
int SendPacket(SOCKET Socket, flatbuffers::FlatBufferBuilder& Builder);
int RecvPacket(SOCKET Socket, char* Buffer);

// Helper function to create JSON response (minimal implementation)
std::string CreateJsonResponse(const std::string& name, bool result);