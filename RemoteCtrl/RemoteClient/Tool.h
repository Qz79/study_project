#pragma once

#include<Windows.h>
#include<atlimage.h>
#include<string>

class CTool
{
public:
    static void Dump(BYTE* pData, size_t nSize) {
        std::string strOut;
        for (size_t i = 0; i < nSize; i++) {
            char buf[8] = "";
            if (i > 0 && (i % 16 == 0))strOut += "\n";
            snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);//&0xFF的操作是为了防止符号位产生负数
            strOut += buf;
        }
        strOut += "\n";
        OutputDebugStringA(strOut.c_str());
    }
    static int Bytes2Image(CImage& image, std::string& strBuffer) {
		BYTE* pData = (BYTE*)strBuffer.c_str();
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);//申请内存
		if (hMem == NULL) {
			MessageBox(NULL,_T("内存不足了\r\n"), _T("提示"),MB_OK);
			Sleep(1);
			return -1;
		}
		IStream* pStream = NULL;
		HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
		if (ret == S_OK) {
			ULONG length = 0;
			pStream->Write(pData, strBuffer.size(), &length);
			LARGE_INTEGER bg = { 0 };
			pStream->Seek(bg, STREAM_SEEK_SET, NULL);
			if ((HBITMAP)image != NULL)image.Destroy();
			image.Load(pStream);
		}
    }
};

