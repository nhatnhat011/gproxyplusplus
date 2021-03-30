#include <cpp-base64/base64.h>

#include "bonjour.h"
#include "util.h"


CBonjour :: CBonjour()
{
	//DNSServiceRegister(&client, 0, kDNSServiceInterfaceIndexAny, "_blizzard", "_udp.local", "", NULL, (uint16_t)6112, 0, "", nullptr, NULL);

	int err = DNSServiceCreateConnection(&client);
	if (err) CONSOLE_Print("[MDNS] DNSServiceCreateConnection failed: " + UTIL_ToString(err) + "\n");

	CONSOLE_Print("[MDNS] DNS registration finished\n");
}
CBonjour :: ~CBonjour()
{
	if (client) DNSServiceRefDeallocate(client);
}

void CBonjour :: Broadcast_Info(bool TFT, unsigned char war3Version, BYTEARRAY mapGameType, BYTEARRAY mapFlags, BYTEARRAY mapWidth, BYTEARRAY mapHeight, string gameName, string hostName, uint32_t hostTime, string mapPath, BYTEARRAY mapCRC, uint32_t slotsTotal, uint32_t slotsOpen, uint16_t port, uint32_t hostCounter, uint32_t entryKey, BYTEARRAY mapSHA1)
{

	DNSServiceRef service1 = client;
	std::string tmp = std::string("_blizzard._udp") + (TFT ? ",_w3xp27" : ",_war327") + UTIL_ToHexString(war3Version + 16);
	const char* temp = tmp.c_str();
	int err = DNSServiceRegister(&service1, kDNSServiceFlagsShareConnection, kDNSServiceInterfaceIndexAny, gameName.c_str(),
		temp, "local", NULL, ntohs(8151), 0, NULL, nullptr, NULL);
	if (err) CONSOLE_Print("[MDNS] DNSServiceRegister 1 failed: " + UTIL_ToString(err) + "\n");

	std::string players_num = UTIL_ToString(1);
	std::string players_max = UTIL_ToString(slotsTotal);
	std::string secret = UTIL_ToString(entryKey);
	std::string time = UTIL_ToString(hostTime);
	std::string statstringD = std::string((char*)mapFlags.data(), mapFlags.size()) + std::string("\x00",1) + 
		std::string((char*)mapWidth.data(), mapWidth.size()) +
		std::string((char*)mapHeight.data(), mapHeight.size()) +
		std::string((char*)mapCRC.data(), mapCRC.size()) + mapPath + std::string("\0", 1) +
		hostName + std::string("\0\0", 2) + std::string((char*)mapSHA1.data(), mapSHA1.size());


	std::string statstringE;
	uint16_t size = statstringD.size();
	unsigned char Mask = 1;

	for (unsigned int i = 0; i < size; ++i)
	{
		if ((statstringD[i] % 2) == 0)
			statstringE.push_back(statstringD[i] + 1);
		else
		{
			statstringE.push_back(statstringD[i]);
			Mask |= 1 << ((i % 7) + 1);
		}

		if (i % 7 == 6 || i == size - 1)
		{
			statstringE.insert(statstringE.end() - 1 - (i % 7), Mask);
			Mask = 1;
		}
	}

	std::string game_data_d;
	if (war3Version == 32)
		game_data_d = gameName + std::string("\0\0", 2) + statstringE + std::string("\0", 1) + (char)slotsTotal +
			std::string("\0\0\0", 3) + std::string("\x01\x20\x43\x00", 4) +
			std::string(1, port % 0x100) + std::string(1, port >> 8);
	else
		game_data_d = std::string("\x01\x00\x00\x00", 4) + statstringE + std::string("\0", 1) + (char)slotsTotal +
			std::string("\0\0\0", 3) + gameName + std::string("\0\0", 2) +
			std::string(1, port % 0x100) + std::string(1, port >> 8);

	std::string game_data = base64_encode((unsigned char*)game_data_d.c_str(), game_data_d.size());

	std::string w66;
	if (war3Version != 30)
		w66	= std::string("\x0A") + ((char)gameName.size()) + gameName + std::string("\x10\0", 2) +
		"\x1A" + (char)(15 + players_num.size()) + "\x0A\x0Bplayers_num\x12" + (char)players_num.size() + players_num +
		"\x1A" + (char)(9 + gameName.size()) + "\x0A\x05_name\x12" + (char)gameName.size() + gameName +
		"\x1A" + (char)(15 + players_max.size()) + "\x0A\x0Bplayers_max\x12" + (char)players_max.size() + players_max + 
		"\x1A" + (char)(20 + time.size()) + "\x0A\x10game_create_time\x12" + (char)time.size() + time +
		"\x1A\x0A\x0A\x05_type\x12\x01\x31" + "\x1A\x0D\x0A\x08_subtype\x12\x01\x30" +
		"\x1A" + (char)(15 + secret.size()) + "\x0A\x0Bgame_secret\x12" + (char)secret.size() + secret +
		"\x1A" + (char)(14 + game_data.size()) + "\x01\x0A\x09game_data\x12" + (char)game_data.size() + "\x01" + game_data +
		"\x1A\x0C\x0A\x07game_id\x12\x01" + ((war3Version == 32) ? "1":"2") +
		"\x1A\x0B\x0A\x06_flags\x12\x01\x30";
	else
		w66 = std::string("\x0A") + ((char)gameName.size()) + gameName + std::string("\x10\0", 2) +
		"\x1A" + (char)(15 + secret.size()) + "\x0A\x0Bgame_secret\x12" + (char)secret.size() + secret +
		"\x1A\x0A\x0A\x05_type\x12\x01\x31" + "\x1A\x0D\x0A\x08_subtype\x12\x01\x30" +
		"\x1A\x0C\x0A\x07game_id\x12\x01" + "1" +
		"\x1A" + (char)(9 + gameName.size()) + "\x0A\x05_name\x12" + (char)gameName.size() + gameName +
		"\x1A" + (char)(15 + players_max.size()) + "\x0A\x0Bplayers_max\x12" + (char)players_max.size() + players_max +
		"\x1A\x0B\x0A\x06_flags\x12\x01\x30" +
		"\x1A" + (char)(15 + players_num.size()) + "\x0A\x0Bplayers_num\x12" + (char)players_num.size() + players_num +
		"\x1A" + (char)(20 + time.size()) + "\x0A\x10game_create_time\x12" + (char)time.size() + time +
		"\x1A" + (char)(14 + game_data.size()) + "\x01\x0A\x09game_data\x12" + (char)game_data.size() + "\x01" + game_data;
	
	DNSRecordRef record;
	err = DNSServiceAddRecord(service1, &record, 0, 66, w66.size(), w66.c_str(), 0);
	if (err) CONSOLE_Print("[MDNS] DNSServiceAddRecord 1 failed: " + UTIL_ToString(err) + "\n");
	return;
}