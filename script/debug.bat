@ECHO OFF

wt --window 0 -p "PowerShell" pwsh -Command "cd %cd% && gdb ./binary/windows/ResourceLoader.exe"
