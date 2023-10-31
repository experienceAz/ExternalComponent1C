
#include "stdafx.h"


#ifdef __linux__
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#endif

#include <stdio.h>

#include <io.h>
#include <errno.h>

#include <wchar.h>
#include "1CGetImageFragment.h"
#include <string.h>
#include <locale.h>

#include <gdiplus.h>

#define TIME_LEN 34
#define BASE_ERRNO 7                                             


ULONG_PTR m_gdiplusToken;
using namespace Gdiplus;


static wchar_t* g_PropNames[] = { L"TestProperty", L"TestProperty2", L"TestProperty3" };
static wchar_t* g_PropNamesRu[] = { L"“естовое—войство", L"“естовое—войство2", L"“естовое—войство3" };

static wchar_t* g_MethodNamesRu[] = {
										L"¬ерси€",
										L"ѕолучить‘рагмент»зображени€"
};

static wchar_t* g_MethodNames[] = {
										L"Version",
										L"GetImageFragment"
};


static const wchar_t g_kClassNames[] = L"C1CGetImageFragment";
static IAddInDefBase* pAsyncEvent = NULL;

uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len = 0);
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len = 0);
uint32_t getLenShortWcharStr(const WCHAR_T* Source);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
std::wstring replace(std::wstring text, std::wstring s, std::wstring d);

//---------------------------------------------------------------------------//

const wchar_t* kComponentVersion = L"1.0";

const wchar_t* kErrMsg_NoImagePath = L"No image path";
const wchar_t* kErrMsg_NoMFCPath = L"No path to application";
const wchar_t* kErrMsg_NoResultPath = L"No result path";

const wchar_t* kErrMsg_LoadImage = L"Error loading image";
const wchar_t* kErrMsg_Timeout = L"Error timeout expired";
const wchar_t* kErrMsg_InternalError = L"Internal error";   // любые внутренние ошибка кода компоненты
const wchar_t* kErrMsg_NoPicturePaths = L"No picture paths";

//---------------------------------------------------------------------------//
long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface)
{
	if (!*pInterface)
	{
		*pInterface = new C1CGetImageFragment;
		return (long)*pInterface;
	}
	return 0;
}
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pIntf)
{
	if (!*pIntf)
		return -1;

	delete* pIntf;
	*pIntf = 0;
	return 0;
}
//---------------------------------------------------------------------------//
const WCHAR_T* GetClassNames()
{
	static WCHAR_T* names = 0;
	if (!names)
		::convToShortWchar(&names, g_kClassNames);
	return names;
}

//---------------------------------------------------------------------------//
#ifndef __linux__
VOID CALLBACK MyTimerProc(
	HWND hwnd, // handle of window for timer messages
	UINT uMsg, // WM_TIMER message
	UINT idEvent, // timer identifier
	DWORD dwTime // current system time
);
#else
static void MyTimerProc(int sig);
#endif //__linux__

// C1CGetImageFragment
//---------------------------------------------------------------------------//
C1CGetImageFragment::C1CGetImageFragment()
{
	m_iMemory = 0;
	m_iConnect = 0;

	pTestPropBool = false;

	pTestPropInt = 15;

	pTestPropStr = "testStr";

	/*pTestPropStr = new char[100];
	std::strcpy(pTestPropStr, "abc 123");*/

}
//---------------------------------------------------------------------------//
C1CGetImageFragment::~C1CGetImageFragment()
{
}
//---------------------------------------------------------------------------//
bool C1CGetImageFragment::Init(void* pConnection)
{
	m_iConnect = (IAddInDefBase*)pConnection;
	return m_iConnect != NULL;
}
//---------------------------------------------------------------------------//
long C1CGetImageFragment::GetInfo()
{
	// Component should put supported component technology version 
	// This component supports 2.0 version
	return 2000;
}
//---------------------------------------------------------------------------//
void C1CGetImageFragment::Done()
{
}
/////////////////////////////////////////////////////////////////////////////
// ILanguageExtenderBase
//---------------------------------------------------------------------------//
bool C1CGetImageFragment::RegisterExtensionAs(WCHAR_T** wsExtensionName)
{
	wchar_t* wsExtension = L"AddInNativeExtension";
	int iActualSize = ::wcslen(wsExtension) + 1;
	WCHAR_T* dest = 0;

	if (m_iMemory)
	{
		if (m_iMemory->AllocMemory((void**)wsExtensionName, iActualSize * sizeof(WCHAR_T)))
			::convToShortWchar(wsExtensionName, wsExtension, iActualSize);
		return true;
	}

	return false;
}
//---------------------------------------------------------------------------//
long C1CGetImageFragment::GetNProps()
{
	// You may delete next lines and add your own implementation code here
	return ePropLast;
}
//---------------------------------------------------------------------------//
long C1CGetImageFragment::FindProp(const WCHAR_T* wsPropName)
{
	long plPropNum = -1;
	wchar_t* propName = 0;

	::convFromShortWchar(&propName, wsPropName);
	plPropNum = findName(g_PropNames, propName, ePropLast);

	if (plPropNum == -1)
		plPropNum = findName(g_PropNamesRu, propName, ePropLast);

	delete[] propName;

	return plPropNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* C1CGetImageFragment::GetPropName(long lPropNum, long lPropAlias)
{
	if (lPropNum >= ePropLast)
		return NULL;

	wchar_t* wsCurrentName = NULL;
	WCHAR_T* wsPropName = NULL;
	int iActualSize = 0;

	switch (lPropAlias)
	{
	case 0: // First language
		wsCurrentName = g_PropNames[lPropNum];
		break;
	case 1: // Second language
		wsCurrentName = g_PropNamesRu[lPropNum];
		break;
	default:
		return 0;
	}

	iActualSize = wcslen(wsCurrentName) + 1;

	if (m_iMemory && wsCurrentName)
	{
		if (m_iMemory->AllocMemory((void**)&wsPropName, iActualSize * sizeof(WCHAR_T)))
			::convToShortWchar(&wsPropName, wsCurrentName, iActualSize);
	}

	return wsPropName;
}

wchar_t* GetWC(char* c)
{
	size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);

	return wc;
}

char* wchar_to_char(wchar_t* pwchar)
{
	// get the number of characters in the string.
	int currentCharIndex = 0;
	char currentChar = pwchar[currentCharIndex];

	while (currentChar != '\0')
	{
		currentCharIndex++;
		currentChar = pwchar[currentCharIndex];
	}

	const int charCount = currentCharIndex + 1;

	// allocate a new block of memory size char (1 byte) instead of wide char (2 bytes)
	char* filePathC = (char*)malloc(sizeof(char) * charCount);

	for (int i = 0; i < charCount; i++)
	{
		// convert to char (1 byte)
		char character = pwchar[i];

		*filePathC = character;

		filePathC += sizeof(char);

	}
	filePathC += '\0';

	filePathC -= (sizeof(char) * charCount);

	return filePathC;
}


//---------------------------------------------------------------------------//
bool C1CGetImageFragment::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{
	wchar_t* wsTmp1 = 0;
	WCHAR_T* wsTmp2 = 0;

	switch (lPropNum)
	{
	case ePropTestBool:
		TV_VT(pvarPropVal) = VTYPE_BOOL; //выставл€ем тип
		TV_BOOL(pvarPropVal) = pTestPropBool;    //выставл€ем значение
		break;
	case ePropTestInt:
		TV_VT(pvarPropVal) = VTYPE_I4; //выставл€ем тип
		TV_I4(pvarPropVal) = pTestPropInt;    //выставл€ем значение
		break;
	case ePropTestStr:
	{
		size_t l = std::strlen(pTestPropStr) + 1;
		pvarPropVal->vt = VTYPE_PSTR;
		pvarPropVal->strLen = l;
		m_iMemory->AllocMemory((void**)&pvarPropVal->pstrVal, l * sizeof(char));
		std::memcpy(pvarPropVal->pstrVal, pTestPropStr, l * sizeof(char));

		break;
	}

	default:
		return false;
	}

	return true;
}


//---------------------------------------------------------------------------//
bool C1CGetImageFragment::SetPropVal(const long lPropNum, tVariant* varPropVal)
{
	wchar_t* wsTmp = 0;
	int size = 0;
	char* mbStr = 0;
	char* name = 0;
	size_t len = std::strlen(varPropVal->pstrVal);

	switch (lPropNum)
	{
	case ePropTestBool:
		if (TV_VT(varPropVal) != VTYPE_BOOL)
			return false;
		pTestPropBool = TV_BOOL(varPropVal);
		break;
	case ePropTestInt:
		if (TV_VT(varPropVal) != VTYPE_I4)
			return false;
		pTestPropInt = TV_I4(varPropVal);
		break;
	case ePropTestStr:
		//delete[] pTestPropStr;
		pTestPropStr = new char[len + 1];
		std::strncpy(pTestPropStr, varPropVal->pstrVal, len + 1);
		break;

	default:
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------//
bool C1CGetImageFragment::IsPropReadable(const long lPropNum)
{
	return true;
}
//---------------------------------------------------------------------------//
bool C1CGetImageFragment::IsPropWritable(const long lPropNum)
{
	return true;
}
//---------------------------------------------------------------------------//
long C1CGetImageFragment::GetNMethods()
{
	return eMethLast;
}
//---------------------------------------------------------------------------//
long C1CGetImageFragment::FindMethod(const WCHAR_T* wsMethodName)
{
	long plMethodNum = -1;
	wchar_t* name = 0;

	::convFromShortWchar(&name, wsMethodName);

	plMethodNum = findName(g_MethodNames, name, eMethLast);

	if (plMethodNum == -1)
		plMethodNum = findName(g_MethodNamesRu, name, eMethLast);

	return plMethodNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* C1CGetImageFragment::GetMethodName(const long lMethodNum, const long lMethodAlias)
{
	if (lMethodNum >= eMethLast)
		return NULL;

	wchar_t* wsCurrentName = NULL;
	WCHAR_T* wsMethodName = NULL;
	int iActualSize = 0;

	switch (lMethodAlias)
	{
	case 0: // First language
		wsCurrentName = g_MethodNames[lMethodNum];
		break;
	case 1: // Second language
		wsCurrentName = g_MethodNamesRu[lMethodNum];
		break;
	default:
		return 0;
	}

	iActualSize = wcslen(wsCurrentName) + 1;

	if (m_iMemory && wsCurrentName)
	{
		if (m_iMemory->AllocMemory((void**)&wsMethodName, iActualSize * sizeof(WCHAR_T)))
			::convToShortWchar(&wsMethodName, wsCurrentName, iActualSize);
	}

	return wsMethodName;
}
//---------------------------------------------------------------------------//
long C1CGetImageFragment::GetNParams(const long lMethodNum)
{
	switch (lMethodNum)
	{
	case eVersion:
		return 0;
	case eGetImageFragment:
		return 7;
	default:
		return 0;
	}

	return 0;
}
//---------------------------------------------------------------------------//
bool C1CGetImageFragment::GetParamDefValue(const long lMethodNum, const long lParamNum,
	tVariant* pvarParamDefValue)
{
	TV_VT(pvarParamDefValue) = VTYPE_EMPTY;

	switch (lMethodNum)
	{
	case eVersion:
	case eGetImageFragment:
		// There are no parameter values by default 
		break;
	default:
		return false;
	}

	return false;
}
//---------------------------------------------------------------------------//
bool C1CGetImageFragment::HasRetVal(const long lMethodNum)
{
	switch (lMethodNum)
	{
	case eVersion:
		return true;
	case eGetImageFragment:
		return true;
	default:
		return false;
	}

	return false;
}
//---------------------------------------------------------------------------//
bool C1CGetImageFragment::CallAsProc(const long lMethodNum,
	tVariant* paParams, const long lSizeArray)
{
	switch (lMethodNum)
	{
	case eVersion:
	case eGetImageFragment:
		break;

	default:
		return false;
	}

	return false;   
}


//---------------------------------------------------------------------------//
bool C1CGetImageFragment::CallAsFunc(const long lMethodNum,
	tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
	switch (lMethodNum)
	{
	case eVersion:
	{
		if (pvarRetValue)
		{
			size_t strLen = wcslen(kComponentVersion);
			if (m_iMemory->AllocMemory((void**)&pvarRetValue->pwstrVal, (strLen + 1) * sizeof(kComponentVersion[0])))
			{
				wcscpy_s(pvarRetValue->pwstrVal, strLen + 1, kComponentVersion);
				pvarRetValue->wstrLen = strLen;
				TV_VT(pvarRetValue) = VTYPE_PWSTR;
			}
		}

		break;
	}

	// ѕолучить‘рагмент»зображени€(ѕуть ‘айлу»зображени€,
	//  ѕуть ‘айлу–езультата, ѕоложение‘рагмента(„исло), —мещениеѕоX(„исло - мм), 
	//  —мещениеѕоY(„исло - мм), Ўирина(„исло-мм), ¬ысота(„исло-мм)) 
	case eGetImageFragment:
	{
		// парам 0 - строка с путем к исходной картинке
		tVariant& pParam0 = paParams[0];

		wchar_t* sourcePath = pParam0.pwstrVal;
		uint32_t sourcePathLen = pParam0.wstrLen;
		if (sourcePathLen == 0)
		{
			addError(kErrMsg_NoPicturePaths);
			return false;
		}

		// парам 1 - строка с путем картинки-результата
		tVariant& pParam1 = paParams[1];
		wchar_t* resultPath = pParam1.pwstrVal;
		uint32_t resultPathLen = pParam1.wstrLen;
		if (resultPathLen == 0)
		{
			addError(kErrMsg_NoPicturePaths);
			return false;
		}

		// парам 2 - ѕоложение‘рагмента
		tVariant& pParam2 = paParams[2];
		int fragmentPos = pParam2.intVal;

		// парам 3 - —мещениеѕоX
		tVariant& pParam3 = paParams[3];
		int offsetX = pParam3.intVal;

		// парам 4 - —мещениеѕоY
		tVariant& pParam4 = paParams[4];
		int offsetY = pParam4.intVal;

		// парам 5 - Ўирина
		tVariant& pParam5 = paParams[5];
		int width = pParam5.intVal;

		// парам 6 - ¬ысота
		tVariant& pParam6 = paParams[6];
		int height = pParam6.intVal;

		// если указан угол  - то ширина и высота считаем фиксированными - 55 и 45 мм.

		bool saveRet = GetImageFragment(sourcePath, resultPath, fragmentPos,
			offsetX, offsetY, width, height);

		TV_VT(pvarRetValue) = VTYPE_BOOL;
		pvarRetValue->bVal = saveRet;

		break;
	}

	default:
		return false;
	}

	return true;
}


//---------------------------------------------------------------------------//
// This code will work only on the client!
#ifndef __linux__
VOID CALLBACK MyTimerProc(
	HWND hwnd,    // handle of window for timer messages
	UINT uMsg,    // WM_TIMER message
	UINT idEvent, // timer identifier
	DWORD dwTime  // current system time
)
{
	if (!pAsyncEvent)
		return;

	wchar_t* who = L"ComponentNative", * what = L"Timer";

	wchar_t* wstime = new wchar_t[TIME_LEN];
	if (wstime)
	{
		wmemset(wstime, 0, TIME_LEN);
		::_ultow(dwTime, wstime, 10);
		pAsyncEvent->ExternalEvent(who, what, wstime);
		delete[] wstime;
	}
}
#else
void MyTimerProc(int sig)
{
	if (pAsyncEvent)
		return;

	WCHAR_T* who = 0, * what = 0, * wdata = 0;
	wchar_t* data = 0;
	time_t dwTime = time(NULL);

	data = new wchar_t[TIME_LEN];

	if (data)
	{
		wmemset(data, 0, TIME_LEN);
		swprintf(data, TIME_LEN, L"%ul", dwTime);
		::convToShortWchar(&who, L"ComponentNative");
		::convToShortWchar(&what, L"Timer");
		::convToShortWchar(&wdata, data);

		pAsyncEvent->ExternalEvent(who, what, wdata);

		delete[] who;
		delete[] what;
		delete[] wdata;
		delete[] data;
}
}
#endif
//---------------------------------------------------------------------------//
void C1CGetImageFragment::SetLocale(const WCHAR_T* loc)
{
#ifndef __linux__
	_wsetlocale(LC_ALL, loc);
#else
	//We convert in char* char_locale
	//also we establish locale
	//setlocale(LC_ALL, char_locale);
#endif
}
/////////////////////////////////////////////////////////////////////////////
// LocaleBase
//---------------------------------------------------------------------------//
bool C1CGetImageFragment::setMemManager(void* mem)
{
	m_iMemory = (IMemoryManager*)mem;
	return m_iMemory != 0;
}
//---------------------------------------------------------------------------//
void C1CGetImageFragment::addError(uint32_t wcode, const wchar_t* source,
	const wchar_t* descriptor, long code)
{
	if (m_iConnect)
	{
		WCHAR_T* err = 0;
		WCHAR_T* descr = 0;

		::convToShortWchar(&err, source);
		::convToShortWchar(&descr, descriptor);

		m_iConnect->AddError(wcode, err, descr, code);
		delete[] err;
		delete[] descr;
	}
}


//---------------------------------------------------------------------------//
void C1CGetImageFragment::addError(const wchar_t* errorText)
{
	if (m_iConnect)
		m_iConnect->AddError(ADDIN_E_NONE, L"C1CGetImageFragment", errorText, 0);
}


//---------------------------------------------------------------------------//
long C1CGetImageFragment::findName(wchar_t* names[], const wchar_t* name,
	const uint32_t size) const
{
	long ret = -1;
	for (uint32_t i = 0; i < size; i++)
	{
		if (!wcscmp(names[i], name))
		{
			ret = i;
			break;
		}
	}
	return ret;
}
//---------------------------------------------------------------------------//
uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len)
{
	if (!len)
		len = ::wcslen(Source) + 1;

	if (!*Dest)
		*Dest = new WCHAR_T[len];

	WCHAR_T* tmpShort = *Dest;
	wchar_t* tmpWChar = (wchar_t*)Source;
	uint32_t res = 0;

	::memset(*Dest, 0, len * sizeof(WCHAR_T));
	do
	{
		*tmpShort++ = (WCHAR_T)*tmpWChar++;
		++res;
	} while (len-- && *tmpWChar);

	return res;
}
//---------------------------------------------------------------------------//
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len)
{
	if (!len)
		len = getLenShortWcharStr(Source) + 1;

	if (!*Dest)
		*Dest = new wchar_t[len];

	wchar_t* tmpWChar = *Dest;
	WCHAR_T* tmpShort = (WCHAR_T*)Source;
	uint32_t res = 0;

	::memset(*Dest, 0, len * sizeof(wchar_t));
	do
	{
		*tmpWChar++ = (wchar_t)*tmpShort++;
		++res;
	} while (len-- && *tmpShort);

	return res;
}
//---------------------------------------------------------------------------//
uint32_t getLenShortWcharStr(const WCHAR_T* Source)
{
	uint32_t res = 0;
	WCHAR_T* tmpShort = (WCHAR_T*)Source;

	while (*tmpShort++)
		++res;

	return res;
}

//---------------------------------------------------------------------------//
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

std::wstring replace(std::wstring text, std::wstring s, std::wstring d)
{
	for (unsigned index = 0; index = text.find(s, index), index != std::wstring::npos;)
	{
		text.replace(index, s.length(), d);
		index += d.length();
	}
	return text;
}

const int defWidthMM = 55; // 55 мм
const int defHeightMM = 45; // 45 мм

//---------------------------------------------------------------------------//
bool C1CGetImageFragment::GetImageFragment(wchar_t* sourcePath, wchar_t* resultPath, int fragmentPos,
	int left, int top, int width, int height)
{
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	_wsplitpath_s(resultPath, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);

	FragmentPosition fragPos = (FragmentPosition)fragmentPos;

	// Initialize GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	// Get the CLSID of the PNG encoder.
	CLSID   encoderClsid;

	if (_wcsicmp(ext, L".bmp") == 0)
		GetEncoderClsid(L"image/bmp", &encoderClsid);
	else if (_wcsicmp(ext, L".jpg") == 0)
		GetEncoderClsid(L"image/jpeg", &encoderClsid);
	else if (_wcsicmp(ext, L".gif") == 0)
		GetEncoderClsid(L"image/gif", &encoderClsid);
	else if (_wcsicmp(ext, L".png") == 0)
		GetEncoderClsid(L"image/png", &encoderClsid);
	else if (_wcsicmp(ext, L".tif") == 0)
		GetEncoderClsid(L"image/tiff", &encoderClsid);

	Bitmap bitmap(sourcePath);

	int imageWidth = bitmap.GetWidth();
	int imageHeight = bitmap.GetHeight();

	REAL xDPI = bitmap.GetHorizontalResolution();
	REAL yDPI = bitmap.GetVerticalResolution();

	const REAL inchMM = 25.4f;

	int defWidth = (int)((REAL)defWidthMM / inchMM * xDPI + 0.5f); // в пиксел€х
	int defHeight = (int)((REAL)defHeightMM / inchMM * yDPI + 0.5f); // в пиксел€х

	// если указан угол  - то ширина и высота считаем фиксированными - 45 и 25 мм.
	switch (fragPos)
	{
	case eLeftTop:
	{
		width = defWidth;
		height = defHeight;
		left = 0;
		top = 0;
		break;
	}

	case eLeftBottom:
	{
		width = defWidth;
		height = defHeight;
		left = 0;
		top = imageHeight - height;
		break;
	}

	case eRightTop:
	{
		width = defWidth;
		height = defHeight;
		left = imageWidth - width;
		top = 0;
		break;
	}

	case eRightBottom:
	{
		width = defWidth;
		height = defHeight;
		left = imageWidth - width;
		top = imageHeight - height;
		break;
	}

	case ePrecisePosition:
	{
		left = (int)((REAL)left / inchMM * xDPI + 0.5f); // в пиксел€х
		top = (int)((REAL)top / inchMM * xDPI + 0.5f); // в пиксел€х
		width = (int)((REAL)width / inchMM * xDPI + 0.5f); // в пиксел€х
		height = (int)((REAL)height / inchMM * xDPI + 0.5f); // в пиксел€х

		if (left < 0)
			left = 0;
		if (top < 0)
			top = 0;

		if (left > imageWidth)
			left = imageWidth;
		if (top > imageHeight)
			top = imageHeight;

		if (width > imageWidth)
			width = imageWidth;
		if (height > imageHeight)
			height = imageHeight;

		if (left + width > imageWidth)
			left = imageWidth - width;
		if (top + height > imageHeight)
			top = imageHeight - height;

		break;
	}

	default:
		return false;
	}

	RectF sourceRect((REAL)left, (REAL)top, (REAL)width, (REAL)height);
	Status stat;

	Bitmap* secondBitmap = bitmap.Clone(sourceRect, PixelFormatDontCare);
	stat = secondBitmap->Save(resultPath, &encoderClsid, NULL);

	delete secondBitmap;

	Gdiplus::GdiplusShutdown(m_gdiplusToken);

	return true;
}
