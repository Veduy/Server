#include "ChatPacket.h"

std::string ChatPacket::ToJsonString()
{
	//JsonDocument.AddMember("UserName", UserName, JsonDocument.GetAllocator());
	//JsonDocument.AddMember("Message", Message, JsonDocument.GetAllocator());
	JsonDocument.SetObject();

	auto& Allocator = JsonDocument.GetAllocator();
	
	JsonDocument.AddMember(
		"UserName",
		rapidjson::Value(UserName.c_str(), Allocator),
		Allocator
	);

	JsonDocument.AddMember(
		"Message",
		rapidjson::Value(Message.c_str(), Allocator),
		Allocator
	);
	
	rapidjson::StringBuffer Buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(Buffer);
	
	JsonDocument.Accept(writer);

	const char* temp = Buffer.GetString();
	return temp;
}

ChatPacket::ChatPacket()
{
	JsonDocument.SetObject();
}

void ChatPacket::Parse(std::string JsonString)
{
	JsonDocument.Parse(JsonString.data());

	UserName = JsonDocument["UserName"].GetString();
	Message = JsonDocument["Message"].GetString();
}

int ChatPacket::Length()
{
	return (int)ToJsonString().length();
}
