#include <Windows.h>
#include <string>
#include <Psapi.h>
#include <sstream>
#include <iostream>
using namespace std;

//Forward declarations
string getExceptionInformation(EXCEPTION_RECORD* er, DWORD code = 0);

typedef struct _EXCEPTION_REGISTRATION
{
    using pFncHandler = void(__cdecl*)(EXCEPTION_RECORD*, _EXCEPTION_REGISTRATION*,
        CONTEXT*, EXCEPTION_RECORD*);

    struct _EXCEPTION_REGISTRATION* oldHandler;
    pFncHandler newHandler;

} EXCEPTION_REGISTRATION, * PEXCEPTION_REGISTRATION;
//Thread information block
EXCEPTION_REGISTRATION* pHandlerBase = (EXCEPTION_REGISTRATION*)__readfsdword(0x18);

EXCEPTION_DISPOSITION __cdecl ExceptionHandler(EXCEPTION_RECORD* pExceptionRecord, void* pEstablisherFrame,
    CONTEXT* pContextRecord, void* pDispatcherContext)
{
    cout<< getExceptionInformation(pExceptionRecord, pExceptionRecord->ExceptionCode);
    return ExceptionContinueExecution;
}

static const char* exceptionCodeToString(const DWORD& code)
{
    switch (code) {
        case STATUS_NO_MEMORY:              
            return "STATUS_NO_MEMORY";
        case STATUS_ILLEGAL_INSTRUCTION:       
            return "STATUS_ILLEGAL_INSTRUCTION";
        case STATUS_INVALID_HANDLE:           
            return "STATUS_INVALID_HANDLE";
        default: 
            return "UNKNOWN EXCEPTION";
    }
}

static string getExceptionInformation(EXCEPTION_RECORD* er,DWORD code)
{
    HMODULE hmod;
    ::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, static_cast<LPCTSTR>(er->ExceptionAddress), &hmod);
    MODULEINFO minfo;
    ::GetModuleInformation(::GetCurrentProcess(), hmod, &minfo, sizeof(minfo));
    char fileName[MAX_PATH];
    ::GetModuleFileNameExA(::GetCurrentProcess(), hmod, fileName, MAX_PATH);

    ostringstream strStream;
    strStream << exceptionCodeToString(code) << " at address 0x" << hex << er->ExceptionAddress << dec
        << " inside " << fileName << " loaded from base address 0x" << hex << minfo.lpBaseOfDll << "\n";

    return strStream.str();
}

int main(int argc, char* argv[])
{
    EXCEPTION_REGISTRATION newHandler = { pHandlerBase->oldHandler,
        (EXCEPTION_REGISTRATION::pFncHandler)(ExceptionHandler) };

    pHandlerBase->oldHandler = &newHandler;
    RaiseException(STATUS_NO_MEMORY, 0, 0, NULL);
    return 0;
}
