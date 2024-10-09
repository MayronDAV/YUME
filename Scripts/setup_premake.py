import sys
import os
from pathlib import Path
import platform

import utils

class PremakeConfiguration:
    premakeVersion = "5.0.0-beta2"
    system = platform.system().lower()
    extension = "zip" if system != "linux" else "tar.gz"
    premakeZipUrls = f"https://github.com/premake/premake-core/releases/download/v{premakeVersion}/premake-{premakeVersion}-{system}.{extension}"
    premakeLicenseUrl = "https://raw.githubusercontent.com/premake/premake-core/master/LICENSE.txt"
    premakeDirectory = os.path.join(".", "Thirdparty", "premake", "bin", system)

    @classmethod
    def Validate(cls):
        if (not cls.CheckIfPremakeInstalled()):
            print("Premake is not installed.")
            return False

        print(f"Correct Premake located at {os.path.abspath(cls.premakeDirectory)}")
        return True

    @classmethod
    def CheckIfPremakeInstalled(cls):
        premakeExe = Path(os.path.join(cls.premakeDirectory, "premake5.exe"))
        if (cls.system == "linux"): 
            premakeExe = Path(os.path.join(cls.premakeDirectory, "premake5"))
        
        if (not premakeExe.exists()):
            return cls.InstallPremake()

        return True

    @classmethod
    def InstallPremake(cls):
        permissionGranted = False
        while not permissionGranted:
            reply = str(input("Premake not found. Would you like to download Premake {0:s}? [Y/N]: ".format(cls.premakeVersion))).lower().strip()[:1]
            if reply == 'n':
                return False
            permissionGranted = (reply == 'y')

        premakePath = os.path.join(cls.premakeDirectory, f"premake-{cls.premakeVersion}-{cls.system}.{cls.extension}")
        print("Downloading {0:s} to {1:s}".format(cls.premakeZipUrls, premakePath))
        utils.DownloadFile(cls.premakeZipUrls, premakePath)
        print("Extracting", premakePath)
        utils.UnzipFile(premakePath, deleteZipFile=True)
        print(f"Premake {cls.premakeVersion} has been downloaded to '{cls.premakeDirectory}'")

        if cls.system == "linux":
            premakeExePath = os.path.join(cls.premakeDirectory, "premake5")
            os.chmod(premakeExePath, 0o755)

        premakeLicensePath = os.path.join(cls.premakeDirectory, "LICENSE.txt")
        print("Downloading {0:s} to {1:s}".format(cls.premakeLicenseUrl, premakeLicensePath))
        utils.DownloadFile(cls.premakeLicenseUrl, premakeLicensePath)
        print(f"Premake License file has been downloaded to '{cls.premakeDirectory}'")

        return True
