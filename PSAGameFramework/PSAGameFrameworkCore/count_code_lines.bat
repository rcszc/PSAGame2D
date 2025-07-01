@echo off
setlocal enabledelayedexpansion
set total_lines=0
set "directory=%cd%"
for /r "%directory%" %%f in (*.h *.hpp *.cpp *.vert *.frag) do (
    for /f "tokens=3" %%l in ('find /c /v "" "%%f"') do (
        set /a total_lines+=%%l
	echo Processing file: %%f
    )
)
echo Total lines: !total_lines!

endlocal
pause