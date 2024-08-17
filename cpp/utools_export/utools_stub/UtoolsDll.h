#pragma once

#include <string>
#include <memory>
#include <windows.h>

class UtoolsDll
{
public:
   explicit UtoolsDll(const std::string& fileName)
      : m_dll(::LoadLibraryA(fileName.c_str()), ::FreeLibrary)
   {
      getProcs();
   }

   /// <summary>
   /// Opens category files (category name to article name)
   /// </summary>
   /// <param name="libPath">Path to EoJ lib directory</param>
   /// <returns>1 - success, 0 - failure</returns>
   std::uint32_t OpenCatDatabases(const char* libPath)
   {
      return m_funcs.openCatDatabases(libPath);
   }

   /// <summary>
   /// Open title files (both English and Japanese), will be h1 tag in html
   /// </summary>
   /// <param name="libPath">Path to EoJ lib directory</param>
   /// <returns>1 - success, 0 - failure</returns>
   std::uint32_t OpenTitleDatabases(const char* libPath)
   {
      return m_funcs.openTitleDatabases(libPath);
   }

   /// <summary>
   /// Opens English-Japanese dictionary
   /// </summary>
   /// <param name="dictFileName"></param>
   /// <returns></returns>
   std::uint32_t DictMapOpen(const char* dictFileName)
   {
      return m_funcs.dictMapOpen(dictFileName);
   }

   /// <summary>
   /// Opens the main article file
   /// </summary>
   /// <param name="dictFileName"></param>
   /// <param name="mode"></param>
   /// <returns></returns>
   std::uint32_t EntryMapOpen(const char* dictFileName, const char* mode)
   {
      // 'r' readonly
      // 'c' append
      // 'w' write
      return m_funcs.entryMapOpen(dictFileName, mode);
   }

   std::uint32_t OpenWordIndex(const char* libPath)
   {
      return m_funcs.openWordIndex(libPath);
   }

   /// <summary>
   /// Opens and reads the category file
   /// </summary>
   /// <param name="result">Result</param>
   /// <param name="catFileName">Path to cl1-tree.txt</param>
   /// <returns>Same as <c>result</c></returns>
   VARIANT* ReadTree(VARIANT* result, const char* catFileName)
   {
      return m_funcs.readTree(result, catFileName);
   }

   /// <summary>
   /// Gets article entiry ids and titles
   /// </summary>
   /// <param name="result">[[enityId:BSTR, article title:BSTR], [enityId:BSTR, article title:BSTR], ..]</param>
   /// <param name="categories">[[entity Id, 0]]</param>
   /// <returns>Same as <c>result</c></returns>
   VARIANT* GetCatFiles(VARIANT* result, VARIANT* categories)
   {
      return m_funcs.getCatFiles(result, categories);
   }

   /// <summary>
   /// Forms an html article file on the given topic
   /// </summary>
   /// <param name="entityId">entityId, e.g. AR000104</param>
   /// <param name="outputHtmlFile">output html file name</param>
   /// <param name="useTextQuery">1 - use text query set by _SetTextQuery function</param>
   /// <returns></returns>
   std::uint32_t EntryMapRetrieve(const char* entityId, const char* outputHtmlFile, std::uint32_t useTextQuery)
   {
      return m_funcs.entryMapRetrieve(entityId, outputHtmlFile, useTextQuery);
   }

   std::uint32_t DictMapRetrive(const char* word, const char* outputHtmlFile, std::uint32_t arg3)
   {
      return m_funcs.dictMapRetrive(word, outputHtmlFile, arg3);
   }

private:
   using OpenDbFn = std::uint32_t(__stdcall*)(const char*);
   using ReadTreeFn = VARIANT * (__stdcall*)(VARIANT*, const char*);
   using GetCatFilesFn = VARIANT * (__stdcall*)(VARIANT*, VARIANT*);
   using EntryMapOpenFn = std::uint32_t(__stdcall*)(const char*, const char*);
   using EntryMapRetrieveFn = std::uint32_t(__stdcall*)(const char*, const char*, std::uint32_t);
   using DictMapRetrieveFn = std::uint32_t(__stdcall*)(const char*, const char*, std::uint32_t);

   struct ExportedFunctions
   {
      OpenDbFn openCatDatabases;
      OpenDbFn openTitleDatabases;

      OpenDbFn openWordIndex;
      OpenDbFn dictMapOpen;

      EntryMapOpenFn entryMapOpen;

      ReadTreeFn readTree;
      GetCatFilesFn  getCatFiles;
      EntryMapRetrieveFn entryMapRetrieve;
      DictMapRetrieveFn dictMapRetrive;
   };

   void* getProc(const char* procName)
   {
      void* proc = ::GetProcAddress(m_dll.get(), procName);
      if (proc == nullptr)
      {
         throw std::runtime_error(std::string("Can't locate ") + procName + "inside dll");
      }
      return proc;
   }

   void getProcs()
   {
      m_funcs.openCatDatabases = (OpenDbFn)getProc("_OpenCatDatabases@4");
      m_funcs.dictMapOpen = (OpenDbFn)getProc("_DictMapOpen@4");
      m_funcs.openTitleDatabases = (OpenDbFn)getProc("_OpenTitleDatabases@4");
      m_funcs.entryMapOpen = (EntryMapOpenFn)getProc("_EntryMapOpen@8");
      m_funcs.openWordIndex = (OpenDbFn)getProc("_OpenWordIndex@4");

      m_funcs.readTree = (ReadTreeFn)getProc("_ReadTree@4");
      m_funcs.getCatFiles = (GetCatFilesFn)getProc("_GetCatFiles@4");
      m_funcs.entryMapRetrieve = (EntryMapRetrieveFn)getProc("_EntryMapRetrieve@12");
      m_funcs.dictMapRetrive = (DictMapRetrieveFn)getProc("_DictMapRetrieve@12");
   }

private:
   std::shared_ptr<std::remove_pointer_t<HMODULE>> m_dll;
   ExportedFunctions m_funcs;
};
