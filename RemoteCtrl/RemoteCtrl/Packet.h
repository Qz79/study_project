#pragma once
#include "pch.h"
#include "framework.h"
#pragma pack(push) //�÷�װ�����
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
	//nSize ��Ϊ���봫������������ʱΪdata�ĳ��ȣ��������õ��˶���
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
		//����ĳ���
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
	// WORD = unsigned short��2���ֽڣ�  DWORD = unsigend long��4���ֽڣ�
	WORD sHead;       //���ݰ���ͷ �̶�FEFF
	DWORD nLength;    //���ݳ��ȣ��ӿ������ʼ����У�����
	WORD sCmd;        //��������
	std::string strData; // ������ 
	WORD sSum;           // ��У�� ���˰�ͷ�ͳ�����������ݼ�����
	std::string strOut;  /*����strDataҲ��һ������������������ݵ�ʱ���ǵ�ַ��
						 ������Ҫ����������һ���Լ���������ȥ�洢������������*/
};
#pragma pack(pop)
typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		pointXY.x = 0;
		pointXY.y = 0;
	}
	WORD nAction;// �ƶ���˫��������
	WORD nButton;//������н����Ҽ�
	POINT pointXY;//����
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