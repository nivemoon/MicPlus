#define COBJMACROS
#define _CRT_SECURE_NO_WARNINGS

#include "startup.h"

#include <objbase.h>
#include <shobjidl.h>
#include <stdio.h>
#include <string.h>

static BOOL startup_get_link_path(
    const AppPaths *paths,
    char *outPath,
    size_t bufferSize)
{
    if (!paths ||
        !outPath ||
        bufferSize == 0 ||
        paths->startupPath[0] == '\0') {
        return FALSE;
    }

    snprintf(
        outPath,
        bufferSize,
        "%s\\MicPlus.lnk",
        paths->startupPath
    );

    return TRUE;
}

BOOL startup_is_enabled(
    const AppPaths *paths)
{
    char linkPath[MAX_PATH];

    if (!startup_get_link_path(
            paths,
            linkPath,
            sizeof(linkPath))) {
        return FALSE;
    }

    return GetFileAttributesA(
        linkPath
    ) != INVALID_FILE_ATTRIBUTES;
}

BOOL startup_create(
    const AppPaths *paths)
{
    char linkPath[MAX_PATH];
    char exeDirectory[MAX_PATH];
    WCHAR wideLinkPath[MAX_PATH];

    char *slash;

    HRESULT hr;
    BOOL needUninitialize = FALSE;

    IShellLinkA *shellLink = NULL;
    IPersistFile *persistFile = NULL;

    if (!paths ||
        paths->exePath[0] == '\0') {
        return FALSE;
    }

    if (!startup_get_link_path(
            paths,
            linkPath,
            sizeof(linkPath))) {
        return FALSE;
    }

    hr = CoInitialize(NULL);

    if (SUCCEEDED(hr)) {
        needUninitialize = TRUE;
    } else if (hr != RPC_E_CHANGED_MODE) {
        return FALSE;
    }

    hr = CoCreateInstance(
        &CLSID_ShellLink,
        NULL,
        CLSCTX_INPROC_SERVER,
        &IID_IShellLinkA,
        (void **)&shellLink
    );

    if (FAILED(hr) ||
        !shellLink) {
        if (needUninitialize) {
            CoUninitialize();
        }

        return FALSE;
    }

    hr = IShellLinkA_SetPath(
        shellLink,
        paths->exePath
    );

    if (FAILED(hr)) {
        IShellLinkA_Release(shellLink);

        if (needUninitialize) {
            CoUninitialize();
        }

        return FALSE;
    }

    lstrcpynA(
        exeDirectory,
        paths->exePath,
        sizeof(exeDirectory)
    );

    slash = strrchr(
        exeDirectory,
        '\\'
    );

    if (slash) {
        *slash = '\0';
    }

    IShellLinkA_SetWorkingDirectory(
        shellLink,
        exeDirectory
    );

    hr = IShellLinkA_QueryInterface(
        shellLink,
        &IID_IPersistFile,
        (void **)&persistFile
    );

    if (SUCCEEDED(hr) &&
        persistFile) {
        if (MultiByteToWideChar(
                CP_ACP,
                0,
                linkPath,
                -1,
                wideLinkPath,
                ARRAYSIZE(wideLinkPath)) == 0) {
            IPersistFile_Release(persistFile);
            IShellLinkA_Release(shellLink);

            if (needUninitialize) {
                CoUninitialize();
            }

            return FALSE;
        }

        hr = IPersistFile_Save(
            persistFile,
            wideLinkPath,
            TRUE
        );

        IPersistFile_Release(
            persistFile
        );
    }

    IShellLinkA_Release(
        shellLink
    );

    if (needUninitialize) {
        CoUninitialize();
    }

    return SUCCEEDED(hr);
}

void startup_remove(
    const AppPaths *paths)
{
    char linkPath[MAX_PATH];

    if (!startup_get_link_path(
            paths,
            linkPath,
            sizeof(linkPath))) {
        return;
    }

    DeleteFileA(
        linkPath
    );
}

void startup_set(
    const AppPaths *paths,
    BOOL enable)
{
    if (enable) {
        startup_create(paths);
    } else {
        startup_remove(paths);
    }
}
