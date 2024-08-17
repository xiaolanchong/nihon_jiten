
#include "pch.h"
#include <memory>
#include <mutex>
#include <cassert>
#include <fstream>
#include <string>
#include "UtoolsDll.h"

namespace
{
   UtoolsDll dll("utools_orig.dll");

   std::wofstream dumpCat("dump_cat.txt", std::ios_base::trunc);
   std::wofstream dumpTree("dump_tree.txt", std::ios_base::trunc);

   std::pair<LONG, LONG> getBounds(const VARIANT& var)
   {
      HRESULT hr = E_FAIL;
      auto dims = SafeArrayGetDim(var.parray);
      assert(dims == 1);
      LONG lbound = 0;
      LONG ubound = 0;
      hr = SafeArrayGetLBound(var.parray, 1, &lbound);
      assert(hr == S_OK);
      hr = SafeArrayGetUBound(var.parray, 1, &ubound);
      assert(hr == S_OK);
      return { lbound, ubound };
   }

   void traverseArray(const VARIANT& var, size_t level, std::wostream& dump)
   {
      switch (var.vt)
      {
      case VT_ARRAY | VT_VARIANT:
         {
            HRESULT hr = E_FAIL;
            const auto [lbound, ubound] = getBounds(var);
            
            for (auto i = lbound; i <= ubound; ++i)
            {
               LONG indices[] = { i };
               CComVariant elem;
               hr = SafeArrayGetElement(var.parray, indices, &elem);
               assert(hr == S_OK);
               traverseArray(elem, level + 1, dump);
            }
         }
         break;
      case VT_ARRAY | VT_BSTR:
         {
            HRESULT hr = E_FAIL;
            const auto [lbound, ubound] = getBounds(var);

            dump << std::wstring(2 * level, ' ') << L"[";
            for (auto i = lbound; i <= ubound; ++i)
            {
               LONG indices[] = { i };
               CComBSTR elem;
               hr = SafeArrayGetElement(var.parray, indices, &elem);
               assert(hr == S_OK);
               dump << (const wchar_t*)elem;
               if (i != ubound )
                  dump << L", ";
            }
            dump << L"]" << std::endl;
         }
         break;
      case VT_BSTR:
         dump << std::wstring(2 * level, ' ') << var.bstrVal << std::endl;
         break;
      case VT_UI2:
         dump << std::wstring(2 * level, ' ') << var.uiVal << std::endl;
         break;
      default:
         assert(!"Unknown variant type");
      }
   }
}


extern "C"
{

#pragma comment(linker, "/export:_OpenCatDatabases@4")
std::uint32_t __stdcall OpenCatDatabases(const char* libPath)
{
   auto res = dll.OpenCatDatabases(libPath);
   return res;
}

#pragma comment(linker, "/export:_CloseCatDatabases@0")
std::uint32_t __stdcall CloseCatDatabases()
{
   return 1;
}

#pragma comment(linker, "/export:_OpenTitleDatabases@4")
std::uint32_t __stdcall OpenTitleDatabases(const char* libPath)
{
   auto res = dll.OpenTitleDatabases(libPath);
   return res;
}

#pragma comment(linker, "/export:_OpenWordIndex@4")
std::uint32_t __stdcall OpenWordIndex(const char* libPath)
{
   return dll.OpenWordIndex(libPath);
}

#pragma comment(linker, "/export:_CloseWordIndex@0")
std::uint32_t __stdcall CloseWordIndex()
{
   return 1;
}

#pragma comment(linker, "/export:_ReadTree@4=_ReadTree@8")
VARIANT* __stdcall ReadTree(VARIANT* result, const char* categoryFileName)
{
   auto res = dll.ReadTree(result, categoryFileName);
   traverseArray(*res, 0, dumpTree);
   dumpTree << std::wstring(25, '-') << std::endl;
   return res;
}

#pragma comment(linker, "/export:_GetCatFiles@4=_GetCatFiles@8")
VARIANT* __stdcall GetCatFiles(VARIANT* result, VARIANT* cats)
{
   auto res = dll.GetCatFiles(result, cats);
   traverseArray(*res, 0, dumpCat);
   dumpCat << std::wstring(25, '-') << std::endl;
   return res;
}

#if 0
_EntryMapRetrieve@12
#endif

#pragma comment(linker, "/export:_EntryMapOpen@8")
std::uint32_t __stdcall EntryMapOpen(const char* dbFileName, const char* mode)
{
   auto res = dll.EntryMapOpen(dbFileName, mode);
   return res;
}

#pragma comment(linker, "/export:_EntryMapClose@0")
std::uint32_t __stdcall EntryMapClose()
{
   return 1;
}

#pragma comment(linker, "/export:_CaptionMapOpen@8")
std::uint32_t __stdcall CaptionMapOpen(const char* dbFileName, std::uint32_t mode)
{
   return 1;
}

#pragma comment(linker, "/export:_MediaMapOpen@8")
std::uint32_t __stdcall MediaMapOpen(const char* dbFileName, std::uint32_t mode)
{
   return 1;
}

#pragma comment(linker, "/export:_DictMapOpen@4")
std::uint32_t __stdcall DictMapOpen(const char* dbFileName)
{
   return dll.DictMapOpen(dbFileName);
}

// these function are only to set up the html generator, not opening files

#pragma comment(linker, "/export:_SetCaptionScript@4")
std::uint32_t __stdcall SetCaptionScript(const char* captionScriptFileName)
{
   return 1;
}

#pragma comment(linker, "/export:_SetCaptionStyle@4")
std::uint32_t __stdcall SetCaptionStyle(const char* captionStyleFileName)
{
   return 1;
}

#pragma comment(linker, "/export:_SetDictScript@4")
std::uint32_t __stdcall SetDictScript(const char* dictScriptFileName)
{
   return 1;
}

#pragma comment(linker, "/export:_SetDictStyle@4")
std::uint32_t __stdcall SetDictStyle(const char* dictStyleFileName)
{
   return 1;
}

#pragma comment(linker, "/export:_SetEntryPrintStyle@4")
std::uint32_t __stdcall SetEntryPrintStyle(const char* printStyleFileName)
{
   return 1;
}

#pragma comment(linker, "/export:_SetEntryScript@4")
std::uint32_t __stdcall SetEntryScript(const char* entryScriptFileName)
{
   return 1;
}

#pragma comment(linker, "/export:_SetEntryStyle@4")
std::uint32_t __stdcall SetEntryStyle(const char* entryStyleFileName)
{
   return 1;
}

#pragma comment(linker, "/export:_SetMediaPath@4")
std::uint32_t __stdcall SetMediaPath(const char* mediaPath)
{
   return 1;
}

#pragma comment(linker, "/export:_SetMediaScript@4")
std::uint32_t __stdcall SetMediaScript(const char* mediaScriptFileName)
{
   return 1;
}

#pragma comment(linker, "/export:_SetMediaStyle@4")
std::uint32_t __stdcall SetMediaStyle(const char* mediaScriptFileName)
{
   return 1;
}

#pragma comment(linker, "/export:_GetImageSize@4=_GetImageSize@8")
std::uint32_t __stdcall GetImageSize(VARIANT* result, const char* imageFileName)
{
   return 1;
}

}