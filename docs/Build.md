# Build VCubeTracker

## Requirements

- Windows 10 64-bit
- Visual Studio 2019
- Git for Windows
- CMake 3.15 or later

## Get qedit.h

1. Download Microsoft Windows Software Development Kit Update for Windows Vista (6.1.6000.16384.10.WindowsSDK_Vista_Feb2007Update_rtm.DVD.Rel.iso) from https://www.microsoft.com/en-us/download/details.aspx?id=14477

2. Open `Setup\WinSDKBuild-SDK_DirectShow_BLD_Common-common.0.cab` from the iso file.

3. Save `qedit_h.99023124_2CFC_4698_80A9_F84FC02DCB6C` as `qedit.h`.

## Set up the Project

1. Locate `qedit.h` into `vendor\winsdk6.1` folder.

2. Open *Git Bash* then clone this repository.

    ```shell
    git clone https://github.com/kurotu/VCubeTracker.git
    ```

3. Then execute `bootstrap.sh`.

    ```shell
    cd /PATH/TO/VCubeTracker
    ./bootstrap.sh
    ```

    This step takes some time.

4. Open build/VCubeTracker.sln then build `INSTALL` target with **Release** configuration.

5. Files are generated to `build\\dist` folder.
