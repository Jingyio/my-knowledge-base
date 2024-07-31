@echo off

set "match=DebugConfig Listings Objects *.uvguix* *.uvoptx *.scvd"

echo.
echo =====================================================
echo ============ Clean VS project files =================
echo =====================================================
for %%i in (%match%) do (
    for /r %%j in (%cs% %%i) do (
        if exist %%j (
            echo [DELETE] %%j
            rd /s /q "%%j"
            del "%%j"
        )
    )
)

pause