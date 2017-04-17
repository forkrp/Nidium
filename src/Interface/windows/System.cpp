/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#include "System.h"

#include <direct.h>
#include <Shlobj.h>
#include <locale.h>
#include <atlstr.h>  

#include <Port/MSWindows.h>

#ifndef _MSC_VER
#error Windows port must be compiled with msvc
#endif

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

namespace Nidium {
namespace Interface {

System::System() : m_EmbedPath(NULL)
    {
     m_fBackingStorePixelRatio = 1.0;

    char nidiumPath[MAX_PATH];
    char parent[MAX_PATH], dir[MAX_PATH];
    if (GetModuleFileName(NULL, nidiumPath, MAX_PATH) != 0) {
        const char *embed = "src/Embed/";
        _splitpath(nidiumPath, NULL, &parent[0], NULL, NULL);
        _splitpath(&parent[0], NULL, &dir[0], NULL, NULL);
        size_t len = strlen(dir) + strlen(embed) + 2;
        m_EmbedPath = static_cast<char *>(malloc(sizeof(char) * len));
        snprintf(m_EmbedPath, len, "%s/%s", dir, embed);
    }
}

System::~System()
{
    if (m_EmbedPath) {
        free(m_EmbedPath);
    }
}

const char *System::getCacheDirectory()
{
    const char *homedir = getUserDirectory();
    char nHome[4096];

    snprintf(nHome, 4096, "%s.config/nidium/", homedir);

    if (_mkdir(nHome) == -1 && errno != EEXIST) {
        ndm_logf(NDM_LOG_ERROR, "System", "Can't create cache directory %s", nHome);
        return NULL;
    }

    return strdup(nHome);
}

const char *System::getEmbedDirectory()
{
    return m_EmbedPath;
}

const char* System::getUserDirectory() {
   // caller must free memory after receiving result
   TCHAR *path = new TCHAR[128];
   SHGetSpecialFolderPath(NULL, path, CSIDL_PERSONAL, FALSE);
    return path;
};

const char *System::getLanguage()
{

    const char *lang;

    lang = setlocale(LC_COLLATE, NULL);

    return lang;
}


const char *System::cwd()
{
    static char dir[MAXPATHLEN];

    getcwd(dir, MAXPATHLEN);

    return dir;
}

void System::sendNotification(const char *title, const char *content, bool sound)
{
    HWND hWnd = NULL;
    LPCTSTR lpText = content;
    LPCTSTR lpCaption = title;
    UINT uType = MB_OK;

    MessageBox(hWnd, lpText, lpCaption, uType);

}

const char *System::execute(const char *cmd)
{
    // http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
    CStringA strResult;
    HANDLE hPipeRead, hPipeWrite;

    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
    saAttr.bInheritHandle = TRUE;   //Pipe handles are inherited by child process.
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe to get results from child's stdout.
    if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0)) {
        return strResult;
    }

    STARTUPINFOW si = {sizeof(STARTUPINFOW)};
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.hStdOutput = hPipeWrite;
    si.hStdError = hPipeWrite;
    si.wShowWindow = SW_HIDE;       // Prevents cmd window from flashing. Requires STARTF_USESHOWWINDOW in dwFlags.

    PROCESS_INFORMATION pi = { 0 };

    BOOL fSuccess = CreateProcessW(NULL, (LPWSTR)cmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    if (!fSuccess) {
       CloseHandle(hPipeWrite);
        CloseHandle(hPipeRead);
        return strResult;
    }

    bool bProcessEnded = false;
    for (; !bProcessEnded;) {
        // Give some timeslice (50ms), so we won't waste 100% cpu.
        bProcessEnded = WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0;

        // Even if process exited - we continue reading, if there is some data available over pipe.
        for (;;) {
            char buf[1024];
            DWORD dwRead = 0;
            DWORD dwAvail = 0;

            if (!::PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL))
                break;

            if (!dwAvail) // no data available, return
                break;

            if (!::ReadFile(hPipeRead, buf, MIN(sizeof(buf) - 1, dwAvail), &dwRead, NULL) || !dwRead)
                // error, the child process might ended
                break;

            buf[dwRead] = 0;
            strResult += buf;
        }
    } //for

    CloseHandle(hPipeWrite);
    CloseHandle(hPipeRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return strResult;

}

} // namespace Interface
} // namespace Nidium

