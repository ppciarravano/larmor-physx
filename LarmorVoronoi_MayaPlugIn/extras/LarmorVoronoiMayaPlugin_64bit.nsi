;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Project name: LarmorVoronoi (Larmor-Physx)
; Mesh Voronoi shatter Maya Plug-in
; Version 1.0Beta (for Maya 2012/2013/2014)
; Released: 18/08/2013
; Author: Pier Paolo Ciarravano
; http://www.larmor.com
; 
; License: This project is released under the Qt Public License (QPL - OSI-Approved Open Source license).
; http://opensource.org/licenses/QPL-1.0
; 
; This software is provided 'as-is', without any express or implied warranty.
; In no event will the authors be held liable for any damages arising from the use of this software.
; 
; $Id$
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; LarmorVoronoiMayaPlugin Nullsoft installer script (see http://nsis.sourceforge.net)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

!include "MUI2.nsh"
;http://nsis.sourceforge.net/Environmental_Variables:_append,_prepend,_and_remove_entries
!include "EnvVarUpdate.nsh" 

!define VERSION "1.0Beta-Build72"
!define SHORT_NAME "LarmorVoronoi Plugin for Maya"
!define MED_NAME "LarmorVoronoiMayaPlugin"
!define FULL_NAME "LarmorVoronoi Plugin for Maya 2012/2013/2014 64bit"
!define RELEASE_DATE "25/08/2013"

Name "${SHORT_NAME}"

Var MAYA_VERSION_NAME
Var MODULE_FILE_LOCATION
Var MAYA_USER_LOCATION


;;;;;;;;;;;;;;;;;;;;;;;;;
; Pages
;;;;;;;;;;;;;;;;;;;;;;;;;

!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_NOAUTOCLOSE

!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_WELCOMEPAGE_TEXT  "This wizard will guide you through the installation of the ${FULL_NAME} version ${VERSION} relased on ${RELEASE_DATE}. Be sure that Maya is closed before you continue. Note to Win2k/XP users: Administrative privileges are required to install the software successfully.$\r$\n$\r$\nAuthor: Pier Paolo Ciarravano http://www.larmor.com"
!insertmacro MUI_PAGE_WELCOME

;!define MUI_LICENSEPAGE_TEXT_TOP "Text for top"
!define MUI_LICENSEPAGE_CHECKBOX
!define MUI_LICENSEPAGE_CHECKBOX_TEXT "I agree and also accept that the plugin auto-checks the updates availability online"
!insertmacro MUI_PAGE_LICENSE License_gpl-3.0.txt

!insertmacro MUI_PAGE_COMPONENTS

!define MUI_DIRECTORYPAGE_TEXT_TOP "The field below specifies the folder where ${FULL_NAME} version ${VERSION} will be installed. Unless you really know what you are doing, you should leave the field below as it is."
!define MUI_DIRECTORYPAGE_TEXT_DESTINATION "${SHORT_NAME} Location"
!define MUI_DIRECTORYPAGE_VARIABLE $INSTDIR
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\doc\LarmorVoronoiMayaPlugin.html"
!define MUI_FINISHPAGE_LINK "Visit us at http://code.google.com/p/larmor-physx/"
!define MUI_FINISHPAGE_LINK_LOCATION "http://code.google.com/p/larmor-physx/"
!insertmacro MUI_PAGE_FINISH

!define MUI_WELCOMEPAGE_TEXT  "This wizard will guide you through the uninstallation of the ${FULL_NAME} version ${VERSION}. Before starting the uninstallation, make sure Maya is not running. Click Next to continue."
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"
  
OutFile "${MED_NAME}-${VERSION}.exe"
InstallDir "$PROGRAMFILES64\LarmorVoronoiMayaPlugin\"
BrandingText "http://www.larmor.com/"
Icon favicon.ico
UninstallIcon favicon.ico
ShowInstDetails show
ShowUninstDetails show


;;;;;;;;;;;;;;;;;;;;;;;;;
; Functions
;;;;;;;;;;;;;;;;;;;;;;;;;

Var INSTVERSION
Function .onInit
	; check if the plugin is already installed

	ClearErrors
	;ReadRegStr $INSTDIR HKLM "SOFTWARE\Larmor.com\${MED_NAME}" "INSTALLDIR"
	ReadRegStr $INSTVERSION HKLM "SOFTWARE\Larmor.com\${MED_NAME}" "VERSION"
	IfErrors 0 PluginFound
	Return

	PluginFound:
		; the plugin is already installed
		MessageBox MB_OK "${SHORT_NAME} version $INSTVERSION is already installed. Make sure that the ${SHORT_NAME} is properly uninstalled and try again."
		Abort ; causes installer to quit     

FunctionEnd


Function installModule

	SetShellVarContext current

	StrCpy $MAYA_USER_LOCATION "$DOCUMENTS\maya\$MAYA_VERSION_NAME"
	IfFileExists "$MAYA_USER_LOCATION\*.*" DoInstallModule
	MessageBox MB_OK "The Maya $MAYA_VERSION_NAME user directory is not found: $MAYA_USER_LOCATION $\r$\nThe module for Maya $MAYA_VERSION_NAME will not be installed."
	Return

	DoInstallModule:
		StrCpy $MODULE_FILE_LOCATION "$DOCUMENTS\maya\$MAYA_VERSION_NAME\modules"

		;MessageBox MB_OK "MODULE_FILE_LOCATION: $MODULE_FILE_LOCATION"
		SetOutPath	$MODULE_FILE_LOCATION
		File "LarmorVoronoi_MayaPlugin.txt"

		FileOpen $0 "$MODULE_FILE_LOCATION\LarmorVoronoi_MayaPlugin.txt" a
		FileSeek $0 0 END
		FileWrite $0 " $INSTDIR$\n"
		FileClose $0

		SetShellVarContext all
		;SetRegView 64
		WriteRegStr HKLM "SOFTWARE\Larmor.com\${MED_NAME}" "$MAYA_VERSION_NAME" "$MODULE_FILE_LOCATION\LarmorVoronoi_MayaPlugin.txt"
 
FunctionEnd


Function un.removeModule

	ClearErrors
	ReadRegStr $MODULE_FILE_LOCATION HKLM "SOFTWARE\Larmor.com\${MED_NAME}" "$MAYA_VERSION_NAME"
	IfErrors 0 ModuleFound
	Return

	ModuleFound:
		; remove the module
		Delete "$MODULE_FILE_LOCATION"
	 
FunctionEnd


;;;;;;;;;;;;;;;;;;;;;;;;;
; Sections
;;;;;;;;;;;;;;;;;;;;;;;;;

InstType "Maya 2012 64bit"
InstType "Maya 2013 64bit"
InstType "Maya 2014 64bit"

Section "!LarmorVoronoi libs" score
	SectionIn RO
	AddSize 1
	SectionIn 1 2 3

	SetShellVarContext all

	CreateDirectory "$INSTDIR\doc"
	CreateDirectory "$INSTDIR\plug-ins"

	SetOutPath "$INSTDIR\doc"
	File "doc\*.*"

	SetOutPath "$INSTDIR\plug-ins"
	File "dll\*.*"

	SetOutPath "$INSTDIR"
	File "License_gpl-3.0.txt"

	; write the installation path into the registry
	;SetRegView 64
	WriteRegStr HKLM "SOFTWARE\Larmor.com\${MED_NAME}" "INSTALLDIR" "$INSTDIR"
	WriteRegStr HKLM "SOFTWARE\Larmor.com\${MED_NAME}" "VERSION" "${VERSION}"

	; add plugin dlls path to env variable
	${EnvVarUpdate} $0 "PATH" "A" "HKLM" "$INSTDIR\plug-ins"  

	; install shortcuts
	CreateDirectory "$SMPROGRAMS\${SHORT_NAME}\"
	CreateShortCut "$SMPROGRAMS\${SHORT_NAME}\Documentation.lnk" "$INSTDIR\doc\LarmorVoronoiMayaPlugin.html"
	CreateShortCut "$SMPROGRAMS\${SHORT_NAME}\Online project page.lnk" "http://code.google.com/p/larmor-physx/"
	CreateShortCut "$SMPROGRAMS\${SHORT_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe"

	; write the uninstall keys & uninstaller for Windows
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MED_NAME}" "DisplayName" "${FULL_NAME} (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MED_NAME}" "UninstallString" "$INSTDIR\uninstall.exe"

	; write unistaller
	SetOutPath $INSTDIR
	WriteUninstaller "uninstall.exe"

	;MessageBox MB_OK "INSTDIR: $INSTDIR" 

SectionEnd


Section "Maya 2012 64bit" s2012
	AddSize 1 
	SectionIn 1

	StrCpy $MAYA_VERSION_NAME "2012-x64"
	Call installModule
SectionEnd


Section /o "Maya 2013 64bit" s2013
	AddSize 1
	SectionIn 2

	StrCpy $MAYA_VERSION_NAME "2013-x64"
	Call installModule
SectionEnd


Section /o "Maya 2014 64bit" s2014
	AddSize 1
	SectionIn 3

	StrCpy $MAYA_VERSION_NAME "2014-x64"
	Call installModule
SectionEnd


!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${score} "LarmorVoronoi core common DLLs"
	!insertmacro MUI_DESCRIPTION_TEXT ${s2012} "LarmorVoronoi Plugin for Maya 2012 64bit (stable Beta version)"
	!insertmacro MUI_DESCRIPTION_TEXT ${s2013} "LarmorVoronoi Plugin for Maya 2013 64bit (Alfa version)"
	!insertmacro MUI_DESCRIPTION_TEXT ${s2014} "LarmorVoronoi Plugin for Maya 2014 64bit (Alfa version)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END


Section "Uninstall"
	SetShellVarContext all
	SetAutoClose false

	; recover install dir
	;SetRegView 64
	ReadRegStr $INSTDIR HKLM "SOFTWARE\Larmor.com\${MED_NAME}" "INSTALLDIR"
	;MessageBox MB_OK "INSTDIR: $INSTDIR" 



	; remove plugin dlls path to env variable
	${un.EnvVarUpdate} $0 "PATH" "R" "HKLM" "$INSTDIR\plug-ins"  

	; remove install directory
	Delete "$INSTDIR\doc\*.*"
	Delete "$INSTDIR\plug-ins\*.*"
	Delete "$INSTDIR\*.*"
	RMDir "$INSTDIR\doc"
	RMDir "$INSTDIR\plug-ins"
	RMDir "$INSTDIR"

	; remove links in start menu
	Delete "$SMPROGRAMS\${SHORT_NAME}\*.*"
	RMDir "$SMPROGRAMS\${SHORT_NAME}"

	; remove the maya plug-in modules
	StrCpy $MAYA_VERSION_NAME "2012-x64"
	Call un.removeModule
	StrCpy $MAYA_VERSION_NAME "2013-x64"
	Call un.removeModule
	StrCpy $MAYA_VERSION_NAME "2014-x64"
	Call un.removeModule

	; remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MED_NAME}"
	DeleteRegKey HKLM "SOFTWARE\Larmor.com\${MED_NAME}"
  
SectionEnd
