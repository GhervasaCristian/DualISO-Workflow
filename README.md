DualISO Magic Lantern Scripted Workflow.

There are multiple tools created in the help of making our life easier.

------------------------
DNG Unpack & Repack Tool
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
-----------------------------