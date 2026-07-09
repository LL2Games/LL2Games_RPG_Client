#pragma once

#include <string>
#include <vector>
#include <optional>

#include "Packet.h"

class PacketParser
{
public:
	static std::string MakeBody(const std::vector<std::string>& datas);
	static std::string MakePacket(uint16_t type, const std::string& body);
	static std::optional<ParsedPacket> Parse(std::vector<char>& buf);

};

