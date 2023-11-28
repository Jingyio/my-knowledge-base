@echo off

set "match=.vs x86 x64 ARM ARM64 Debug Release bin obj"

echo.
echo =====================================================
echo ============ Clean VS project files =================
echo =====================================================
for %%i in (%match%) do (
    for /r %%j in (%cs% %%i) do (
        if exist %%j (
            echo [DELETE] %%j
            rd /s /q "%%j"
        )
    )
)

pause