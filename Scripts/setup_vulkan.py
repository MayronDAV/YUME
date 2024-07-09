import os
import sys
import subprocess
from pathlib import Path
import platform

import utils

from io import BytesIO
from urllib.request import urlopen

class VulkanConfiguration:
    requiredVulkanVersion = "1.3.283.0"
    installVulkanVersion = "1.3.283.0"
    vulkanDirectory = "./Thirdparty/VulkanSDK"
    system = platform.system().lower()

    @classmethod
    def Validate(cls):
        if (not cls.CheckVulkanSDK()):
            print("Vulkan SDK not installed correctly.")
            return
            
        if (not cls.CheckVulkanSDKDebugLibs()):
            print("\nNo Vulkan SDK debug libs found. Install Vulkan SDK with debug libs.")

    @classmethod
    def CheckVulkanSDK(cls):
        vulkanSDK = os.environ.get("VULKAN_SDK")
        if (vulkanSDK is None):
            print("\nYou don't have the Vulkan SDK installed!")
            cls.__InstallVulkanSDK()
            return False
        else:
            print(f"\nLocated Vulkan SDK at {vulkanSDK}")

        if (cls.requiredVulkanVersion not in vulkanSDK):
            print(f"You don't have the correct Vulkan SDK version! (Engine requires {cls.requiredVulkanVersion})")
            cls.__InstallVulkanSDK()
            return False
    
        print(f"Correct Vulkan SDK located at {vulkanSDK}")
        return True

    @classmethod
    def __InstallVulkanSDK(cls):
        permissionGranted = False
        while not permissionGranted:
            reply = str(input("Would you like to install VulkanSDK {0:s}? [Y/N]: ".format(cls.installVulkanVersion))).lower().strip()[:1]
            if reply == 'n':
                return
            permissionGranted = (reply == 'y')

        name = "VulkanSDK" if cls.system != "linux" else "vulkansdk-linux-x86_64"
        extension = "-Installer.exe" if cls.system != "linux" else ".tar.xz"
        vulkanInstallURL = f"https://sdk.lunarg.com/sdk/download/{cls.installVulkanVersion}/{cls.system}/{name}-{cls.installVulkanVersion}{extension}"
        vulkanPath = f"{cls.vulkanDirectory}/{name}-{cls.installVulkanVersion}{extension}"
        print("Downloading {0:s} to {1:s}".format(vulkanInstallURL, vulkanPath))
        utils.DownloadFile(vulkanInstallURL, vulkanPath)

        if (cls.system == "windows"):
            print("Running Vulkan SDK installer...")
            os.startfile(os.path.abspath(vulkanPath))

        if (cls.system == "linux"):
            print("Unziping Vulkan SDK...")
            utils.UnzipFile(vulkanPath)

            vulkanExeDir = f"{cls.vulkanDirectory}/{name}-{cls.installVulkanVersion}/{cls.installVulkanVersion}"
            vulkansdkInstaller = f"{vulkanExeDir}/{name.split('-')[0]}"
            if vulkansdkInstaller.exists():
                print("Running Vulkan SDK installer...")
                subprocess.run([f"./{vulkansdkInstaller}"], check=True)
            else:
                print("Vulkan SDK installer not found!")

            setupScript = f"{vulkanExeDir}/setup_env"
            if setupScript.exists():
                print("Setting up Vulkan SDK environment...")
                subprocess.run([f"./{setupScript}"], check=True)
            else:
                print("Setup script not found!")

        print("Re-run this script after installation!")
        quit()

    @classmethod
    def CheckVulkanSDKDebugLibs(cls):
        vulkanSDK = os.environ.get("VULKAN_SDK")
        shadercdLib = Path(f"{vulkanSDK}/Lib/shaderc_sharedd.lib")
        
        return shadercdLib.exists()

if __name__ == "__main__":
    VulkanConfiguration.Validate()