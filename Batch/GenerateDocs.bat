@echo off
echo ================================
echo Generate Doxygen Documentation
echo ================================
echo.

cd /d "%WJWORLD_ROOT%"
doxygen Doxyfile

echo.
echo ================================
echo Done!
echo ================================
echo.
pause
