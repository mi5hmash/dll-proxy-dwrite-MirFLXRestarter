[![License: Unlicense](https://img.shields.io/badge/License-Unlicense-blueviolet.svg)](https://opensource.org/licenses/Unlicense)
[![Visual Studio 2022](https://custom-icon-badges.demolab.com/badge/Visual%20Studio%202022-5C2D91.svg?&logo=visual-studio&logoColor=white)](https://visualstudio.microsoft.com/)

# :interrobang: What is MirFLX

The MirFLX program, developed by Farelogix, serves as an interface between ticketing actions performed in the SPRK tool and an agency's back-office accounting system. It facilitates the processing of Machinable Interface Record (MIR) transactions, enabling agencies to request and acquire ticketing data from servers for accounting and invoicing purposes.

# :thinking: The problem to solve

<img src="https://github.com/mi5hmash/dll-proxy-dwrite-MirFLXRestarter/blob/main/.resources/images/error_cant_connect.png" alt="Error_Cant_Connect"/>

When the program encounters an issue connecting to the Farelogix server, the download service halts. It then has to be manually restarted using the appropriate button in the menu or by restarting the program. Until this is done, new transactions will not be downloaded, and back-office processes come to a standstill.

This is problematic because:
* The program might freeze and interrupt operations several times a day.
* The local machine administrator is not notified about the halt of the download task and only becomes aware of the issue when someone reports missing data in the back-office system.

# :bulb: My solution 
Since the program lacks an auto-restart feature and its source code isn't open, the simplest solution for me was to create a new thread that would monitor the status of transaction downloads and restart the program in case of any issues.

To achieve this, I located a spot in memory where a boolean value is stored, determining whether the program sends requests for new transactions to the Farelogix servers. Fortunately, the pointer to this value is static, so it is stored in the same location on every machine, as long as the program version does not change.

The next step was to write code that creates a new thread to check the aforementioned value every three minutes. If it is different from "1", the program gets restarted.

The remaining challenge was figuring out how to inject my code into the program. To this end, I created a DLL library that poses as "DWrite.dll" but is actually a proxy between the program and the real library. This allows my code to run while maintaining the original library's functionality.

# :man_mechanic: Installation
> [!CAUTION]
> I am not responsible for any damage that may result from the use of my modification. You use it at your own risk.

1. Navigate to the folder where MirFLX was installed. 

> [!TIP]
> At the time of writing this, the default installation path is:
```text
C:\Program Files\Farelogix Inc\MirFLX Install
```

2. Compile the code yourself or download the [latest release](https://github.com/mi5hmash/dll-proxy-dwrite-MirFLXRestarter/releases/latest), then extract the DLL file that matches the program's architecture.

3. Make sure that the SHA256 checksum of the EXE file matches the one in the release notes or the one included in the source code.

> [!TIP]
> You can display the file's checksum using the following command in the cmd console:
```cmd
powershell -Command "(Get-FileHash -Algorithm SHA256 'C:\Program Files\Farelogix Inc\MirFLX Install\MirFLX.exe').Hash"
```

4. If it matches, paste the extracted DLL file into the main installation directory of the program.

<img src="https://github.com/mi5hmash/dll-proxy-dwrite-MirFLXRestarter/blob/main/.resources/images/dll_destination.png" alt="Dll_Destination"/>

5. You can now launch the MirFLX.exe and test the modification by manually halting the downloads and waiting for 3 minutes. If the program restarts after this time, everything is working as expected.

# :scroll: Additional notes
| MirFLX Version | Architecture | Pointer        | SHA256         |
|----------------|--------------|----------------|----------------|
| 2.3            | x86          | clr.dll+749204 | 60B17B1536032D78904032CC3D882633B0074847101E01303C9FA4E837217651 |
| 2.3            | x64          | clr.dll+912E54 | BB2B9E99D1B91044C18036CA15074689DC0785A1F2DB0A5D6515A9FA0060BE1D |