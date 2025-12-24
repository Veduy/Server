#pragma once
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#include <string>	
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

class IPacket
{
public:
	virtual void Parse(std::string JsonString) = 0;
	virtual std::string ToJsonString() = 0;
	virtual int Length() = 0;
};

