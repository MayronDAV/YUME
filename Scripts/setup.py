
import os
import subprocess
import platform

from setup_python import PythonConfiguration as PythonRequirements

# Make sure everything we need for the setup is installed
PythonRequirements.Validate()

from setup_premake import PremakeConfiguration as PremakeRequirements
from setup_vulkan import VulkanConfiguration as VulkanRequirements
os.chdir('./') # Change from devtools/scripts directory to root

premakeInstalled = PremakeRequirements.Validate()
VulkanRequirements.Validate()

print("\nUpdating submodules...")
subprocess.call(["git", "submodule", "update", "--init", "--recursive"])

if (premakeInstalled):
    print("\nRunning premake...")
    system = platform.system()
    extension = "bat" if system != "Linux" else "sh"
    script_path = os.path.abspath(f"./Scripts/{system}/{system.lower()}_generate.{extension}")
    print(f"Script path: {script_path}")
    subprocess.call([script_path, "nopause"])

    print("\nSetup completed!")
else:
    print("YUME requires Premake to generate project files.")
