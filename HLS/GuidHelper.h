#ifndef _SUREBUS_GuidHelper_H_
#define _SUREBUS_GuidHelper_H_
#include <string>

#define _FS_MAX_GUID 40

class GuidHelper
{
public:
	GuidHelper();
	GuidHelper(const std::string& strGuid);
	GuidHelper(unsigned char* pBuffer);
	~GuidHelper();


	const char* GetGuid(void);
	const char* GetGuidNoHyphen(void);
	const unsigned char* GetBuffer(void);

private:
	GuidHelper(const GuidHelper& rGuid){};
	GuidHelper& operator=(const GuidHelper& rGuid){ return *this; };
	unsigned char m_pBuffer[16];
	char m_szGuid[_FS_MAX_GUID];

};

#endif