#include <iostream>
#include <cstdint>
#include <type_traits>
#include <windows.h>
#include <atlbase.h>
#include <atlcom.h>
#include <vector>
#include <assert.h>
#include <fstream>
#include "../utools_stub/UtoolsDll.h"

const std::string rootEojDir = R"(e:\Program Files (x86)\Kodansha Encyclopedia of Japan)";
const auto libEojDir = rootEojDir + R"(\lib\)";

CComVariant createCategoryParameter(const wchar_t* catName)
{
   SAFEARRAYBOUND bounds[1];
   bounds[0].lLbound = 0;
   bounds[0].cElements = 2;

   SAFEARRAY* psa = SafeArrayCreate(VT_VARIANT, 1, bounds);

   HRESULT hr = E_FAIL;
   CComVariant nameElem(catName);
   LONG index1[] = { 0 };
   hr = ::SafeArrayPutElement(psa, index1, &nameElem);

   CComVariant idElem(short(0));
   LONG index2[] = { 1 };
   hr = ::SafeArrayPutElement(psa, index2, &idElem);

   CComVariant nestedElem;
   nestedElem.vt = VT_ARRAY | VT_VARIANT;
   nestedElem.parray = psa;

   /// upper level
   bounds[0].lLbound = 0;
   bounds[0].cElements = 1;

   psa = ::SafeArrayCreate(VT_VARIANT, 1, bounds);
   hr = ::SafeArrayPutElement(psa, index1, &nestedElem);

   CComVariant res;
   res.vt = VT_ARRAY | VT_VARIANT;
   res.parray = psa;

   return res;
}

struct SubCategory
{
   std::string id;
   std::string name;
};

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

std::vector<SubCategory> convertSubCategory(const CComVariant& subCatArray)
{
   assert(subCatArray.vt == (VT_ARRAY | VT_VARIANT));

   std::vector<SubCategory> result;

   HRESULT hr = E_FAIL;
   const auto [lbound, ubound] = getBounds(subCatArray);

   result.reserve(ubound - lbound + 1);
   for (auto i = lbound; i <= ubound; ++i)
   {
      LONG indices[] = { i };
      CComVariant elem;
      hr = SafeArrayGetElement(subCatArray.parray, indices, &elem);
      assert(hr == S_OK);
      
      {
         assert(elem.vt == (VT_ARRAY | VT_BSTR));
         const auto [innerlbound, innerubound] = getBounds(elem);
         assert(innerubound - innerlbound + 1 == 2);

         CComBSTR entityId;
         LONG index1[] = { innerlbound + 0 };
         hr = SafeArrayGetElement(elem.parray, index1, &entityId);
         assert(hr == S_OK);

         CComBSTR subCatName;
         LONG index2[] = { innerlbound + 1 };
         hr = SafeArrayGetElement(elem.parray, index2, &subCatName);
         assert(hr == S_OK);

         result.push_back({ (const char*) CW2A(entityId), (const char*)CW2A(subCatName)});
      }
   }

   return result;
}

std::vector<SubCategory> getSubCategories(UtoolsDll& dll, const std::string& catName)
{
   CComVariant catNameVar = createCategoryParameter(CComBSTR(catName.c_str()));
   CComVariant catRes;
   dll.GetCatFiles(&catRes, &catNameVar);
   return convertSubCategory(catRes);
}

void getAllSubcategories(UtoolsDll& dll)
{
   std::ifstream toc(R"(c:\project\html\nihon_jiten\toc.txt)");
   std::ofstream tocWithSubcats(R"(c:\project\html\nihon_jiten\toc_titles.txt)", std::ios_base::trunc);

   std::string line;
   size_t counter = 0;
   while (std::getline(toc, line))
   {
      tocWithSubcats << line << std::endl;

      auto pos = line.find_first_not_of('\t');
      auto pos_after = line.find('\t', pos);
      auto firstToken = line.substr(pos, pos_after - pos);
      if (firstToken.size() != 5 || islower(firstToken[0]) || islower(firstToken[1]))
      {
         continue;
      }

      const auto prefix = line.substr(0, pos) + '\t';
      auto subCats = getSubCategories(dll, firstToken);

      for (const auto& [entityId, subCatName] : subCats)
      {
         tocWithSubcats << prefix << entityId << '\t' << subCatName << std::endl;
      }

      //if (counter > 2)
      //   return;
      ++counter;
   }
}

void initEncylopedia(UtoolsDll& dll)
{
   dll.OpenCatDatabases(libEojDir.c_str());
   dll.OpenTitleDatabases(libEojDir.c_str());
   const auto eojArticleFile = libEojDir + "eoj.udb";
   const char mode = 'r';
   dll.EntryMapOpen(eojArticleFile.c_str(), &mode);
}

void testCategory()
{
   UtoolsDll dll(rootEojDir + R"(\bin\utools.dll)");
   initEncylopedia(dll);

   CComVariant treeVar;
   dll.ReadTree(&treeVar, (libEojDir + "cl1-tree.txt").c_str());

   auto subCats = getSubCategories(dll, "LS100");

   dll.EntryMapRetrieve("AR000104", R"(c:\project\html\nihon_jiten\cpp\utools_export\output\AR000104.html)", 0);
   
   getAllSubcategories(dll);
   int a = 1;
}

bool retrieveWord(const std::string& word, const std::string& outputFileName)
{
   UtoolsDll dll(rootEojDir + R"(\bin\utools.dll)");
   dll.DictMapOpen((libEojDir + "dict.udb").c_str());

   auto res = dll.DictMapRetrive(word.c_str(), outputFileName.c_str(), 1);
   return res != 0;
}

bool retrieveArticle(const std::string& id, const std::string& fileName)
{
   UtoolsDll dll(rootEojDir + R"(\bin\utools.dll)");
   initEncylopedia(dll);
   return dll.EntryMapRetrieve(id.c_str(), fileName.c_str(), 0);
}

int main(int argc, const char* argv[])
{
   if (argc != 4)
   {
      std::cout << "utools_export: mode entity filename" << std::endl;
      return 0;
   }
   
   std::string mode = argv[1];

   if (mode == "article")
   {
      std::string entityId = argv[2];
      std::string outputFileName = argv[3];
      auto res = retrieveArticle(entityId, outputFileName);
      return res ? 0 : -1;
   }

   if (mode == "word")
   {
      std::string word = argv[2];
      std::string outputFileName = argv[3];
      auto res = retrieveWord(word, outputFileName);
      return res ? 0 : -1;
   }

   if (mode == "category")
   {
      testCategory();
      return 0;
   }

   return 0;
}
