#define _CRT_SECURE_NO_WARNINGS

#include "CasProxy.h"

#include <stdlib.h>
#include <wchar.h>
#include <stdarg.h>
#include <tchar.h>

DWORD g_dwIP = 0x7F000001UL;
WORD g_wPort = 6900UL;

// ファイルローカル変数
static HINSTANCE hinstDLL;

extern "C" {
	LONG WINAPI SCardConnectAHook(SCARDCONTEXT hContext, LPCSTR szReader, DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol);
	LONG WINAPI SCardConnectWHook(SCARDCONTEXT hContext, LPCWSTR szReader, DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol);
	LONG WINAPI SCardDisconnectHook(SCARDHANDLE hCard, DWORD dwDisposition);
	LONG WINAPI SCardEstablishContextHook(DWORD dwScope, LPCVOID pvReserved1, LPCVOID pvReserved2, LPSCARDCONTEXT phContext);
	LONG WINAPI SCardFreeMemoryHook(SCARDCONTEXT hContext, LPVOID pvMem);
	LONG WINAPI SCardGetStatusChangeAHook(SCARDCONTEXT hContext, DWORD dwTimeout, LPSCARD_READERSTATE_A rgReaderStates, DWORD cReaders);
	LONG WINAPI SCardGetStatusChangeWHook(SCARDCONTEXT hContext, DWORD dwTimeout, LPSCARD_READERSTATE_W rgReaderStates, DWORD cReaders);
	LONG WINAPI SCardIsValidContextHook(SCARDCONTEXT hContext);
	LONG WINAPI SCardListReadersAHook(SCARDCONTEXT hContext, LPCSTR mszGroups, LPSTR mszReaders, LPDWORD pcchReaders);
	LONG WINAPI SCardListReadersWHook(SCARDCONTEXT hContext, LPCWSTR mszGroups, LPWSTR mszReaders, LPDWORD pcchReaders);
	LONG WINAPI SCardReleaseContextHook(SCARDCONTEXT hContext);
	LONG WINAPI SCardStatusAHook(SCARDHANDLE hCard, LPSTR szReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen);
	LONG WINAPI SCardStatusWHook(SCARDHANDLE hCard, LPWSTR szReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen);
	LONG WINAPI SCardTransmitHook(SCARDHANDLE hCard, LPCSCARD_IO_REQUEST pioSendPci, LPCBYTE pbSendBuffer, DWORD cbSendLength, LPSCARD_IO_REQUEST pioRecvPci, LPBYTE pbRecvBuffer, LPDWORD pcbRecvLength);
}
extern const SCARD_IO_REQUEST g_rgSCardT1PciHook = { SCARD_PROTOCOL_T1, sizeof(SCARD_IO_REQUEST) };

static const LPSTR szBCASReaderNameA = "BonCasLink Proxy Card Reader\0\0";
static const LPWSTR szBCASReaderNameW = L"BonCasLink Proxy Card Reader\0\0";
static char szNameBufA[64];
static WCHAR szNameBufW[64];
static DWORD dwNameBufLen;

LONG WINAPI SCardConnectAHook(SCARDCONTEXT hContext, LPCSTR szReader, DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol)
{
	// プロキシインスタンス生成
	CCasProxy *pCasProxy = new CCasProxy(NULL);

	// IP,Port設定
	pCasProxy->Setting(g_dwIP, g_wPort);

	// サーバに接続
	if (!pCasProxy->Connect())
	{
		delete pCasProxy;
		*phCard = NULL;
		return SCARD_E_READER_UNAVAILABLE;
	}

	// ハンドルに埋め込む
	*phCard = reinterpret_cast<SCARDHANDLE>(pCasProxy);

	if (pdwActiveProtocol)
		*pdwActiveProtocol = SCARD_PROTOCOL_T1;

	return SCARD_S_SUCCESS;
}

LONG WINAPI SCardConnectWHook(SCARDCONTEXT hContext, LPCWSTR szReader, DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard, LPDWORD pdwActiveProtocol)
{
	// プロキシインスタンス生成
	CCasProxy *pCasProxy = new CCasProxy(NULL);

	// IP,Port設定
	pCasProxy->Setting(g_dwIP, g_wPort);

	// サーバに接続
	if (!pCasProxy->Connect())
	{
		delete pCasProxy;
		*phCard = NULL;
		return SCARD_E_READER_UNAVAILABLE;
	}

	// ハンドルに埋め込む
	*phCard = reinterpret_cast<SCARDHANDLE>(pCasProxy);

	if (pdwActiveProtocol)
		*pdwActiveProtocol = SCARD_PROTOCOL_T1;

	return SCARD_S_SUCCESS;
}

LONG WINAPI SCardDisconnectHook(SCARDHANDLE hCard, DWORD dwDisposition)
{
	// サーバから切断
	CCasProxy *pCasProxy = reinterpret_cast<CCasProxy *>(hCard);
	if (pCasProxy)
		delete pCasProxy;

	return SCARD_S_SUCCESS;
}

LONG WINAPI SCardEstablishContextHook(DWORD dwScope, LPCVOID pvReserved1, LPCVOID pvReserved2, LPSCARDCONTEXT phContext)
{
	return SCARD_S_SUCCESS;
}

LONG WINAPI SCardFreeMemoryHook(SCARDCONTEXT hContext, LPCVOID pvMem)
{
	return SCARD_S_SUCCESS;
}

LONG WINAPI SCardGetStatusChangeAHook(SCARDCONTEXT hContext, DWORD dwTimeout, LPSCARD_READERSTATE_A rgReaderStates, DWORD cReaders)
{
	// 手抜き
	if (lstrcmpA(rgReaderStates->szReader, szBCASReaderNameA) != 0)
		rgReaderStates->dwEventState = SCARD_STATE_IGNORE | SCARD_STATE_CHANGED | SCARD_STATE_UNKNOWN;
	else
		rgReaderStates->dwEventState = SCARD_STATE_PRESENT;
	return SCARD_S_SUCCESS;
}

LONG WINAPI SCardGetStatusChangeWHook(SCARDCONTEXT hContext, DWORD dwTimeout, LPSCARD_READERSTATE_W rgReaderStates, DWORD cReaders)
{
	// 手抜き
	if (lstrcmpW(rgReaderStates->szReader, szBCASReaderNameW) != 0)
		rgReaderStates->dwEventState = SCARD_STATE_IGNORE | SCARD_STATE_CHANGED | SCARD_STATE_UNKNOWN;
	else
		rgReaderStates->dwEventState = SCARD_STATE_PRESENT;
	return SCARD_S_SUCCESS;
}

LONG WINAPI SCardIsValidContextHook(SCARDCONTEXT hContext)
{
	return SCARD_S_SUCCESS;
}

LONG WINAPI SCardListReadersAHook(SCARDCONTEXT hContext, LPCSTR mszGroups, LPSTR mszReaders, LPDWORD pcchReaders)
{
	if (mszReaders)
	{
		if (*pcchReaders == SCARD_AUTOALLOCATE)
		{
			*((void **)mszReaders) = szNameBufA;
			*pcchReaders = dwNameBufLen;
		}
		else
		{
			if (*pcchReaders >= dwNameBufLen)
			{
				memcpy(mszReaders, szNameBufA, dwNameBufLen);
				*pcchReaders = dwNameBufLen;
			}
			else
			{
				DWORD dw = *pcchReaders;
				LPSTR p = szNameBufA;
				while (dw)
				{
					*mszReaders++ = *p++;
					dw--;
				}
			}
		}
	}
	else
		*pcchReaders = dwNameBufLen;
	return SCARD_S_SUCCESS;
}

LONG WINAPI SCardListReadersWHook(SCARDCONTEXT hContext, LPCWSTR mszGroups, LPWSTR mszReaders, LPDWORD pcchReaders)
{
	if (mszReaders)
	{
		if (*pcchReaders == SCARD_AUTOALLOCATE)
		{
			*((void **)mszReaders) = szNameBufW;
			*pcchReaders = dwNameBufLen;
		}
		else
		{
			if (*pcchReaders >= dwNameBufLen)
			{
				memcpy(mszReaders, szNameBufW, dwNameBufLen * sizeof(WCHAR));
				*pcchReaders = dwNameBufLen;
			}
			else
			{
				DWORD dw = *pcchReaders;
				LPWSTR p = szNameBufW;
				while (dw)
				{
					*mszReaders++ = *p++;
					dw--;
				}
			}
		}
	}
	else
		*pcchReaders = dwNameBufLen;
	return SCARD_S_SUCCESS;
}

LONG WINAPI SCardReleaseContextHook(SCARDCONTEXT hContext)
{
	return SCARD_S_SUCCESS;
}

LONG WINAPI SCardStatusAHook(SCARDHANDLE hCard, LPSTR szReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen)
{
	CCasProxy *pCasProxy = reinterpret_cast<CCasProxy *>(hCard);
	if (!pCasProxy)
		return SCARD_E_INVALID_HANDLE;

	LPSTR pReaderName = szBCASReaderNameA;

	if (szReaderName)
	{
		if (*pcchReaderLen == SCARD_AUTOALLOCATE)
		{
			*((void **)szReaderName) = pReaderName;
			*pcchReaderLen = lstrlenA(pReaderName) + 2;
		}
		else
		{
			int len = lstrlenA(pReaderName) + 2;
			if (*pcchReaderLen >= (DWORD)len)
			{
				memcpy(szReaderName, pReaderName, len);
				*pcchReaderLen = len;
			}
			else
			{
				DWORD dw = *pcchReaderLen;
				LPSTR p = pReaderName;
				while (dw)
				{
					*szReaderName++ = *p++;
					dw--;
				}
			}
		}
	}
	else
		*pcchReaderLen = lstrlenA(pReaderName) + 2;
	if (pdwState)
		*pdwState = SCARD_PRESENT;
	if (pdwProtocol)
		*pdwProtocol = SCARD_PROTOCOL_T1;
	if (pcbAtrLen)
	{
		if (*pcbAtrLen == SCARD_AUTOALLOCATE)
			*((LPBYTE *)pbAtr) = (LPBYTE)"";
		*pcbAtrLen = 0;
	}
	return SCARD_S_SUCCESS;
}

LONG WINAPI SCardStatusWHook(SCARDHANDLE hCard, LPWSTR szReaderName, LPDWORD pcchReaderLen, LPDWORD pdwState, LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen)
{
	CCasProxy *pCasProxy = reinterpret_cast<CCasProxy *>(hCard);
	if (!pCasProxy)
		return SCARD_E_INVALID_HANDLE;

	LPWSTR pReaderName = szBCASReaderNameW;

	if (szReaderName)
	{
		if (*pcchReaderLen == SCARD_AUTOALLOCATE)
		{
			*((void **)szReaderName) = pReaderName;
			*pcchReaderLen = lstrlenW(pReaderName) + 2;
		}
		else
		{
			int len = lstrlenW(pReaderName) + 2;
			if (*pcchReaderLen >= (DWORD)len)
			{
				memcpy(szReaderName, pReaderName, len * sizeof(WCHAR));
				*pcchReaderLen = len;
			}
			else
			{
				DWORD dw = *pcchReaderLen;
				LPWSTR p = pReaderName;
				while (dw)
				{
					*szReaderName++ = *p++;
					dw--;
				}
			}
		}
	}
	else
		*pcchReaderLen = lstrlenW(pReaderName) + 2;
	if (pdwState)
		*pdwState = SCARD_PRESENT;
	if (pdwProtocol)
		*pdwProtocol = SCARD_PROTOCOL_T1;
	if (pcbAtrLen)
	{
		if (*pcbAtrLen == SCARD_AUTOALLOCATE)
			*((LPBYTE *)pbAtr) = (LPBYTE)"";
		*pcbAtrLen = 0;
	}
	return SCARD_S_SUCCESS;
}

LONG WINAPI SCardTransmitHook(SCARDHANDLE hCard, LPCSCARD_IO_REQUEST pioSendPci, LPCBYTE pbSendBuffer, DWORD cbSendLength, LPSCARD_IO_REQUEST pioRecvPci, LPBYTE pbRecvBuffer, LPDWORD pcbRecvLength)
{
	// サーバにリクエスト送受信
	CCasProxy *pCasProxy = reinterpret_cast<CCasProxy *>(hCard);
	if (!pCasProxy)
		return SCARD_E_INVALID_HANDLE;

	const DWORD dwRecvLen = pCasProxy->TransmitCommand(pbSendBuffer, cbSendLength, pbRecvBuffer);
	if (pcbRecvLength)
		*pcbRecvLength = dwRecvLen;

	return (dwRecvLen) ? SCARD_S_SUCCESS : SCARD_E_TIMEOUT;
}

static bool Init()
{
	char *pA;
	WCHAR *pW;
	pA = szNameBufA;
	pW = szNameBufW;

	lstrcpyA(pA, szBCASReaderNameA);
	pA += (lstrlenA(szBCASReaderNameA) + 1);
	*pA = '\0';
	lstrcpyW(pW, szBCASReaderNameW);
	pW += (lstrlenW(szBCASReaderNameW) + 1);
	*pW = L'\0';

	dwNameBufLen = (DWORD)(pA - szNameBufA) + 1;

	return true;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call){
	case DLL_PROCESS_ATTACH:
		hinstDLL = hModule;
		if (!Init())
			return FALSE;

		{
		  TCHAR szDllPath[_MAX_PATH] = _T("");
		  if (::GetModuleFileName(hModule, szDllPath, _MAX_PATH) != 0) {
		    TCHAR szDrive[_MAX_DRIVE];
		    TCHAR szDir[_MAX_DIR];
		    TCHAR szFName[_MAX_FNAME];
		    _tsplitpath_s(szDllPath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, szFName, _MAX_FNAME, NULL, 0);

		    TCHAR szIniPath[_MAX_PATH] = _T("");
		    _tmakepath_s(szIniPath, _MAX_PATH, szDrive, szDir, szFName, _T("ini"));

		    TCHAR szBuff[64] = _T("");
		    ::GetPrivateProfileString(_T("Server"), _T("IP"), _T("127.0.0.1"), szBuff, 64, szIniPath);
		    char szHost[64] = "";
		    size_t len = 0;
		    ::wcstombs_s(&len, szHost, 64, szBuff, 63);
		    DWORD ip = ::inet_addr(szHost);
		    g_dwIP = ((ip & 0xFF) << 24) + ((ip & 0xFF00) << 8) + ((ip & 0xFF0000) >> 8) + ((ip & 0xFF000000) >> 24);

		    g_wPort = ::GetPrivateProfileInt(_T("Server"), _T("Port"), 6900UL, szIniPath);
		  }
		}
		break;

	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}
