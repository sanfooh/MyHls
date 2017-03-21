#pragma once
#include "FFMpegHeader.h"
//使用简单的引用计数，让所有的session都共享一个packet的data,除data之外的packet字段,都有各自的副本。
/// <summary>
/// all client share the packet->data,so the last client delete the data
/// </summary>
struct PacketWrapper
{
	AVPacket packet;
	int refcount = 0;
public:
	PacketWrapper(AVPacket& packet)
	{
		this->packet = packet;
	}

	/// <summary>
	/// References this instance.
	/// </summary>
	void Ref()
	{
		refcount++;
	}

	/// <summary>
	/// Uns the reference.
	/// </summary>
	void UnRef()
	{
		refcount--;
		if (refcount == 0)
		{
			if (packet.buf->size > 0)
			{
				av_packet_unref(&packet);
			}
			delete this;
		}
	}
};