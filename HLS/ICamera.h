#pragma once
#include <vector>
using namespace std;
/// <summary>
/// wrapper for ts packet (for first frame acceleration)
///用于首帧加速
/// </summary>
struct HlsPacket
{
	vector<unsigned char> data;
	size_t index;
	bool isKey;
	float duration = 0.0;
};

/// <summary>
/// break circular reference
/// </summary>
class ICamera
{
public:
	virtual bool GetTsSeg(HlsPacket& packet)=0;
	virtual bool GetTsSeg(HlsPacket& packet,size_t from) = 0;
};
