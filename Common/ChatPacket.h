#pragma once

#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#include "Packet.h"
#include <string>

class ChatPacket : public IPacket
{
public:
	ChatPacket();

	// Inherited via IPacket
	virtual void Parse(std::string JsonString) override;
	virtual int Length() override;
	virtual std::string ToJsonString() override;

public:
	std::string UserName;
	std::string Message;

	rapidjson::Document JsonDocument;
};

