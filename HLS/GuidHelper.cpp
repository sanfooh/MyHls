
#include "GuidHelper.h"

#ifdef WIN32
#include <objbase.h>

#define sscanf sscanf_s
#define sprintf _snprintf_s
#else
#include <sys/param.h>
#include <uuid/uuid.h>
#endif

GuidHelper::GuidHelper()
{
	memset(m_szGuid, 0, _FS_MAX_GUID);

#ifdef WIN32
	GUID randomGuid;
	//´´½¨Ëæ»úGUID
	randomGuid = GUID_NULL;
	::CoCreateGuid(&randomGuid);
	if (randomGuid == GUID_NULL)
	{
		fprintf(stderr, "Couldn't create a random GUID\n");
		return;
	}
	memcpy(m_pBuffer, &randomGuid, 16);
#else
	uuid_t uid; // uuid_t is defined as unsigned char[16]
	uuid_generate(uid);
	memcpy(m_pBuffer, uid, 16);
#endif
}


GuidHelper::GuidHelper(const std::string& strGuid)
{
	memset(m_szGuid, 0, _FS_MAX_GUID);
	unsigned uid[16];
	sscanf(strGuid.c_str(), "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		&uid[0], &uid[1], &uid[2], &uid[3],
		&uid[4], &uid[5], &uid[6], &uid[7],
		&uid[8], &uid[9], &uid[10], &uid[11],
		&uid[12], &uid[13], &uid[14], &uid[15]);
	for (int i = 0; i < 16; i++)
	{
		m_pBuffer[i] = (unsigned char)uid[i];
	}
}

GuidHelper::GuidHelper(unsigned char* pBuffer)
{
	memset(m_szGuid, 0, _FS_MAX_GUID);
	memcpy(m_pBuffer, pBuffer, 16);
}

GuidHelper::~GuidHelper()
{
}


const char* GuidHelper::GetGuid(void)
{
	sprintf(m_szGuid, _FS_MAX_GUID, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		m_pBuffer[0], m_pBuffer[1], m_pBuffer[2], m_pBuffer[3],
		m_pBuffer[4], m_pBuffer[5], m_pBuffer[6], m_pBuffer[7],
		m_pBuffer[8], m_pBuffer[9], m_pBuffer[10], m_pBuffer[11],
		m_pBuffer[12], m_pBuffer[13], m_pBuffer[14], m_pBuffer[15]);

	return m_szGuid;
}

const char* GuidHelper::GetGuidNoHyphen(void)
{
	sprintf(m_szGuid, _FS_MAX_GUID, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		m_pBuffer[0], m_pBuffer[1], m_pBuffer[2], m_pBuffer[3],
		m_pBuffer[4], m_pBuffer[5], m_pBuffer[6], m_pBuffer[7],
		m_pBuffer[8], m_pBuffer[9], m_pBuffer[10], m_pBuffer[11],
		m_pBuffer[12], m_pBuffer[13], m_pBuffer[14], m_pBuffer[15]);

	return m_szGuid;
}



const unsigned char* GuidHelper::GetBuffer(void)
{
	return m_pBuffer;
}
