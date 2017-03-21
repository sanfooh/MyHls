#pragma once
#include <thread>
#include "Client.h"
/// <summary>
/// 请求连接管理
/// </summary>
class ClientManage
{
	map<string, Client*> clients;
	vector<string> allKeys;
	mutex allKeysLock;

	int deleteTheadGo = 0;
	const int seesionTimeout = 10;//second
	ICamera* camera;
public:


	/// <summary>
	/// Initializes a new instance of the <see cref="ClientManage"/> class.
	/// </summary>
	ClientManage()
	{
		InitialFFMPEG();

		//delete the dead client timing
		thread deleteThead([&]() {
			while (deleteTheadGo == 0)
			{
				DeleteClient();
				Util::SleepMs(10000);
			}
			deleteTheadGo = 2;
		});
		deleteThead.detach();
	}

	~ClientManage()
	{
		deleteTheadGo = 1;
		while (deleteTheadGo != 2)
		{
			Util::SleepMs(100);
		}
	}

	/// <summary>
	/// Sets the camera.
	/// </summary>
	/// <param name="camera">The camera.</param>
	void SetCamera(ICamera* camera)
	{
		this->camera = camera;
	}

	/// <summary>
	/// Adds the client.
	/// </summary>
	/// <param name="guid">The unique identifier.</param>
	/// <param name="client">The client.</param>
	/// <param name="tag">The tag.</param>
	/// <returns></returns>
	bool AddClient(const char* guid, Client** client, string tag)
	{
		//创建一个正常的client
		auto newClient = new Client(camera, guid, 10, tag);
		clients[guid] = newClient;
		*client = newClient;
		allKeysLock.lock();
		allKeys.push_back(guid);
		allKeysLock.unlock();
		return true;
	}

	/// <summary>
	/// Gets the client.
	/// </summary>
	/// <param name="guid">The unique identifier.</param>
	/// <param name="client">The client.</param>
	/// <returns></returns>
	bool GetClient(const char* guid, Client** client)
	{
		auto it = clients.find(guid);
		if (it != clients.end())
		{
			*client = it->second;
			return true;
		}
		return false;
	}

	/// <summary>
	/// Detetes the key.
	/// </summary>
	/// <param name="key">The key.</param>
	void DeteteKey(string key)
	{
		allKeysLock.lock();
		for (auto i = allKeys.begin(); i != allKeys.end();)
		{
			if (*i == key)
			{
				i = allKeys.erase(i);
			}
			else
				i++;
		}
		allKeysLock.unlock();
	}

	/// <summary>
	/// Deletes the client.
	/// </summary>
	void DeleteClient()
	{
		allKeysLock.lock();
		auto copy = allKeys;
		allKeysLock.unlock();
		chrono::steady_clock::time_point end = chrono::steady_clock::now();

		for (auto j = copy.begin(); j != copy.end(); j++)
		{
			auto it = clients.find(*j);
			if (it != clients.end())
			{
				auto client = it->second;
				auto second = (double)(chrono::duration_cast<chrono::microseconds>(end - (client->lastActivityTime)).count())*chrono::microseconds::period::num / chrono::microseconds::period::den;
				if (second > seesionTimeout)
				{
					DeteteKey(client->clientName);
					delete client;
					clients.erase(it);
				}

			}
		}

	}

	/// <summary>
	/// Clears the client.
	/// </summary>
	void ClearClient()
	{
		allKeysLock.lock();
		for (auto j = allKeys.begin(); j != allKeys.end(); j++)
		{
			auto it = clients.find(*j);
			if (it != clients.end())
			{
				delete it->second;
			}
		}
		clients.clear();
		allKeysLock.unlock();

	}

	/// <summary>
	/// Gets the clients count.
	/// </summary>
	/// <returns></returns>
	int GetClientsCount()
	{
		return clients.size();
	}

	/// <summary>
	/// Gets the information.
	/// </summary>
	/// <returns></returns>
	string GetInfo()
	{
		string ret;
		allKeysLock.lock();
		auto copy = allKeys;
		allKeysLock.unlock();
		for (auto i = copy.begin(); i != copy.end(); i++)
		{
			auto it = clients.find(*i);
			if (it != clients.end())
			{
				auto client = it->second;
				ret += client->GetInfo();
			}
		}
		return ret;
	}
};