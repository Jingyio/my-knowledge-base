@chcp 65001
@echo off

set "search=EXIT"
set "replace=EXTI"

echo.
echo =====================================================
echo ============ Rename project folders =================
echo =====================================================
setlocal enabledelayedexpansion
for /d /r %%f in (*) do (
    set "originName=%%f"
    set "newName=%%~nxf"
    set "newName=!newName:%search%=%replace%!"

    if "%%~nxf" neq "!newName!" (
        echo [RENAME] Folder "!originName!" to "!newName!"
        ren "!originName!" "!newName!"
    ) else (
        echo [SKIP  ] Folder "!originName!"
    )
)
endlocal

echo.
echo =====================================================
echo ============ Rename project files ===================
echo =====================================================
setlocal enabledelayedexpansion
for /r %%f in (*) do (
    set "originName=%%f"
    set "newName=%%~nxf"
    set "newName=!newName:%search%=%replace%!"

    if "%%~nxf" neq "!newName!" (
        echo [RENAME] file "!originName!" to "!newName!"
        ren "!originName!" "!newName!"
    ) else (
        echo [SKIP  ] File "!originName!"
    )
)
endlocal

echo.
echo =====================================================
echo ============ Modify keywords in files ===============
echo =====================================================
setlocal enabledelayedexpansion
for /r %%f in (*) do (
    :: Do not process the current file"
    if "%%f" neq "%~dp0%~nx0" (

        :: Replace string %search% with %replace%"
        set "file=%%f"
        set "absPath=!file:%cd%\=!"
        start python ReplaceFileContent.py !absPath! !search! !replace!
        
        echo [MODIFY] File "!file!"
    )
)
endlocal

echo.
echo.
pause