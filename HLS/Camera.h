#pragma once
#include "FFMpegHeader.h"
#include <string>
#include "PacketWrapper.h"
#include <mutex>
#include <queue>
#include "ICamera.h"
#include <chrono>
using namespace std;


/// <summary>
/// representative a camera who provide rtsp stream 
/// </summary>
class Camera :public ICamera
{
	/// <summary>
	/// The packet
	/// </summary>
	AVPacket packet;
	/// <summary>
	/// The ic
	/// </summary>
	AVFormatContext *ic = NULL;
	/// <summary>
	/// The ret
	/// </summary>
	int ret = 0;
	/// <summary>
	/// The video stream index
	/// </summary>
	int videoIndex = 0;
	/// <summary>
	/// The try connect count maximum
	/// </summary>
	const int tryConnectCountMax = 3;
public:
	/// <summary>
	/// The URL
	/// </summary>
	string url = "";
	/// <summary>
	/// The tag
	/// </summary>
	string tag;
	/// <summary>
	/// The parameters
	/// </summary>
	AVCodecParameters * parameters;
	/// <summary>
	/// The is connect
	/// </summary>
	bool isConnect = false;
	/// <summary>
	/// The try connect count
	/// </summary>
	int tryConnectCount = 0;
	/// <summary>
	/// The connect fail
	/// </summary>
	bool connectFail = false;
	/// <summary>
	/// The client manage
	/// </summary>
	ClientManage clientManage;
	/// <summary>
	/// The close event
	/// </summary>
	CloseEvent closeEvent;

	/// <summary>
	/// The in packets
	/// </summary>
	vector<HlsPacket> inPackets;
	/// <summary>
	/// The in packets locker
	/// </summary>
	mutex inPacketsLocker;
	/// <summary>
	/// The in packets maximum size
	/// </summary>
	int inPacketsMaxSize = 100;

	/// <summary>
	/// The BSF context
	/// </summary>
	AVBSFContext * bsfContext = NULL;
	/// <summary>
	/// The oc
	/// </summary>
	AVFormatContext* oc = NULL;
	/// <summary>
	/// The ts io buffer
	/// </summary>
	unsigned char * tsIoBuffer = NULL;
	/// <summary>
	/// The ts io buffer length
	/// </summary>
	const int tsIoBufferLen = 4000;
	/// <summary>
	/// The ts data
	/// </summary>
	vector<unsigned char> tsData;
	/// <summary>
	/// The parse packet count
	/// </summary>
	int parsePacketCount = 100;
	/// <summary>
	/// The millisecondtimebase
	/// </summary>
	AVRational MILLISECONDTIMEBASE = AVRational{ 1, 1000 };
	/// <summary>
	/// The packet count
	/// </summary>
	size_t packetCount = 0;

	/// <summary>
	/// Initializes a new instance of the <see cref="Camera"/> class.
	/// </summary>
	Camera()
	{
		clientManage.SetCamera(this);
	}

	/// <summary>
	/// Connects this instance.
	/// </summary>
	/// <returns></returns>
	bool Connect()
	{
		if (ic != NULL)
		{
			avformat_free_context(ic);
		}
		ic = avformat_alloc_context();
		AVDictionary* options = NULL;
		av_dict_set(&options, "rtsp_transport", "tcp", 0);
		av_dict_set(&options, "stimeout", "6000", 0);
		ret = avformat_open_input(&ic, url.c_str(), NULL, &options);
		if (ret < 0)
		{
			Util::Debug("avformat_open_input from %s fail ret is %s", tag.c_str(), PrintfFfmpegError(ret).c_str());
			return false;
		}
		ic->probesize = 4096;
		ic->flags |= AVFMT_FLAG_NOBUFFER;
		Util::Debug("start find streaminfo from %", url.c_str());
		ret = avformat_find_stream_info(ic, NULL);

		if (ret < 0)
		{
			Util::Debug("start find streaminfo from % fail ret is %s", tag.c_str(), PrintfFfmpegError(ret).c_str());
			return false;
		}
		videoIndex = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
		if (videoIndex < 0)
		{
			return false;
		}
		parameters = ic->streams[videoIndex]->codecpar;


		Util::Debug("start find streaminfo from %s sueccess", tag.c_str());
		isConnect = true;
		return true;
	}


	/// <summary>
	/// Writes the data.
	/// </summary>
	/// <param name="buf">The buf.</param>
	/// <param name="bufsize">The bufsize.</param>
	void WriteData(uint8_t *buf, unsigned short bufsize)
	{
		size_t size = tsData.size();
		tsData.resize(size + bufsize);
		memcpy(tsData.data() + size, buf, bufsize);
	}

	/// <summary>
	/// Fills the iobuffer.ffmpeeg use
	/// </summary>
	/// <param name="opaque">The opaque.</param>
	/// <param name="buf">The buf.</param>
	/// <param name="bufsize">The bufsize.</param>
	/// <returns></returns>
	static int FillIobuffer(void * opaque, uint8_t *buf, int bufsize) {
		auto camera = (Camera*)opaque;
		camera->WriteData(buf, bufsize);
		return bufsize;
	}

	/// <summary>
	/// Initials the ts context.
	/// </summary>
	void InitialTSContext()
	{
		auto filter = av_bsf_get_by_name("h264_mp4toannexb");
		ret = av_bsf_alloc(filter, &bsfContext);
		ret = avcodec_parameters_copy(bsfContext->par_in, parameters);
		ret = av_bsf_init(bsfContext);
		ret = avformat_alloc_output_context2(&oc, NULL, "mpegts", NULL);
		auto encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
		auto ostream = avformat_new_stream(oc, encoder);
		ostream->index = 0;
		avcodec_parameters_copy(oc->streams[0]->codecpar, parameters);

		tsIoBuffer = (unsigned char *)av_malloc(tsIoBufferLen);
		AVIOContext *avio = avio_alloc_context(tsIoBuffer, tsIoBufferLen, 1, this, NULL, FillIobuffer, NULL);
		oc->pb = avio;
		oc->flags = AVFMT_FLAG_CUSTOM_IO;
	}

	/// <summary>
	/// Gets the ts seg.first package to accelerate
	/// </summary>
	/// <param name="packet">The packet.</param>
	/// <returns></returns>
	bool GetTsSeg(HlsPacket& packet)
	{
		if (inPackets.empty())
		{
			return false;
		}

		bool ret = true;
		inPacketsLocker.lock();
		printf("");
		auto it = inPackets.rbegin();
		float duration = 0.0;
		bool findKey = false;
		int count = 0;
		while (true)
		{
			if (it == inPackets.rend())
			{
				inPacketsLocker.unlock();
				return false;
			}
			else
				if (it->isKey&AV_PKT_FLAG_KEY)
				{
					break;
				}
			it++;
		}

		while (true)
		{
			packet.index = it->index;
			packet.data.insert(packet.data.end(), it->data.begin(), it->data.end());
			duration += it->duration;
			if (it == inPackets.rbegin())
			{
				break;
			}
			it--;
		}


		packet.duration = duration;
		inPacketsLocker.unlock();
		if (!ret)
		{
			Util::Debug("to index %d,inPackets size %d", packet.index, inPackets.size());
		}
		return ret;
	}

	/// <summary>
	/// Gets the ts seg.
	/// </summary>
	/// <param name="packet">The packet.</param>
	/// <param name="from">From.</param>
	/// <returns></returns>
	bool GetTsSeg(HlsPacket& packet, size_t from)
	{
		bool ret = false;
		inPacketsLocker.lock();
		float duration = 0.0;
		int count = 0;
		if (inPackets.size() > 1)
		{
			auto it = inPackets.begin();
			while (it != inPackets.end())
			{
				if (it->index > from)
				{
					packet.data.insert(packet.data.end(), it->data.begin(), it->data.end());
					duration += it->duration;
					ret = true;
					if (count++ > 30)
					{
						break;
					}
				}
				packet.index = it->index;
				it++;
			}
		}
		packet.duration = duration;
		inPacketsLocker.unlock();
		if (!ret)
		{
			Util::Debug("from index %d,to index %d,inPackets size %d", from, packet.index, inPackets.size());
		}
		if (count<2)
		{
			Util::Debug("count %d", count);
			ret = false;
		}
		return ret;
	}

	/// <summary>
	/// Creates the ts.
	/// </summary>
	/// <param name="packet">The packet.</param>
	void CreateTs(AVPacket& packet)
	{
		float tsDuration = 0.0;
		bool hasKeyFrame = false;
		if (bsfContext == NULL)
		{
			InitialTSContext();
		}
		tsData.clear();
		tsDuration = 0;
		ret = avformat_write_header(oc, NULL);
		AVPacket tmpPacket = { 0 };

		av_packet_ref(&tmpPacket, &packet);
		ret = av_bsf_send_packet(bsfContext, &tmpPacket);
		while ((ret = av_bsf_receive_packet(bsfContext, &tmpPacket)) == 0)
		{
			auto duration = ((float)av_rescale_q(tmpPacket.duration, AVRational{ 1, 90000 }, MILLISECONDTIMEBASE)) / 1000;
			tsDuration += duration;
			ret = av_interleaved_write_frame(oc, &tmpPacket);
			av_packet_unref(&tmpPacket);
		}


		ret = av_write_trailer(oc);
		HlsPacket hlsPacket;
		hlsPacket.data = tsData;
		hlsPacket.duration = tsDuration;
		hlsPacket.index = packetCount++;
		hlsPacket.isKey = packet.flags&AV_PKT_FLAG_KEY;

		av_packet_unref(&packet);
		inPacketsLocker.lock();
		inPackets.push_back(hlsPacket);
		if (inPackets.size() > inPacketsMaxSize)
		{
			inPackets.erase(inPackets.begin());
		}
		inPacketsLocker.unlock();

	}

	/// <summary>
	/// Pulls this instance.
	/// </summary>
	void Pull()
	{
		while (!closeEvent.isQuit)
		{
			if (!isConnect)
			{
				tryConnectCount++;
				if (Connect())
				{
					Util::Debug("%s connect success try times:%d", this->tag.c_str(), tryConnectCount);
				}
				else
				{
					Util::Debug("%s connect fail try times:%d", this->tag.c_str(), tryConnectCount);
					if (tryConnectCount > tryConnectCountMax)
					{
						Util::Debug("%s connect try times:%d>max count,thread quit", this->tag.c_str(), tryConnectCount);
						connectFail = true;
						break;
					}

				}
			}

			if (isConnect)
			{
				if (av_read_frame(ic, &packet) == 0)
				{
					if (packet.stream_index == videoIndex)
					{
						CreateTs(packet);
					}
					else
					{
						av_packet_unref(&packet);
					}
				}
				else
				{
					isConnect = false;
					Util::Debug("read frame error");
				}

			}


		}
		closeEvent.CloseNow();
	}

	/// <summary>
	/// Starts the pull thread.
	/// </summary>
	void StartPullThread()
	{
		thread th(&Camera::Pull, this);
		th.detach();
	}

	/// <summary>
	/// Finalizes an instance of the <see cref="Camera"/> class.
	/// </summary>
	~Camera()
	{
		clientManage.ClearClient();
		closeEvent.WaitClose();

	}

};



/// <summary>
/// Camera Mangage
/// </summary>
class CameraMangage
{
	/// <summary>
	/// The pull ins
	/// </summary>
	map<string, Camera*> pullIns;
	/// <summary>
	/// The keys
	/// </summary>
	vector<string> keys;
	/// <summary>
	/// The key lock
	/// </summary>
	mutex keyLock;
	/// <summary>
	/// The ss
	/// </summary>
	stringstream ss;
	/// <summary>
	/// The close event
	/// </summary>
	CloseEvent closeEvent;
public:

	/// <summary>
	/// Initializes a new instance of the <see cref="CameraMangage"/> class.
	/// </summary>
	CameraMangage()
	{
		//定时清理超时client
		thread deleteThead([&]() {
			while (!closeEvent.isQuit)
			{
				auto copy = GetAllKey();
				int clientCount = 0;
				for (auto i = copy.begin(); i != copy.end(); i++)
				{
					auto it = pullIns.find(*i);
					if (it != pullIns.end())
					{
						if (it->second->connectFail)
						{
							if (Delete(*i))
							{
								Util::Debug("delete success");
							}
							else
							{
								Util::Debug("delete fail");
							}
						}
					}
				}
				Util::SleepMs(1000);
			}
			closeEvent.CloseNow();
		});
		deleteThead.detach();

	}
	/// <summary>
	/// Finalizes an instance of the <see cref="CameraMangage"/> class.
	/// </summary>
	~CameraMangage()
	{
		closeEvent.WaitClose();
	}

	/// <summary>
	/// Adds the specified context.
	/// </summary>
	/// <param name="context">The context.</param>
	/// <param name="key">The key.</param>
	/// <returns></returns>
	bool Add(Camera** context, string key)
	{
		auto it = pullIns.find(key);
		if (it != pullIns.end())
		{
			return false;
		}
		*context = new Camera();
		pullIns[key] = *context;
		keyLock.lock();
		keys.push_back(key);
		keyLock.unlock();
		return true;
	}

	/// <summary>
	/// Gets the specified key.
	/// </summary>
	/// <param name="key">The key.</param>
	/// <param name="des">The DES.</param>
	/// <returns></returns>
	bool Get(string key, Camera** des)
	{
		auto it = pullIns.find(key);
		if (it != pullIns.end())
		{
			*des = it->second;
			return true;
		}

		return false;
	}

	/// <summary>
	/// Deletes the specified key.
	/// </summary>
	/// <param name="key">The key.</param>
	/// <returns></returns>
	bool Delete(string key)
	{
		auto it = pullIns.find(key);
		if (it != pullIns.end())
		{
			delete it->second;
			pullIns.erase(it);
			keyLock.lock();
			for (auto i = keys.begin(); i != keys.end();)
			{
				if (*i == key)
				{
					i = keys.erase(i);
				}
				else
					i++;
			}
			keyLock.unlock();
			return true;
		}
		return false;
	}

	/// <summary>
	/// Gets all key.
	/// </summary>
	/// <returns></returns>
	vector<string> GetAllKey()
	{
		keyLock.lock();
		auto copy = keys;
		keyLock.unlock();
		return copy;

	}

	/// <summary>
	/// Gets the list json.
	/// </summary>
	/// <returns></returns>
	string GetListJson()
	{
		string ret = "[";
		auto copy = GetAllKey();
		for (auto i = copy.begin(); i != copy.end(); i++)
		{
			auto it = pullIns.find(*i);
			if (it != pullIns.end())
			{
				if (it->second->isConnect)
				{
					ret += "\"" + it->second->tag + "\",";
				}

			}
		}
		if (ret.length() > 1)
		{
			ret.pop_back();

		}
		ret += "]";
		return ret;
	}

	/// <summary>
	/// Gets the information.
	/// </summary>
	/// <returns></returns>
	string GetInfo()
	{
		ss.clear();
		ss.str("");
		string ret = "";
		auto copy = GetAllKey();
		int clientCount = 0;
		for (auto i = copy.begin(); i != copy.end(); i++)
		{
			auto it = pullIns.find(*i);
			if (it != pullIns.end())
			{
				clientCount += it->second->clientManage.GetClientsCount();
				ss << "camera:" << it->second->tag << " current apcket:" << it->second->packetCount << "</br>" << it->second->clientManage.GetInfo() << "-----------</br>";
			}
		}
		string str = ss.str();
		ss.str("");
		ss << "client count:" << clientCount << "</br>" << str;
		return ss.str();
	}
};