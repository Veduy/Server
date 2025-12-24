#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

#include <iostream>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

class Data
{
public:
	int A;
	int B;
	char* Buffer;
	std::string BufferS;
};

using namespace rapidjson;

int main()
{
    Data data;
    data.A = 1;
    data.B = 2;
    memcpy(data.Buffer, "this is char", sizeof(data.Buffer));
    data.BufferS = "this is string";

    const char* temp;
    memcpy(&temp, &data, sizeof(Data));
  
    // 1. Parse a JSON string into DOM.
    const char* json = "{\"project\":\"rapidjson\",\"stars\":10}";
    Document d;
    d.Parse(json);

    // 2. Modify it by DOM.
    Value& s = d["stars"];
    s.SetInt(s.GetInt() + 1);
 
    d.AddMember("gold", 1000, d.GetAllocator());

    // 3. Stringify the DOM
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);

    // Output {"project":"rapidjson","stars":11}
    std::cout << buffer.GetString() << std::endl;
    return 0;
	
}