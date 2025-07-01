@echo off
cd /d %~dp0

REM 删除目录 
set RelativeDir=.vs\PSAGameFramework\v17\ipch\

if exist "%RelativeDir%" (
	for /d %%p in ("%RelativeDir%\*.*") do rmdir "%%p" /s /q
	for %%p in ("%RelativeDir%\*.*") do del "%%p" /f /q

	echo All files in %RelativeDir% have been deleted.
) else (
	echo Directory %RelativeDir% does not exist.
)

git add .

set /p CommitMessage="Enter commit message: "
git commit -m "%CommitMessage%"

git push origin master

pause