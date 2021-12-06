import platform
import subprocess

if __name__ == "__main__":
    print("Running premake5...")

    OS_NAME: str = platform.system()

    if OS_NAME == "Windows":
        subprocess.run("premake5 vs2019", cwd = "..")
    elif OS_NAME == "Darwin":
        subprocess.run("premake5 xcode4", cwd = "..")
    elif OS_NAME == "Linux":
        subprocess.run("premake5 gmake2", cwd = "..")
    else:
        print("Unidentified operating system. Using default Premake build action.");
        subprocess.run("premake5", cwd = "..")
