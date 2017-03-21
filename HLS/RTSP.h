#pragma once
#include "mongoose.h"
#include <string>
#include < regex>
#include "mongoose.h"
using namespace std;

//暂时未用 not support now
char const* allowedCommandNames = "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE";

//#define PACKET_BUFFER_END            (unsigned int)0x00000000
//
//
//#define MAX_RTP_PKT_LENGTH     1400
//
//#define DEST_IP                "180.101.59.185"
//#define DEST_PORT            1234
//
//#define H264                    96
//
//typedef struct
//{
//	/**//* byte 0 */
//	unsigned char csrc_len : 4;        /**//* expect 0 */
//	unsigned char extension : 1;        /**//* expect 1, see RTP_OP below */
//	unsigned char padding : 1;        /**//* expect 0 */
//	unsigned char version : 2;        /**//* expect 2 */
//										  /**//* byte 1 */
//	unsigned char payload : 7;        /**//* RTP_PAYLOAD_RTSP */
//	unsigned char marker : 1;        /**//* expect 1 */
//										 /**//* bytes 2, 3 */
//	unsigned short seq_no;
//	/**//* bytes 4-7 */
//	unsigned  long timestamp;
//	/**//* bytes 8-11 */
//	unsigned long ssrc;            /**//* stream number is used here. */
//} RTP_FIXED_HEADER;
//
//typedef struct {
//	//byte 0
//	unsigned char TYPE : 5;
//	unsigned char NRI : 2;
//	unsigned char F : 1;
//
//} NALU_HEADER; /**//* 1 BYTES */
//
//typedef struct {
//	//byte 0
//	unsigned char TYPE : 5;
//	unsigned char NRI : 2;
//	unsigned char F : 1;
//
//
//} FU_INDICATOR; /**//* 1 BYTES */
//
//typedef struct {
//	//byte 0
//	unsigned char TYPE : 5;
//	unsigned char R : 1;
//	unsigned char E : 1;
//	unsigned char S : 1;
//} FU_HEADER; /**//* 1 BYTES */
//
//typedef struct
//{
//	int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
//	unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
//	unsigned max_size;            //! Nal Unit Buffer size
//	int forbidden_bit;            //! should be always FALSE
//	int nal_reference_idc;        //! NALU_PRIORITY_xxxx
//	int nal_unit_type;            //! NALU_TYPE_xxxx    
//	char *buf;                    //! contains the first byte followed by the EBSP
//	unsigned short lost_packets;  //! true, if packet loss is detected
//} NALU_t;
//
//
//
//RTP_FIXED_HEADER        *rtp_hdr;
//NALU_HEADER		*nalu_hdr;
//FU_INDICATOR	*fu_ind;
//FU_HEADER		*fu_hdr;
struct RtspRequest
{
	static const int OPTIONS = 0;
	static const int DESCRIBE = 1;
	static const int SETUP = 2;
	static const int PLAY = 3;

	int order;
	string url;
	int cseq;
	string session;
};
//static int info2 = 0, info3 = 0;
class RTSP
{
public:

	static void handleCmd_OPTIONS(mg_connection *c, RtspRequest& request)
	{
		mg_printf(c,
			"RTSP/1.0 200 OK\r\n"
			"Server: sanRTSPServer\r\n"
			"CSeq: %d\r\n"
			"Public: %s\r\n"
			"Content - Length: 0\r\n"
			"Cache - Control : no - cache\r\n\r\n",
			request.cseq, allowedCommandNames);
	}

	static void handleCmd_DESCRIBE(mg_connection *c, RtspRequest& request)
	{

		char sdp[] =
			"v=0\r\n"
			"o=OnewaveUServerNG 1451516402 1025358037 IN IP4 192.168.1.212\r\n"
			"s=/xxx666\r\n"
			"e=NONE\r\n"
			"c=IN IP4 0.0.0.0\r\n"
			"t=0 0\r\n"
			"a=range:clock=19700100T000000Z-\r\n"
			"m=video 10000 RTP/AVP 96\r\n"
			"a=rtpmap:96 H264/90000\r\n"
			"a=fmtp:96 packetization-mode=1; sprop-parameter-sets=Z00AKZpmA8ARPyzUBAQFAAADA+gAAOpgBA==,aO48gA==; profile-level-id=4D0029\r\n"
			"a=control:track0\r\n\r\n";

		/*		"v=0\r\n"
				"o=- 0 0 IN IP4 127.0.0.1\r\n"
				"s=No Name\r\n"
				"t=0 0\r\n"
				"a=tool:libavformat 57.56.101\r\n"
				"m=video 0 RTP/AVP 96\r\n"
				"a=rtpmap:96 H264/90000\r\n"
				"a=fmtp:96 packetization-mode=1; sprop-parameter-sets=Z00AKZpmA8ARPyzUBAQFAAADA+gAAOpgBA==,aO48gA==; profile-level-id=4D0029\r\n"
				"a=control:streamid=0\r\n\r\n";*/

		mg_printf(c,
			"RTSP/1.0 200 OK\r\n"
			"Server:sanRTSPServer\r\n"
			"CSeq: %d\r\n"
			"Content-Length:%d\r\n"
			"Content-Type:application/sdp\r\n\r\n"
			"%s", request.cseq, strlen(sdp), sdp);
	}
	static void handleCmd_SETUP(mg_connection *c, RtspRequest& request)
	{
		mg_printf(c,
			"RTSP/1.0 200 OK\r\n"
			"Server:sanRTSPServer\r\n"
			"CSeq: %d\r\n"
			"Transport: RTP/AVP/TCP;unicast;interleaved=0-1;ssrc=6B8B4567\r\n"
			"Session: %s\r\n\r\n",
			request.cseq,
			request.session.c_str()
		);
	}
	static void handleCmd_PLAY(mg_connection *c, RtspRequest& request)
	{
		mg_printf(c,
			"RTSP/1.0 200 OK \r\n"
			"Server:sanRTSPServer \r\n"
			"CSeq: %d \r\n"
			//"RTP-Info:  \r\n"
			//"url=rtsp://192.168.1.212/1.mpg/track0;seq=9200;rtptime=214793785 \r\n"
			"Range: npt=1489397260.877566-\r\n"
			//"RTP-Info: url=track0;seq=29049;rtptime=2135162\r\n"
			"Session: %s\r\n\r\n",
			request.cseq,
			request.session.c_str()
		);
	}

	static void handleCmd_bad(mg_connection *c)
	{
		mg_printf(c,
			"RTSP/1.0 400 Bad Request\r\nAllow: 8\r\n\r\n",
			allowedCommandNames);
	}




	static string GetRegex(string& str, string restr, int group)
	{
		regex re(restr);
		smatch sm;
		auto ret = sregex_iterator(str.begin(), str.end(), re);
		if (ret != std::sregex_iterator())
		{
			return (*ret)[group].str().c_str();
		}
		else
		{
			return "";
		}
	}


	static int GetOrder(string& str)
	{
		int ret = -1;
		auto order = GetRegex(str, "(OPTIONS|DESCRIBE|SETUP|PLAY)", 1);
		if (order == "OPTIONS")
		{
			ret = RtspRequest::OPTIONS;
		}
		if (order == "DESCRIBE")
		{
			ret = RtspRequest::DESCRIBE;
		}
		if (order == "SETUP")
		{
			ret = RtspRequest::SETUP;
		}
		if (order == "PLAY")
		{
			ret = RtspRequest::PLAY;
		}

		return ret;
	}

	static int GetCSeq(string& str)
	{
		return atoi(GetRegex(str, "CSeq:\\s*(\\d+)", 1).c_str());
	}

	static string GetSession(string& str)
	{
		return GetRegex(str, "Session:\\s*([^\\s]+)", 1);
	}


	static string GetUrl(string& str)
	{
		return GetRegex(str, "(OPTIONS|DESCRIBE|SETUP|PLAY)\\s*(rtsp:[^\\s]+)", 2);
	}

	static RtspRequest ParseRtspRequest(string str)
	{
		RtspRequest request;
		request.order = GetOrder(str);
		request.session = GetSession(str);
		request.cseq = GetCSeq(str);
		request.url = GetUrl(str);
		return request;
	}

};
//
//class CH264_RTP_PACK
//{
//#define RTP_VERSION 2  
//
//	typedef struct NAL_msg_s
//	{
//		bool eoFrame;
//		unsigned char type;     // NAL type  
//		unsigned char *start;   // pointer to first location in the send buffer  
//		unsigned char *end; // pointer to last location in send buffer  
//		unsigned long size;
//	} NAL_MSG_t;
//
//	typedef struct
//	{
//		//LITTLE_ENDIAN  
//		unsigned short   cc : 4;      /* CSRC count                 */
//		unsigned short   x : 1;       /* header extension flag      */
//		unsigned short   p : 1;       /* padding flag               */
//		unsigned short   v : 2;       /* packet type                */
//		unsigned short   pt : 7;      /* payload type               */
//		unsigned short   m : 1;       /* marker bit                 */
//
//		unsigned short    seq;      /* sequence number            */
//		unsigned long     ts;       /* timestamp                  */
//		unsigned long     ssrc;     /* synchronization source     */
//	} rtp_hdr_t;
//
//	typedef struct tagRTP_INFO
//	{
//		NAL_MSG_t   nal;        // NAL information  
//		rtp_hdr_t   rtp_hdr;    // RTP header is assembled here  
//		int hdr_len;            // length of RTP header  
//
//		unsigned char *pRTP;    // pointer to where RTP packet has beem assembled  
//		unsigned char *start;   // pointer to start of payload  
//		unsigned char *end;     // pointer to end of payload  
//
//		unsigned int s_bit;     // bit in the FU header  
//		unsigned int e_bit;     // bit in the FU header  
//		bool FU_flag;       // fragmented NAL Unit flag  
//	} RTP_INFO;
//
//public:
//	CH264_RTP_PACK(unsigned long H264SSRC, unsigned char H264PAYLOADTYPE = 96, unsigned short MAXRTPPACKSIZE = 1472)
//	{
//		m_MAXRTPPACKSIZE = MAXRTPPACKSIZE;
//		if (m_MAXRTPPACKSIZE > 10000)
//		{
//			m_MAXRTPPACKSIZE = 10000;
//		}
//		if (m_MAXRTPPACKSIZE < 50)
//		{
//			m_MAXRTPPACKSIZE = 50;
//		}
//
//		memset(&m_RTP_Info, 0, sizeof(m_RTP_Info));
//
//		m_RTP_Info.rtp_hdr.pt = H264PAYLOADTYPE;
//		m_RTP_Info.rtp_hdr.ssrc = H264SSRC;
//		m_RTP_Info.rtp_hdr.v = RTP_VERSION;
//
//		m_RTP_Info.rtp_hdr.seq = 0;
//	}
//
//	~CH264_RTP_PACK(void)
//	{
//	}
//
//	//传入Set的数据必须是一个完整的NAL,起始码为0x00000001。  
//	//起始码之前至少预留10个字节，以避免内存COPY操作。  
//	//打包完成后，原缓冲区内的数据被破坏。  
//	bool Set(unsigned char *NAL_Buf, unsigned long NAL_Size, unsigned long Time_Stamp, bool End_Of_Frame)
//	{
//		unsigned long startcode = StartCode(NAL_Buf);
//
//		if (startcode != 0x01000000)
//		{
//			return false;
//		}
//
//		int type = NAL_Buf[4] & 0x1f;
//		if (type < 1 || type > 12)
//		{
//			return false;
//		}
//
//		m_RTP_Info.nal.start = NAL_Buf;
//		m_RTP_Info.nal.size = NAL_Size;
//		m_RTP_Info.nal.eoFrame = End_Of_Frame;
//		m_RTP_Info.nal.type = m_RTP_Info.nal.start[4];
//		m_RTP_Info.nal.end = m_RTP_Info.nal.start + m_RTP_Info.nal.size;
//
//		m_RTP_Info.rtp_hdr.ts = Time_Stamp;
//
//		m_RTP_Info.nal.start += 4; // skip the syncword  
//
//		if ((m_RTP_Info.nal.size + 7) > m_MAXRTPPACKSIZE)
//		{
//			m_RTP_Info.FU_flag = true;
//			m_RTP_Info.s_bit = 1;
//			m_RTP_Info.e_bit = 0;
//
//			m_RTP_Info.nal.start += 1; // skip NAL header  
//		}
//		else
//		{
//			m_RTP_Info.FU_flag = false;
//			m_RTP_Info.s_bit = m_RTP_Info.e_bit = 0;
//		}
//
//		m_RTP_Info.start = m_RTP_Info.end = m_RTP_Info.nal.start;
//		m_bBeginNAL = true;
//
//		return true;
//	}
//
//	//循环调用Get获取RTP包，直到返回值为NULL  
//	unsigned char* Get(unsigned short *pPacketSize)
//	{
//		if (m_RTP_Info.end == m_RTP_Info.nal.end)
//		{
//			*pPacketSize = 0;
//			return NULL;
//		}
//
//		if (m_bBeginNAL)
//		{
//			m_bBeginNAL = false;
//		}
//		else
//		{
//			m_RTP_Info.start = m_RTP_Info.end;  // continue with the next RTP-FU packet  
//		}
//
//		int bytesLeft = m_RTP_Info.nal.end - m_RTP_Info.start;
//		int maxSize = m_MAXRTPPACKSIZE - 12;   // sizeof(basic rtp header) == 12 bytes  
//		if (m_RTP_Info.FU_flag)
//			maxSize -= 2;
//
//		if (bytesLeft > maxSize)
//		{
//			m_RTP_Info.end = m_RTP_Info.start + maxSize;   // limit RTP packetsize to 1472 bytes  
//		}
//		else
//		{
//			m_RTP_Info.end = m_RTP_Info.start + bytesLeft;
//		}
//
//		if (m_RTP_Info.FU_flag)
//		{   // multiple packet NAL slice  
//			if (m_RTP_Info.end == m_RTP_Info.nal.end)
//			{
//				m_RTP_Info.e_bit = 1;
//			}
//		}
//
//		m_RTP_Info.rtp_hdr.m = m_RTP_Info.nal.eoFrame ? 1 : 0; // should be set at EofFrame  
//		if (m_RTP_Info.FU_flag && !m_RTP_Info.e_bit)
//		{
//			m_RTP_Info.rtp_hdr.m = 0;
//		}
//
//		m_RTP_Info.rtp_hdr.seq++;
//
//		unsigned char *cp = m_RTP_Info.start;
//		cp -= (m_RTP_Info.FU_flag ? 14 : 12);
//		m_RTP_Info.pRTP = cp;
//
//		unsigned char *cp2 = (unsigned char *)&m_RTP_Info.rtp_hdr;
//		cp[0] = cp2[0];
//		cp[1] = cp2[1];
//
//		cp[2] = (m_RTP_Info.rtp_hdr.seq >> 8) & 0xff;
//		cp[3] = m_RTP_Info.rtp_hdr.seq & 0xff;
//
//		cp[4] = (m_RTP_Info.rtp_hdr.ts >> 24) & 0xff;
//		cp[5] = (m_RTP_Info.rtp_hdr.ts >> 16) & 0xff;
//		cp[6] = (m_RTP_Info.rtp_hdr.ts >> 8) & 0xff;
//		cp[7] = m_RTP_Info.rtp_hdr.ts & 0xff;
//
//		cp[8] = (m_RTP_Info.rtp_hdr.ssrc >> 24) & 0xff;
//		cp[9] = (m_RTP_Info.rtp_hdr.ssrc >> 16) & 0xff;
//		cp[10] = (m_RTP_Info.rtp_hdr.ssrc >> 8) & 0xff;
//		cp[11] = m_RTP_Info.rtp_hdr.ssrc & 0xff;
//		m_RTP_Info.hdr_len = 12;
//		/*!
//		* /n The FU indicator octet has the following format:
//		* /n
//		* /n      +---------------+
//		* /n MSB  |0|1|2|3|4|5|6|7|  LSB
//		* /n      +-+-+-+-+-+-+-+-+
//		* /n      |F|NRI|  Type   |
//		* /n      +---------------+
//		* /n
//		* /n The FU header has the following format:
//		* /n
//		* /n      +---------------+
//		* /n      |0|1|2|3|4|5|6|7|
//		* /n      +-+-+-+-+-+-+-+-+
//		* /n      |S|E|R|  Type   |
//		* /n      +---------------+
//		*/
//		if (m_RTP_Info.FU_flag)
//		{
//			// FU indicator  F|NRI|Type  
//			cp[12] = (m_RTP_Info.nal.type & 0xe0) | 28;  //Type is 28 for FU_A  
//														 //FU header     S|E|R|Type  
//			cp[13] = (m_RTP_Info.s_bit << 7) | (m_RTP_Info.e_bit << 6) | (m_RTP_Info.nal.type & 0x1f); //R = 0, must be ignored by receiver  
//
//			m_RTP_Info.s_bit = m_RTP_Info.e_bit = 0;
//			m_RTP_Info.hdr_len = 14;
//		}
//		m_RTP_Info.start = &cp[m_RTP_Info.hdr_len];    // new start of payload  
//
//		*pPacketSize = m_RTP_Info.hdr_len + (m_RTP_Info.end - m_RTP_Info.start);
//		return m_RTP_Info.pRTP;
//	}
//
//	void test()
//	{
//		
//	}
//
//private:
//	unsigned int StartCode(unsigned char *cp)
//	{
//		unsigned int d32;
//		d32 = cp[3];
//		d32 <<= 8;
//		d32 |= cp[2];
//		d32 <<= 8;
//		d32 |= cp[1];
//		d32 <<= 8;
//		d32 |= cp[0];
//		return d32;
//	}
//
//
//
//private:
//	RTP_INFO m_RTP_Info;
//	bool m_bBeginNAL;
//	unsigned short m_MAXRTPPACKSIZE;
//};



//static void ev_rtsp_handler(struct mg_connection *c, int ev, void *ev_data) {
//	struct mbuf *io = &c->recv_mbuf;
//	switch (ev)
//	{
//	case MG_EV_RECV:
//	{
//		auto request = RTSP::ParseRtspRequest(io->buf);
//		switch (request.order)
//		{
//		case RtspRequest::OPTIONS:
//			RTSP::handleCmd_OPTIONS(c, request);
//			break;
//		case RtspRequest::DESCRIBE:
//			RTSP::handleCmd_DESCRIBE(c, request);
//			break;
//		case RtspRequest::SETUP:
//		{
//			GuidHelper helper;
//			auto guid = helper.GetGuidNoHyphen();
//			request.session = guid;
//			RTSP::handleCmd_SETUP(c, request);
//			break;
//		}
//		case RtspRequest::PLAY:
//		{
//			Client *session = NULL;
//			sessionManage.AddClient(request.session.c_str(), &session, Client::ClientTypeRtsp, "");
//			session->c = c;

//			RTSP::handleCmd_PLAY(c, request);
//			break;
//		}
//		default:
//			RTSP::handleCmd_bad(c);
//			break;
//		}
//		mbuf_remove(io, io->len);

//		break;
//	}
//	case MG_EV_SEND:
//		//Util::Debug("send");
//		break;

//	case MG_EV_CLOSE:
//		//Util::Debug("close");
//		break;
//	default:

//		break;
//	}
//}
