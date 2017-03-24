// HLS.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "FFMpegHeader.h"
#include "mongoose.h"
#include "Filter.h"
#include "RTSP.h"
#include "ClientManage.h"
#include "TSSegment.h"
#include "PacketWrapper.h"
#include "Camera.h"
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"wsock32.lib")
#pragma comment(lib,"lib/avcodec.lib")
#pragma comment(lib,"lib/avfilter.lib")
#pragma comment(lib,"lib/avformat.lib")
#pragma comment(lib,"lib/avutil.lib")

using namespace std;

/// <summary>
/// The s_http_server_opts
/// </summary>
mg_serve_http_opts s_http_server_opts;
/// <summary>
/// The camera manage
/// </summary>
CameraMangage cameraManage;


const string listenRtspIP="localhost";
const string listenRtspPort = "50010";
const string localIp = "localhost";
const string listenWebPort = "8070";


class MongooseHelper
{
	/// <summary>
	/// Sends the json.
	/// </summary>
	/// <param name="c">The c.</param>
	/// <param name="code">The code.</param>
	/// <param name="ret">The ret.</param>
	/// <param name="data">The data.</param>
	/// <param name="message">The message.</param>
	static void SendJson(mg_connection *c, int code, int ret, const char* data, const char* message)
	{
		stringstream ss;
		ss << "{\"ret\":" << ret << ",\"data\":" << (strlen(data) == 0 ? "{}" : data) << ",\"message\":\"" << message << "\"}";
		mg_printf(c, "HTTP/1.1 %d OK\n"
			"Connection:close\n"
			"Access-Control-Allow-Origin:*\n"
			"Content-Length: %d\n\n"
			"%s", code, ss.str().size(), ss.str().c_str());
	}

	/// <summary>
	/// Ev_handlers the specified c.
	/// </summary>
	/// <param name="c">The c.</param>
	/// <param name="ev">The ev.</param>
	/// <param name="p">The p.</param>
	static void ev_handler(mg_connection *c, int ev, void *p) {
		char tmp[1024] = { 0 };
		if (ev == MG_EV_HTTP_REQUEST) {

			http_message *hm = (http_message *)p;
			auto mg_host = mg_get_http_header(hm, "Host");
			string host(mg_host->p, mg_host->p + mg_host->len);
			auto mg_uri = hm->uri;
			string uri(mg_uri.p, mg_uri.p + mg_uri.len);
			vector<string> args;
			string uriTmp = uri;
			char* ptmp = strtok((char*)uriTmp.c_str(), "/");
			while (ptmp != NULL)
			{
				args.push_back(ptmp);
				ptmp = strtok(NULL, "/");
			}

			if (args.size() < 1)
			{
				return;
			}
			else
				if (args[0] == "Play")
				{
					switch (args.size())
					{
					case 2:
					{
						string guid = Util::GetGuid();
						memset(tmp, 0, 1024);
						sprintf_s(tmp, "http://%s%s/sessionid%s/", host.c_str(), uri.c_str(), guid.c_str());
						mg_http_send_redirect(c, 302, mg_mk_str(tmp), mg_mk_str("Access-Control-Allow-Origin:*"));
					}
					break;
					case 3:
					{
						string& tag = args[1];
						Camera *context;
						if (!cameraManage.Get(tag, &context))
						{
							goto Error;
						}
						string sessionId = args[2].substr(strlen("sessionid"));
						Client *session = NULL;
						if (context->clientManage.GetClient(sessionId.c_str(), &session))
						{
							session->ResponeM3u8(c, false);
						}
						else
						{
							if (context->clientManage.AddClient(sessionId.c_str(), &session, tag))
							{
								session->ResponeM3u8(c, true);
							}
							else
							{
								//throw "add session fail";
								Util::Debug("add client fail");
							}
						}
					}
					break;
					case 4:
					{
						string& tag = args[1];
						Camera *context;
						if (!cameraManage.Get(tag, &context))
						{
							goto Error;
						}
						string sessionId = args[2].substr(strlen("sessionid"));
						if (args[3].find("ts") != string::npos)
						{
							Client *session = NULL;
							if (context->clientManage.GetClient(sessionId.c_str(), &session))
							{
								const char *p = strrchr(uri.c_str(), '/') + 1;
								session->ResponeTs(c, p);
							}
							else
							{
								goto Error;

							}
						}
						else
						{
							goto Error;

						}
					}
					break;
					default:
					Error:
						mg_printf(c, "HTTP/1.1 404 OK\n"
							"Content-Type:application/vnd.apple.mpegurl\n"
							"Cache-Control:no-cache\n"
							"Connection:close\n"
							"Access-Control-Allow-Origin:*\n"
							"Content-Length: 0\n\n");
						break;
					}
				}
				else
					if (args[0] == "AddPull")
					{
						string& tag = args[1];
						string url = Util::Replace(args[2], "%", "/");
						int listenPort;
						Camera* context;
						if (cameraManage.Add(&context, tag, Camera::SteamTypePull, listenPort))
						{
							context->url = url;
							context->tag = tag;
							context->StartPullThread();
							auto data = Util::Format("{\"type\":pull,\"pushUrl\":\"%s\",\"pullUrl\":\"http://%s:%s/Play/%s\"}", url.c_str(), listenRtspIP.c_str(), listenRtspPort.c_str(), tag.c_str());
							SendJson(c, 200, 0, data.c_str(), "");
						}
						else
						{
							SendJson(c, 200, 0, "", "the pull stream already exist");
						}
					}

					else
						if (args[0] == "AddPush")
						{
							string& tag = args[1];
							int listenPort;
							//string url = Util::Replace(args[2], "%", "/");
							Camera* context;
							if (cameraManage.Add(&context, tag,Camera::SteamTypePush, listenPort))
							{
								context->tag = tag;
								context->StartPullThread();
								auto data= Util::Format("{\"type\":push,\"pushUrl\":\"rtsp://%s:%d/live.sdp\",\"pullUrl\":\"http://%s:%s/Play/%s\"}", localIp.c_str(), listenPort,listenRtspIP.c_str(), listenRtspPort.c_str(), tag.c_str());
								SendJson(c, 200, 0, data.c_str(), "connect in 30 seconds");
							}
							else
							{
								SendJson(c, 200, 0, "", "the pull stream already exist");
							}
						}
						else
							if (args[0] == "GetPullList")
							{
								auto json = cameraManage.GetListJson();
								SendJson(c, 200, 1, json.c_str(), "connect in 30 seconds");
							}
							else
								if (args[0] == "GetUsage")
								{
									SendJson(c, 200, 1, "", Util::GetProcessStat().c_str());
								}
								else
									if (args[0] == "GetClients")
									{

										string message = cameraManage.GetInfo();
										SendJson(c, 200, 1, "", message.c_str());
									}
		}
	}

	/// <summary>
	/// Ev_web_handlers the specified c.
	/// </summary>
	/// <param name="c">The c.</param>
	/// <param name="ev">The ev.</param>
	/// <param name="p">The p.</param>
	static void ev_web_handler(mg_connection *c, int ev, void *p)
	{
		if (ev == MG_EV_HTTP_REQUEST)
		{
			mg_serve_http(c, (struct http_message *) p, s_http_server_opts);
		}
	}


public:
	/// <summary>
	/// Listens the specified live port.
	/// </summary>
	/// <param name="livePort">The live port.</param>
	/// <param name="listenWebPort">The web port.</param>
	static void Listen(const char* livePort, const char* webPort)
	{
		mg_mgr mgr, mrg_web;
		mg_connection *c;
		mg_connection *c_web;


		mg_mgr_init(&mgr, NULL);
		c = mg_bind(&mgr, livePort, ev_handler);
		mg_set_protocol_http_websocket(c);


		mg_mgr_init(&mrg_web, NULL);
		c_web = mg_bind(&mrg_web, webPort, ev_web_handler);
		mg_set_protocol_http_websocket(c_web);
		s_http_server_opts.document_root = "player/";
		s_http_server_opts.enable_directory_listing = "yes";

		for (;;) {
			mg_mgr_poll(&mgr, 1);
			mg_mgr_poll(&mrg_web, 1);
		}
		mg_mgr_free(&mgr);
		mg_mgr_free(&mrg_web);
	}
};




/// <summary>
/// _tmains the specified argc.
/// </summary>
/// <param name="argc">The argc.</param>
/// <param name="argv">The argv.</param>
/// <returns></returns>
int _tmain(int argc, _TCHAR* argv[])
{
	vector<string> ipcs;
	//ipcs.push_back("rtsp://admin:12345@192.168.1.57");
	//ipcs.push_back("rtsp://admin:12345@192.168.1.48");
	//ipcs.push_back("rtsp://admin:12345@192.168.1.61");
	//ipcs.push_back("rtsp://admin:12345@192.168.1.56");
	//ipcs.push_back("rtsp://admin:12345@192.168.1.55");
	//ipcs.push_back("rtsp://admin:12345@192.168.1.66");
	//ipcs.push_back("rtsp://admin:12345@192.168.1.63");
	//ipcs.push_back("rtsp://admin:12345@192.168.1.49");
	//ipcs.push_back("rtsp://admin:12345@192.168.1.67");
	//ipcs.push_back("rtsp://admin:12345@192.168.1.53");
	//ipcs.push_back("rtsp://admin:12345@192.168.1.60");
	//ipcs.push_back("rtsp://admin:12345@192.168.1.58");
	//ipcs.push_back("rtsp://admin:12345@192.168.1.50");
	//ipcs.push_back("rtsp://admin:12345@192.168.1.59");
	//ipcs.push_back("rtsp://admin:12345@192.168.1.43");
	//ipcs.push_back("rtsp://admin:12345@192.168.1.47");
	//ipcs.push_back("rtsp://admin:a88888888@192.168.1.36");
	//ipcs.push_back("rtsp://admin:a88888888@192.168.1.35");
	//ipcs.push_back("rtsp://admin:12345@192.168.1.64");
	//ipcs.push_back("test");
	//int j = 0;
	//for (auto i = ipcs.begin(); i != ipcs.end(); i++)
	//{
	//	char tag[100] = { 0 };
	//	sprintf(tag, "%d", j++);
	//	Camera * context;
	//	if (cameraManage.Add(&context, tag,false))
	//	{
	//		//context->url = (*i);
	//		context->tag = tag;
	//		context->StartPullThread();
	//	}
	//}

	MongooseHelper::Listen(listenRtspPort.c_str(), listenWebPort.c_str());
	return 1;
}