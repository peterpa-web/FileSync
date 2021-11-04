!include "MUI2.nsh"

; FileSyncSetup.nsi
;
; It will install the applications into a directory that the user selects,

;--------------------------------

; The name of the installer
Name "FileSync"
!define MYAPP FileSync
!define MYAPPNAME "FileSync"
;SetShellVarContext all

!define WINSYS32 $%windir%\system32
!define /date CRDATE "%Y%m%d"

; The file to write
OutFile "FileSyncSetup${CRDATE}.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\Peter Pagel\${MYAPP}"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\${MYAPP}" "Install_Dir"

;Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------

; The stuff to install
Section "Application" Section1

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "..\Release\FileSync.exe"
  File "..\FileSync\License.txt"
  File "..\FileSync\gpl.txt"
  File /r "..\FileSync\html"
  File "${WINSYS32}\mfc100.dll"
  File "${WINSYS32}\mfc100u.dll"
  File "${WINSYS32}\msvcr100.dll"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\${MYAPP} "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MYAPP}" "DisplayName" "${MYAPPNAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MYAPP}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MYAPP}" "DisplayVersion" "${CRDATE}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MYAPP}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MYAPP}" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts" Section2

;    CreateDirectory "$SMPROGRAMS\${MYAPP}"
;    CreateShortCut "$SMPROGRAMS\${MYAPP}\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
    CreateShortCut "$SMPROGRAMS\${MYAPP}.lnk" "$INSTDIR\${MYAPP}.exe" "" "$INSTDIR\${MYAPP}.exe" 0
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Sources" Section3

  CreateDirectory "$INSTDIR\sources"
  SetOutPath $INSTDIR\sources
  File "..\*.sln"
  CreateDirectory "$INSTDIR\sources\FileSync"
  SetOutPath $INSTDIR\sources\FileSync
  File "..\FileSync\*.h"
  File "..\FileSync\*.c*"
  File "..\FileSync\*.rc"
  File "..\FileSync\*.vcx*"
  CreateDirectory "$INSTDIR\sources\FileSync\res"
  SetOutPath $INSTDIR\sources\FileSync\res
  File "..\FileSync\res\*.*"
  CreateDirectory "$INSTDIR\sources\CDVD"
  SetOutPath $INSTDIR\sources\CDVD
  File "..\CDVD\*.h"
  File "..\CDVD\*.c*"
  File "..\CDVD\*.vcx*"
  CreateDirectory "$INSTDIR\sources\CDVD\IsoFS"
  SetOutPath $INSTDIR\sources\CDVD\IsoFS
  File "..\CDVD\IsoFS\*.h"
  File "..\CDVD\IsoFS\*.c*"
  
SectionEnd

;--------------------------------
;Descriptions
  LangString DESC_Section1 ${LANG_ENGLISH} "Application installation files."
  LangString DESC_Section2 ${LANG_ENGLISH} "Optional start menu shortcut."
  LangString DESC_Section3 ${LANG_ENGLISH} "Optional application sources."

  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${Section1} $(DESC_Section1)
    !insertmacro MUI_DESCRIPTION_TEXT ${Section2} $(DESC_Section2)
    !insertmacro MUI_DESCRIPTION_TEXT ${Section3} $(DESC_Section3)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END


;--------------------------------

; Uninstaller

Section "Uninstall" UnSection1
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MYAPP}"
  DeleteRegKey HKLM SOFTWARE\${MYAPP}
  DeleteRegKey HKCU "Software\Peter Pagel\${MYAPP}"

  ; Remove files and uninstaller
  Delete "$INSTDIR\FileSync.exe"
  RMDir /r /REBOOTOK "$INSTDIR\html"
  RMDir /r /REBOOTOK "$INSTDIR\sources"
  Delete "$INSTDIR\License.txt"
  Delete "$INSTDIR\gpl.txt"
  Delete "$INSTDIR\mfc100.dll"
  Delete "$INSTDIR\mfc100u.dll"
  Delete "$INSTDIR\msvcr100.dll"
  Delete "$INSTDIR\uninstall.exe"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\${MYAPP}.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\${MYAPP}"
  RMDir "$INSTDIR"

SectionEnd
