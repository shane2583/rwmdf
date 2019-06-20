MDF4 Lib Linux Examples

The MDFLib is a class library which compiles under Windows and Linux. The file mdfConfig.h
determines the settings.
// Linux Settings:
#define _MDF4_WONLY 0
#define _MDF4_DUMP  0
#define _MDF4_XML   0
#define _MDF4_UTF8  1
#define _MDF4_MD5   1
#define _MDF4_WIN   0
#define _MDF4_1     1

// Windows Settings:
#define _MDF4_WONLY 0
#define _MDF4_DUMP  0
#define _MDF4_XML   1
#define _MDF4_UTF8  1
#define _MDF4_MD5   1
#define _MDF4_WIN   1
#define _MDF4_1     1

The Lib is tested under Ubuntu using Netbeans as IDE. It should work with any other ID as well.

The contents of the archive is:
In "Common" all MDF4 lib files
In "MDF4_SimpleWriter" the files of a writer example.
In "MDF4_SimpleReader" the files of a reader example.

The layout is:
  
  project root (sln-file for VC2015)
  - Common
  - MDF4_SimpleWriter
  - MDF4_SimpleReader

In VC a project for the Write example can be created as follows:
 - Create an empty Win32 project
 - Add the .h and .cpp files from Common and MDF4_SimpleWriter (use mdfConfig.h from MDF4_SimpleWriter)
 - Compile

In Netbeans it is basically the same. In Eclipse it should be possible, but I could not figure out how to do it (no C++?).


