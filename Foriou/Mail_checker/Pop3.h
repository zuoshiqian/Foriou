// Pop3.h: interface for the CPop3 class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_POP3_H__00D8F75D_FA68_4B73_8352_618E6B15B0CC__INCLUDED_)
#define AFX_POP3_H__00D8F75D_FA68_4B73_8352_618E6B15B0CC__INCLUDED_

#include <Winsock2.h>
#include <atlstr.h>
#include <vector>

using namespace std;

using ATL::CString;

#define	RECEIVEBUFSIZE 2048
#define COMMAND_END_FLAG "\r\n"

//#define DATA_END_FLAG "\r\n.\r\n"
#define DATA_END_FLAG ".\r\n"
//mayby have problem, if pop3 server send packet only ".\r\n"
//then we use "\r\n.\r\n" as end flag will occur error.

typedef unsigned (WINAPI *PBEGINTHREADEX_THREADFUNC)(LPVOID lpThreadParameter);
typedef unsigned (*PBEGINTHREADEX_THREADID);

//use CString variable to save packet from server
//#define USE_CSTRING

class CPop3
{
public://Variable

public://Function
	CPop3();
	virtual ~CPop3();
	BOOL Create(LPCSTR pszHostName, int nPort = 110);
	BOOL Connect(LPCSTR pszUser, LPCSTR pszPassword , BOOL bAPOPAuthentication = false);
	BOOL DisConnect();
	BOOL Close();
	BOOL GetTimeOut(DWORD *dwTimeOut);
	BOOL SetTimeOut(DWORD dwTimeOut);
	BOOL GetReceiveBufSize(long *lSize);
	BOOL SetReceiveBufSize(long lSize);
	BOOL GetMailList(std::vector<CString>& strResult);
	BOOL GetMail(int nMailIndex , CString *strMail);
	BOOL GetMailHeader(int nMailIndex , CString *strMailHeader);
	BOOL GetLastError(CString *msg);
	BOOL GetMailSize(int nMailIndex, long *lSize);
	BOOL GetStat(CString *strStat);
	BOOL GetMailDate(int nMailIndex, CString *strDate);
	BOOL GetMailReceiver(int nMailIndex, CString *strReceiver);
	BOOL GetMailSender(int nMailIndex, CString *strSender);
	BOOL GetMailSubject(int nMainIndex, CString *strSubject);
	BOOL GetMail(int nMailIndex, LPCSTR tmpfilename);
	BOOL DeleteMail(int nMailIndex);
	BOOL ResetMail();
	BOOL IsConnected();

private://Variable
	HANDLE		m_hEventWaitingRespond; //event of end receive packet from server
	HANDLE		m_hEventExitThread; //event of exit thread
	long		m_lReceiveBufSize; //receive buffer size
	DWORD		m_dwTimeout;
	DWORD		m_dwThreadId; //thread id of receive packet from server
	HANDLE		m_hThreadHandle; //thread handle of receive packet from server
	BOOL		m_bSocketOK; //if socket is created
	BOOL		m_bConnected;//if connect is ok
	SOCKET		m_sPop3Socket; // Socket of POP3
	WSAEVENT	m_hEvent[2];//  HANDLE of network event and exit thread event
	BOOL		m_bReceiving;
	CHAR		*m_szReceiveBuf; //Buffer to receive packet from server
	CString		m_strEnd; //end flag
	CString		m_strLastError; //Last error message
	FILE*		m_fpTmp;
#ifdef USE_CSTRING
	CString		m_strReceive; //Buffer save packet from POP3 server
#else
	CHAR		*m_szReceive;//Buffer save packet from POP3 server
	long		m_lReceive;//the length that receive from server
#endif


private://Function
	BOOL CheckMessage();
	void WaitForThreadExit();
	BOOL GetSocketResult(CString *msg , CString EndFlag);
	BOOL SetLastError(CString msg);
	static DWORD WINAPI MessageLoopThread(LPVOID pVoid);

/*********************************************************************************************
/* Definition for MD5
**********************************************************************************************/
	CString m_strTimeStamp;
	BYTE  m_lpszBuffer[64];		//input buffer
	ULONG m_nCount[2];			//number of bits, modulo 2^64 (lsb first)
	ULONG m_lMD5[4];			//MD5 checksum
	//interface functions for the RSA MD5 calculation
	CString MD5_GetMD5(BYTE* pBuf, UINT nLength);
	//RSA MD5 implementation
	void MD5_Init();
	void MD5_Transform(BYTE Block[64]);
	void MD5_Update(BYTE* Input, ULONG nInputLen);
	CString MD5_Final();
	inline DWORD MD5_RotateLeft(DWORD x, int n);
	inline void MD5_FF( DWORD& A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T);
	inline void MD5_GG( DWORD& A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T);
	inline void MD5_HH( DWORD& A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T);
	inline void MD5_II( DWORD& A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T);
	//utility functions
	void MD5_DWordToByte(BYTE* Output, DWORD* Input, UINT nLength);
	void MD5_ByteToDWord(DWORD* Output, BYTE* Input, UINT nLength);
/*********************************************************************************************
/* Definition for MD5
**********************************************************************************************/

protected:
};

/*********************************************************************************************
/* Definition for MD5
**********************************************************************************************/
//Magic initialization constants
#define MD5_INIT_STATE_0 0x67452301
#define MD5_INIT_STATE_1 0xefcdab89
#define MD5_INIT_STATE_2 0x98badcfe
#define MD5_INIT_STATE_3 0x10325476

//Constants for Transform routine.
#define MD5_S11  7
#define MD5_S12 12
#define MD5_S13 17
#define MD5_S14 22
#define MD5_S21  5
#define MD5_S22  9
#define MD5_S23 14
#define MD5_S24 20
#define MD5_S31  4
#define MD5_S32 11
#define MD5_S33 16
#define MD5_S34 23
#define MD5_S41  6
#define MD5_S42 10
#define MD5_S43 15
#define MD5_S44 21

//Transformation Constants - Round 1
#define MD5_T01  0xd76aa478 //Transformation Constant 1
#define MD5_T02  0xe8c7b756 //Transformation Constant 2
#define MD5_T03  0x242070db //Transformation Constant 3
#define MD5_T04  0xc1bdceee //Transformation Constant 4
#define MD5_T05  0xf57c0faf //Transformation Constant 5
#define MD5_T06  0x4787c62a //Transformation Constant 6
#define MD5_T07  0xa8304613 //Transformation Constant 7
#define MD5_T08  0xfd469501 //Transformation Constant 8
#define MD5_T09  0x698098d8 //Transformation Constant 9
#define MD5_T10  0x8b44f7af //Transformation Constant 10
#define MD5_T11  0xffff5bb1 //Transformation Constant 11
#define MD5_T12  0x895cd7be //Transformation Constant 12
#define MD5_T13  0x6b901122 //Transformation Constant 13
#define MD5_T14  0xfd987193 //Transformation Constant 14
#define MD5_T15  0xa679438e //Transformation Constant 15
#define MD5_T16  0x49b40821 //Transformation Constant 16

//Transformation Constants - Round 2
#define MD5_T17  0xf61e2562 //Transformation Constant 17
#define MD5_T18  0xc040b340 //Transformation Constant 18
#define MD5_T19  0x265e5a51 //Transformation Constant 19
#define MD5_T20  0xe9b6c7aa //Transformation Constant 20
#define MD5_T21  0xd62f105d //Transformation Constant 21
#define MD5_T22  0x02441453 //Transformation Constant 22
#define MD5_T23  0xd8a1e681 //Transformation Constant 23
#define MD5_T24  0xe7d3fbc8 //Transformation Constant 24
#define MD5_T25  0x21e1cde6 //Transformation Constant 25
#define MD5_T26  0xc33707d6 //Transformation Constant 26
#define MD5_T27  0xf4d50d87 //Transformation Constant 27
#define MD5_T28  0x455a14ed //Transformation Constant 28
#define MD5_T29  0xa9e3e905 //Transformation Constant 29
#define MD5_T30  0xfcefa3f8 //Transformation Constant 30
#define MD5_T31  0x676f02d9 //Transformation Constant 31
#define MD5_T32  0x8d2a4c8a //Transformation Constant 32

//Transformation Constants - Round 3
#define MD5_T33  0xfffa3942 //Transformation Constant 33
#define MD5_T34  0x8771f681 //Transformation Constant 34
#define MD5_T35  0x6d9d6122 //Transformation Constant 35
#define MD5_T36  0xfde5380c //Transformation Constant 36
#define MD5_T37  0xa4beea44 //Transformation Constant 37
#define MD5_T38  0x4bdecfa9 //Transformation Constant 38
#define MD5_T39  0xf6bb4b60 //Transformation Constant 39
#define MD5_T40  0xbebfbc70 //Transformation Constant 40
#define MD5_T41  0x289b7ec6 //Transformation Constant 41
#define MD5_T42  0xeaa127fa //Transformation Constant 42
#define MD5_T43  0xd4ef3085 //Transformation Constant 43
#define MD5_T44  0x04881d05 //Transformation Constant 44
#define MD5_T45  0xd9d4d039 //Transformation Constant 45
#define MD5_T46  0xe6db99e5 //Transformation Constant 46
#define MD5_T47  0x1fa27cf8 //Transformation Constant 47
#define MD5_T48  0xc4ac5665 //Transformation Constant 48

//Transformation Constants - Round 4
#define MD5_T49  0xf4292244 //Transformation Constant 49
#define MD5_T50  0x432aff97 //Transformation Constant 50
#define MD5_T51  0xab9423a7 //Transformation Constant 51
#define MD5_T52  0xfc93a039 //Transformation Constant 52
#define MD5_T53  0x655b59c3 //Transformation Constant 53
#define MD5_T54  0x8f0ccc92 //Transformation Constant 54
#define MD5_T55  0xffeff47d //Transformation Constant 55
#define MD5_T56  0x85845dd1 //Transformation Constant 56
#define MD5_T57  0x6fa87e4f //Transformation Constant 57
#define MD5_T58  0xfe2ce6e0 //Transformation Constant 58
#define MD5_T59  0xa3014314 //Transformation Constant 59
#define MD5_T60  0x4e0811a1 //Transformation Constant 60
#define MD5_T61  0xf7537e82 //Transformation Constant 61
#define MD5_T62  0xbd3af235 //Transformation Constant 62
#define MD5_T63  0x2ad7d2bb //Transformation Constant 63
#define MD5_T64  0xeb86d391 //Transformation Constant 64
//Null data (except for first BYTE) used to finalise the checksum calculation
static unsigned char PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
/*********************************************************************************************
/* Definition for MD5
**********************************************************************************************/

#endif // !defined(AFX_POP3_H__00D8F75D_FA68_4B73_8352_618E6B15B0CC__INCLUDED_)
