#ifdef _DEBUG
// http://blog.m-ri.de/index.php/2009/08/09/vs-tippstricks-einfache-ausgabe-mit-trace-auch-in-der-release-version/#comments
//    CTraceToOutputDebugString
//        Is a nice replacment class for TRACE
//        Easy to use with:
//            #undef TRACE
//            #define TRACE    CTraceToOutputDebugString()
 
class CTraceToOutputDebugString
{
public:
    // Non Unicode output helper
    void operator()(PCSTR pszFormat, ...)
    {
        va_list ptr;
        va_start(ptr, pszFormat);
        TraceV(pszFormat,ptr);
        va_end(ptr);
    }
 
    // Unicode output helper
    void operator()(PCWSTR pszFormat, ...)
    {
        va_list ptr;
        va_start(ptr, pszFormat);
        TraceV(pszFormat,ptr);
        va_end(ptr);
    }
 
private:
    // Non Unicode output helper
    void TraceV(PCSTR pszFormat, va_list args)
    {
        // Format the output buffer
        char szBuffer[1024];
        int n = sprintf_s(szBuffer, _countof(szBuffer), "[%d] ", GetCurrentThreadId());
        _vsnprintf_s(szBuffer+n, _countof(szBuffer)-n, _TRUNCATE, pszFormat, args);
        OutputDebugStringA(szBuffer);
    }
 
    // Unicode output helper
    void TraceV(PCWSTR pszFormat, va_list args)
    {
        wchar_t szBuffer[1024];
        int n = swprintf_s(szBuffer, _countof(szBuffer), _T("[%d] "), GetCurrentThreadId());
        _vsnwprintf_s(szBuffer+n, _countof(szBuffer)-n, _TRUNCATE, pszFormat, args);
        OutputDebugStringW(szBuffer);
    }
};
#undef TRACE
//#define TRACE ATLTRACE(_T("%d "),GetCurrentThreadId()); ATLTRACE
#define TRACE    CTraceToOutputDebugString()
#endif
