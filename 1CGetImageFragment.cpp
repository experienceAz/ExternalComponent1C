
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
#define BASE_ERRNO     7                                             


ULONG_PTR m_gdiplusToken;
using namespace Gdiplus;


static wchar_t *g_PropNames[] = { L"TestProp", L"TraceLavel", L"Port", L"IP" };
static wchar_t *g_PropNamesRu[] = { L"ТестовоеСвойство", L"ТестовоеСвойство2", L"ТестовоеСвойство3", L"ТестовоеСвойство4" };

static wchar_t *g_MethodNamesRu[] = {
                                        L"Версия",
                                        L"ПолучитьФрагментИзображения",
                                        L"ТестовыйМетод"
                                    };
                                        
static wchar_t *g_MethodNames[] =   {
                                        L"Version", 
                                        L"GetImageFragment",
                                        L"TestMethod"
                                    };


static const wchar_t g_kClassNames[] = L"C1CGetImageFragment";
static IAddInDefBase *pAsyncEvent = NULL;

uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len = 0);
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len = 0);
uint32_t getLenShortWcharStr(const WCHAR_T* Source);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
std::wstring replace(std::wstring text, std::wstring s, std::wstring d);

//---------------------------------------------------------------------------//

const wchar_t* kComponentVersion = L"1.0";

const wchar_t* kErrMsg_NoImagePath =  L"No image path";
const wchar_t* kErrMsg_NoMFCPath =  L"No path to application";
const wchar_t* kErrMsg_NoResultPath =  L"No result path";

const wchar_t* kErrMsg_LoadImage    =  L"Error loading image";
const wchar_t* kErrMsg_Timeout      =  L"Error timeout expired";
const wchar_t* kErrMsg_InternalError =     L"Internal error";   // любые внутренние ошибка кода компоненты
const wchar_t* kErrMsg_NoPicturePaths    = L"No picture paths";

//---------------------------------------------------------------------------//
long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface)
{
    if(!*pInterface)
    {
        *pInterface= new C1CGetImageFragment;
        return (long)*pInterface;
    }
    return 0;
}
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pIntf)
{
   if(!*pIntf)
      return -1;

   delete *pIntf;
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
    wchar_t *wsExtension = L"AddInNativeExtension";
    int iActualSize = ::wcslen(wsExtension) + 1;
    WCHAR_T* dest = 0;

    if (m_iMemory)
    {
        if(m_iMemory->AllocMemory((void**)wsExtensionName, iActualSize * sizeof(WCHAR_T)))
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

    wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsPropName = NULL;
    int iActualSize = 0;

    switch(lPropAlias)
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

bool C1CGetImageFragment::wstring_to_p(std::wstring str, tVariant* val) {
    char* t1;
    TV_VT(val) = VTYPE_PWSTR;
    m_iMemory->AllocMemory((void**)&t1, (str.length() + 1) * sizeof(WCHAR_T));
    memcpy(t1, str.c_str(), (str.length() + 1) * sizeof(WCHAR_T));
    val->pstrVal = t1;
    val->strLen = str.length();
    return true;
}


//---------------------------------------------------------------------------//
bool C1CGetImageFragment::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{ 
    switch (lPropNum)
    {
    case ePropTest:
        TV_VT(pvarPropVal) = VTYPE_BOOL;
        TV_BOOL(pvarPropVal) = m_boolEnabled;
        break;

    default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------//
bool C1CGetImageFragment::SetPropVal(const long lPropNum, tVariant* varPropVal)
{ 
    switch (lPropNum)
    {
    case eMethTest:
        ::convFromShortWchar(&pTestProp, TV_WSTR(varPropVal));
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

    wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsMethodName = NULL;
    int iActualSize = 0;

    switch(lMethodAlias)
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
        if(m_iMemory->AllocMemory((void**)&wsMethodName, iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(&wsMethodName, wsCurrentName, iActualSize);
    }

    return wsMethodName;
}
//---------------------------------------------------------------------------//
long C1CGetImageFragment::GetNParams(const long lMethodNum)
{ 
    switch(lMethodNum)
    { 
    case eMethTest:
        return 0;
    default:
        return 0;
    }
       
    return 0;
}
//---------------------------------------------------------------------------//
bool C1CGetImageFragment::GetParamDefValue(const long lMethodNum, const long lParamNum,
                          tVariant *pvarParamDefValue)
{ 
    TV_VT(pvarParamDefValue) = VTYPE_EMPTY;

    switch(lMethodNum)
    {   
    case eMethTest:
        return true;

    default:
        return false;
    }

    return false;
} 
//---------------------------------------------------------------------------//
bool C1CGetImageFragment::HasRetVal(const long lMethodNum)
{ 
    switch(lMethodNum)
    {    
    case eMethTest:
        return false;
    default:
        return false;
    }

    return false;
}

void C1CGetImageFragment::testMeth()
{
    int a = 4;
}

//---------------------------------------------------------------------------//
bool C1CGetImageFragment::CallAsProc(const long lMethodNum,
                    tVariant* paParams, const long lSizeArray)
{ 
    switch(lMethodNum)
    {
    case eMethTest:
        testMeth();
        return true;
        break;

    default:
        return false;
    }

    return false;   // as func
}


//---------------------------------------------------------------------------//
bool C1CGetImageFragment::CallAsFunc(const long lMethodNum,
    tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
    return false;
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

    wchar_t *who = L"ComponentNative", *what = L"Timer";

    wchar_t *wstime = new wchar_t[TIME_LEN];
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

    WCHAR_T *who = 0, *what = 0, *wdata = 0;
    wchar_t *data = 0;
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
        WCHAR_T *err = 0;
        WCHAR_T *descr = 0;
        
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
    wchar_t* tmpWChar = (wchar_t*) Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len*sizeof(WCHAR_T));
    do
    {
        *tmpShort++ = (WCHAR_T)*tmpWChar++;
        ++res;
    }
    while (len-- && *tmpWChar);

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

    ::memset(*Dest, 0, len*sizeof(wchar_t));
    do
    {
        *tmpWChar++ = (wchar_t)*tmpShort++;
        ++res;
    }
    while (len-- && *tmpShort);

    return res;
}
//---------------------------------------------------------------------------//
uint32_t getLenShortWcharStr(const WCHAR_T* Source)
{
    uint32_t res = 0;
    WCHAR_T *tmpShort = (WCHAR_T*)Source;

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
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
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
	for(unsigned index=0; index=text.find(s, index), index!=std::wstring::npos;)
	{
		text.replace(index, s.length(), d);
		index+=d.length();
	}
	return text;
}
