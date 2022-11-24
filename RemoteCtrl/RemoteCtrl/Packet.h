#pragma once
#include "pch.h"
#include "framework.h"
#pragma pack(push) //让封装类对齐
#pragma pack(1)
class CPacket {
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else {
			strData.clear();
		}

		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sSum += BYTE(strData[j]) & 0xFF;
		}
		TRACE("pack sSum:%d\r\n", sSum);
	}
	//BYTE = unsigned char ,size_t =unsigned int ;
	//nSize 作为传入传出参数，传入时为data的长度，传出是用掉了多少
	CPacket(const BYTE* pData, size_t& nSize)
	{
		size_t i = 0;
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) {
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i);
		i += 4;
		if (nLength + i > nSize) {
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i);
		i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i);
		i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;
			return;
		}
		nSize = 0;
	}
	~CPacket() {}
	CPacket& operator=(const CPacket& pack) {
		if (this != &pack) {
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}
	int Size() {
		//求包的长度
		return nLength + 6;
	}
	const char* Data() {
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		//TRACE("nLength:%d\r\n", nLength);
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		//TRACE("sSum:%d\r\n", sSum);
		return strOut.c_str();
	}
public:
	// WORD = unsigned short（2个字节）  DWORD = unsigend long（4个字节）
	WORD sHead;       //数据包包头 固定FEFF
	DWORD nLength;    //数据长度，从控制命令开始到和校验结束
	WORD sCmd;        //控制命令
	std::string strData; // 包数据 
	WORD sSum;           // 和校验 除了包头和长度以外的数据加起来
	std::string strOut;  /*由于strData也是一个对象，所以在输出数据的时候是地址，
						 所以需要在类中声明一个自己缓冲区，去存储整个包的数据*/
};
#pragma pack(pop)
typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		pointXY.x = 0;
		pointXY.y = 0;
	}
	WORD nAction;// 移动、双击、单击
	WORD nButton;//左键、中建、右键
	POINT pointXY;//坐标
}MOUSEEV, * PMOUSEEV;
typedef struct file_info {
	file_info() {
		Isinvalid = FALSE;
		IsDirectory = -1;
		HasNext = TRUE;
		memset(FileName, 0, sizeof(FileName));
	}
	BOOL Isinvalid;
	BOOL IsDirectory;
	BOOL HasNext;
	char FileName[256];
}FILEINFO, * PFILEINFO;