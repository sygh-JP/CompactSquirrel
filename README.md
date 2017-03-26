# CompactSquirrel
Compact Squirrel by sygh.

This is a library forked from Squirrel in order to use it in Windows Store Apps (WinRT-based applications).

## Development Environment (開発環境)
* Microsoft Visual Studio 2012 Update 5
* Squirrel 3.1
* Sqrat 0.8.92

Note that the current latest version of Sqrat (0.9.2) has some changes of specification and has some Unicode-related issues around calls of sq_pushstring in "sqratUtil.h".

## Target Environment (ターゲット環境)
* Windows Vista/Windows 7/Windows 8.1/Windows 10 (Desktop)
* Windows 8.1/Windows 10 (WinRT)

## How to Build (ビルド方法)
1. Download Squirrel 3.1 and copy "include", "sqstdlib" and "squirrel" folders to "squirrel3"
1. Download Sqrat 0.8.92 and copy "include" folder to "sqrat"
1. Build Squirrel by "squirrel3/_sln_vs2012/squirrel_vs2012.sln" or "squirrel3/_sln_vs2012_win8/squirrel_vs2012_win8.sln"
1. Build test projects by "sqrat/MySqratTest1/MySqBindersTests.sln" or "StoreAppSquirrelTest1/StoreAppSquirrelTest1.sln"

2017-03-26, sygh.
