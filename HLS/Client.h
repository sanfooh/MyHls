#pragma once
#include "PacketWrapper.h"
#include "TSSegment.h"
#include "Util.h"
#include "ICamera.h"
/// <summary>
/// 一个前端连接,包括网页 APP VLC等
/// </summary>
class Client
{
private:
	/// <summary>
	/// The out ts segs
	/// </summary>
	TsSegBuffer outTsSegs;
	/// <summary>
	/// The out ts seg maximum count
	/// </summary>
	int outTsSegMaxCount;
	/// <summary>
	/// The ts total count
	/// </summary>
	size_t tsTotalCount = 0;
	/// <summary>
	/// The ret
	/// </summary>
	int ret;
public:

	/// <summary>
	/// The client name,a GUID use inner
	/// </summary>
	string clientName;
	/// <summary>
	/// The last activity time,for detect timeout
	/// </summary>
	chrono::steady_clock::time_point lastActivityTime;
	/// <summary>
	/// mongoose connect
	/// </summary>
	mg_connection *c;
	/// <summary>
	/// tag is the connect logo,which is for add and public
	/// </summary>
	string tag;
	/// <summary>
	/// The camera,representative a camera who provide rtsp stream 
	/// </summary>
	ICamera * camera = NULL;
	/// <summary>
	/// The current in packet index
	/// </summary>
	size_t currentInPacketIndex;


	/// <summary>
	/// Initializes a new instance of the <see cref="Client"/> class.
	/// </summary>
	/// <param name="camera">The camera.</param>
	/// <param name="clientName">Name of the client.</param>
	/// <param name="tsSegCount">The ts seg count.</param>
	/// <param name="tag">The tag.</param>
	Client(ICamera * camera, const char* clientName, int tsSegCount, string tag)
	{
		printf("%s client create\n", clientName);
		this->camera = camera;
		this->clientName = clientName;
		this->outTsSegMaxCount = tsSegCount;
		this->tag = tag;
		outTsSegs.SetSegCountMax(tsSegCount);
		lastActivityTime = chrono::steady_clock::now();
	}

	/// <summary>
	/// Finalizes an instance of the <see cref="Client"/> class.
	/// </summary>
	~Client()
	{
		printf("%s client quit\n", clientName.c_str());
	}

	/// <summary>
	/// Respones the ts.
	/// </summary>
	/// <param name="c">The c.</param>
	/// <param name="fileName">Name of the file.</param>
	void ResponeTs(mg_connection *c, const char* fileName)
	{

		auto ts = outTsSegs.GetTs(fileName);
		mg_printf(c, "HTTP/1.1 200 OK\n"
			"Content-Type:video/mp2t\n"
			"Cache-Control:no-cache\n"
			"Connection:close\n"
			"Access-Control-Allow-Origin:*\n"
			"Content-Length: %d\n\n",
			ts.size());
		mg_send(c, ts.data(), ts.size());
		lastActivityTime = chrono::steady_clock::now();
	}

	/// <summary>
	/// Respones the m3u8.
	/// </summary>
	/// <param name="c">The c.</param>
	/// <param name="isFirstRequest">if set to <c>true</c> [is first request].</param>
	void ResponeM3u8(mg_connection *c, bool isFirstRequest = false)
	{
		if (isFirstRequest)
		{
			HlsPacket packet;
			if (camera->GetTsSeg(packet))
			{
				if (packet.data.size() > 0)
				{
					outTsSegs.Push(packet.duration, tsTotalCount++, packet.data);
				}
				currentInPacketIndex = packet.index;
			}
			else
			{
				Util::Debug("get first ts fail");
			}
		}
		else
		{
			HlsPacket packet;
			if (camera->GetTsSeg(packet, currentInPacketIndex))
			{
				if (packet.data.size() > 0)
				{
					outTsSegs.Push(packet.duration, tsTotalCount++, packet.data);
				}
				currentInPacketIndex = packet.index;
			}
			else
			{
				Util::Debug("get ts fail");
			}
		}

		string m3u8 = outTsSegs.GetM3u8();
		mg_printf(c, "HTTP/1.1 200 OK\n"
			"Content-Type:application/vnd.apple.mpegurl\n"
			"Cache-Control:no-cache\n"
			"Connection:close\n"
			"Access-Control-Allow-Origin:*\n"
			"Content-Length: %d\n\n"
			"%s",
			m3u8.length(), m3u8.c_str());
		lastActivityTime = chrono::steady_clock::now();
	}

	/// <summary>
	/// Gets the information.
	/// </summary>
	/// <returns></returns>
	string GetInfo()
	{
		stringstream ss;
		ss << "client name:" << clientName << " TS buffer:" << outTsSegs.segs.size() << "</br>";
		return ss.str();
	}
};

