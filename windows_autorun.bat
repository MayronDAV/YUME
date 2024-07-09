@echo off
python --version
IF %ERRORLEVEL% NEQ 0 (
	echo Python isn't installed
	echo Installing Python...
	bitsadmin /transfer myDownloadJob /download /priority normal https://www.python.org/ftp/python/3.12.4/python-3.12.4-amd64.exe %cd%\Thirdparty\Python\Windows\python-3.12.4-amd64.exe
	echo Running Python Installer...
	start Thirdparty\Python\Windows\python-3.12.4-amd64.exe
) ELSE (
	echo Python is installed
	echo Setting up project...
	call Scripts/Windows/windows_setup.bat
)
PAUSE