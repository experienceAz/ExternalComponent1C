#ifndef __ADDINNATIVE_H__
#define __ADDINNATIVE_H__

#include <ComponentBase.h>
#include "AddInDefBase.h"
#include "IMemoryManager.h"


enum FragmentPosition
{
    eLeftTop = 0,       // левый верхний угол
    eLeftBottom,        // левый нижний угол
    eRightTop,          // правый верхний угол
    eRightBottom,       // правый нижний угол
    ePrecisePosition    // точное положение
};

///////////////////////////////////////////////////////////////////////////////
// class C1CGetImageFragment
class C1CGetImageFragment : public IComponentBase
{
public:
    enum Props
    {
        ePropTestBool = 0,
        ePropTestInt,
        ePropTestStr,
        ePropLast      // Always last
    };

    enum Methods
    {
        eVersion = 0,       // Версия Компоненты
        eGetImageFragment,  // ПолучитьФрагментИзображения

        eMethLast           // Always last
    };

    C1CGetImageFragment(void);
    virtual ~C1CGetImageFragment();
    // IInitDoneBase
    virtual bool ADDIN_API Init(void*);
    virtual bool ADDIN_API setMemManager(void* mem);
    virtual long ADDIN_API GetInfo();
    virtual void ADDIN_API Done();
    // ILanguageExtenderBase
    virtual bool ADDIN_API RegisterExtensionAs(WCHAR_T**);
    virtual long ADDIN_API GetNProps();
    virtual long ADDIN_API FindProp(const WCHAR_T* wsPropName);
    virtual const WCHAR_T* ADDIN_API GetPropName(long lPropNum, long lPropAlias);
    virtual bool ADDIN_API GetPropVal(const long lPropNum, tVariant* pvarPropVal);
    virtual bool ADDIN_API SetPropVal(const long lPropNum, tVariant* varPropVal);
    virtual bool ADDIN_API IsPropReadable(const long lPropNum);
    virtual bool ADDIN_API IsPropWritable(const long lPropNum);
    virtual long ADDIN_API GetNMethods();
    virtual long ADDIN_API FindMethod(const WCHAR_T* wsMethodName);
    virtual const WCHAR_T* ADDIN_API GetMethodName(const long lMethodNum,
        const long lMethodAlias);
    virtual long ADDIN_API GetNParams(const long lMethodNum);
    virtual bool ADDIN_API GetParamDefValue(const long lMethodNum, const long lParamNum,
        tVariant* pvarParamDefValue);
    virtual bool ADDIN_API HasRetVal(const long lMethodNum);
    virtual bool ADDIN_API CallAsProc(const long lMethodNum,
        tVariant* paParams, const long lSizeArray);
    virtual bool ADDIN_API CallAsFunc(const long lMethodNum,
        tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray);
    // LocaleBase
    virtual void ADDIN_API SetLocale(const WCHAR_T* loc);

private:
    long findName(wchar_t* names[], const wchar_t* name, const uint32_t size) const;
    void addError(uint32_t wcode, const wchar_t* source,
        const wchar_t* descriptor, long code);

    // добавление ошибки в Предприятие в момент вызова функции компоненты
    void addError(const wchar_t* errorText);

    bool GetImageFragment(wchar_t* sourcePath, wchar_t* resultPath, int fragmentPos,
        int left, int top, int width, int height);

    // Attributes
    IAddInDefBase* m_iConnect;
    IMemoryManager* m_iMemory;

    bool                m_boolEnabled;
    uint32_t            m_uiTimer;

    bool pTestPropBool;
    int32_t pTestPropInt;

    char* pTestPropStr;

};

#endif //__ADDINNATIVE_H__
