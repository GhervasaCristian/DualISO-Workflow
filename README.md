DualISO Magic Lantern Scripted Workflow.

There are multiple tools created in the help of making our life easier.

------------------------
# DNG Unpack & Repack Tool
A high-performance C-based utility designed to manage large batches of files (specifically tailored for DNG or image workflows). This tool provides a unified interface to Unpack files into organized batches or Repack (flatten) them back into their parent directories.

📂 Project Location
src/DNG_Unpack_Repack

✨ Features
Unified Menu: A single executable to handle both unpacking and repacking workflows.

Automated Configuration: Reads the working directory from config.inf. If the file is missing, it prompts the user and generates a new one.

Batch Unpacking: Segregates files into subfolders (3 files per folder by default) to prevent directory bloating and improve processing speed.

Recursive Repacking: Automatically flattens subdirectories within the OUT folder, moving files back to their primary parent and cleaning up empty folders.

Windows Optimized: Utilizes Win32 API for fast file operations and directory traversal.

🛠 Directory Structure & what is actually doing
The tool expects a specific folder hierarchy based on the path defined in your config.inf (e.g., G:\Scripting):

[Base Path] 
FROM THIS
 ├── config.inf       # Stores the path to the root folder
 ├── IN/              # Source folder for UNPACK operations
      └── Folder_A/   # Files here will be batched
      └── Folder_B/   # Files here will be batched
TO THIS
 ├── config.inf       
 ├── IN/              
      └── Folder_A/ 
	  │ └── Folder_A/
	  │ └── Folder_A0/
	  │ └── Folder_A1/	  
      └── Folder_B/   	   
	    └── Folder_B/
	    └── Folder_B0/
	    └── Folder_B1/			   
		   
The Repack function is the reverse of the unpack function. It's usefull when you want to unpack files of a Cinema DNG clip.
		   
🚀 How To Use
1. Configuration
Place a file named config.inf in the same directory as the executable. Inside, paste the full path to your scripting directory: G:\Scripting

2. Running the Tool
Launch the executable. You will be presented with the following menu:

Unpack:

Looks inside [Base Path]\IN.

Creates a prefix folder based on the parent name.

Moves files into sub-batches of 3 (e.g., Prefix_000000, Prefix_000001).

Repack:

Looks inside [Base Path]\OUT.

Finds all sub-folders and moves their files "up" to the parent folder.

Deletes the now-empty sub-folders.

Exit: Safely closes the application.

🏗 Compilation
To compile this tool on Windows using GCC (MinGW):

Bash

gcc main.c -o DNG_Tool.exe

------------------------
# FileBlankMirroring

Command Syntax
The program expects exactly two arguments after the name:

DOS

FileBlankMirroring.exe <Folder_With_Real_Files> <Folder_To_Create_Mirrors>
Usage Examples
1. Manual Run (CMD/PowerShell): If you have a folder of large images at G:\Project\Originals and you want to create a lightweight test set in your IN folder:

DOS

FileBlankMirroring.exe "G:\Project\Originals" "G:\Scripting\IN"
2. Integration with your Batch Script: You can update your "Reset" batch script (from the first message) to use this tool to automatically populate your environment:

Code snippet

@echo off
cd /d "%~dp0"

:: Delete old IN and create fresh one
if exist "IN" rd /s /q "IN"
mkdir "IN"

:: Mirror real files as 1-byte placeholders for fast testing
FileBlankMirroring.exe "G:\Original_Backup" "G:\Scripting\IN"

echo Test environment is ready.
pause
Summary of logic
The code uses the Win32 API (CreateFileA and WriteFile) to ensure maximum speed on Windows. By writing exactly one 0 byte and then calling SetEndOfFile, it ensures the OS allocates the minimum possible space on the disk, making it much faster than a standard copy command.

------------------------
# FileBlankMirroring

The compares.c script (likely compiled as compares.exe) is a "Difference Finder" for photographers or editors. It looks for files that do not have a pair in another folder.

What it does:
Scans Two Folders: It looks inside FolderA and FolderB for specific file types (defaults are .cr2 and .dng).
Ignores Extensions: It compares files based on their basename (e.g., IMG_001.dng and IMG_001.cr2 are considered a "match").
Finds the "Loners": It identifies files that exist in Folder A but NOT in Folder B, and vice-versa.
Isolates Unmatched Files: It copies every file that doesn't have a matching partner into a third folder (defaults to a folder named UNMATCHED).

Example Scenario:
Folder A: photo1.dng, photo2.dng, photo3.dng
Folder B: photo1.cr2, photo2.cr2
Result: photo3.dng is copied to the UNMATCHED folder because it has no partner in Folder B.

1. The Basic Call
This compares A and B and puts lonely files into a folder named UNMATCHED.

DOS

compares.exe "C:\Photos\Set1" "C:\Photos\Set2"
2. Specifying an Output Folder
This compares A and B and puts lonely files into a folder you name (e.g., MissingFiles).

DOS

compares.exe "C:\Photos\Set1" "C:\Photos\Set2" "C:\Photos\MissingFiles"

