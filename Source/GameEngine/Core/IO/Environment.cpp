// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.0 (2010/01/01)

#include "Core/Core.h"
#include "Environment.h"

eastl::vector<eastl::string>* Environment::msDirectories = 0;

//----------------------------------------------------------------------------
// Initialization/termination support.
//----------------------------------------------------------------------------
void Environment::OnInitialize()
{
    if (!msDirectories)
    {
        msDirectories = new eastl::vector<eastl::string>();
        return;
    }

    LogError("Directory list already allocated.\n");
}
//----------------------------------------------------------------------------
void Environment::OnTerminate()
{
    if (msDirectories)
    {
        delete(msDirectories);
        return;
    }

    LogError("No directory list to deallocate.\n");
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
const eastl::wstring Environment::GetWorkingDirectory()
{
	eastl::wstring wdir;
	#if defined(_WINDOWS_API_)
		wchar_t tmp[_MAX_PATH];
		_wgetcwd(tmp, _MAX_PATH);
		wdir = tmp;
		eastl::replace(wdir.begin(), wdir.end(), '\\', '/');

		wdir.validate();
	#endif

	return wdir;
}

//----------------------------------------------------------------------------
eastl::string Environment::GetAbsolutePath(const eastl::string& filename)
{
#if defined(_WINDOWS_API_)
	char *p=0;
	char fpath[_MAX_PATH];
	p = _fullpath(fpath, filename.c_str(), _MAX_PATH);
	eastl::string tmp(p);
	eastl::replace(tmp.begin(), tmp.end(), '\\', '/');
	tmp.validate();

	return tmp;
#else
	return path(filename);
#endif
}

//----------------------------------------------------------------------------
bool Environment::InsertDirectory(const eastl::string& directory)
{
    if (!msDirectories)
    {
        OnInitialize();
    }

    eastl::vector<eastl::string>::iterator iter = msDirectories->begin();
    eastl::vector<eastl::string>::iterator end = msDirectories->end();
    for (/**/; iter != end; ++iter)
    {
        if (directory == *iter)
        {
            return false;
        }
    }
    msDirectories->push_back(directory);
    return true;
}
//----------------------------------------------------------------------------
bool Environment::RemoveDirectory(const eastl::string& directory)
{
    if (!msDirectories)
    {
		OnInitialize();
    }

    eastl::vector<eastl::string>::iterator iter = msDirectories->begin();
    eastl::vector<eastl::string>::iterator end = msDirectories->end();
    for (/**/; iter != end; ++iter)
    {
        if (directory == *iter)
        {
            msDirectories->erase(iter);
            return true;
        }
    }
    return false;
}
//----------------------------------------------------------------------------
void Environment::RemoveAllDirectories()
{
    if (!msDirectories)
    {
		OnInitialize();
    }

    msDirectories->clear();
}
//----------------------------------------------------------------------------
eastl::string Environment::GetPathR(const eastl::string& fileName)
{
    return GetPath(fileName, "r");
}
//----------------------------------------------------------------------------
eastl::string Environment::GetPathW(const eastl::string& fileName)
{
    return GetPath(fileName, "w");
}
//----------------------------------------------------------------------------
eastl::string Environment::GetPathRW(const eastl::string& fileName)
{
    return GetPath(fileName, "r+");
}
//----------------------------------------------------------------------------
eastl::string Environment::GetPath(const eastl::string& fileName, const char* attributes)
{
    if (!msDirectories)
    {
		OnInitialize();
    }

	eastl::vector<eastl::string>::iterator iter = msDirectories->begin();
	eastl::vector<eastl::string>::iterator end = msDirectories->end();
	for (/**/; iter != end; ++iter)
	{
		eastl::string decorated = *iter + fileName;
		FILE* testFile = fopen(decorated.c_str(), attributes);
		if (testFile)
		{
			fclose(testFile);
			return decorated;
		}
	}

    return eastl::string();
}
//----------------------------------------------------------------------------
