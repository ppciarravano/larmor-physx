;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Larmor-Physx Version 1.1.0 2013
; Copyright (c) 2013 Pier Paolo Ciarravano - http://www.larmor.com
; All rights reserved.
;
; This file is part of LarmorVoronoi Maya Plugin (http://code.google.com/p/larmor-physx/).
;
; LarmorVoronoi Maya Plugin is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; LarmorVoronoi Maya Plugin is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
; 
; You should have received a copy of the GNU General Public License
; along with LarmorVoronoi Maya Plugin. If not, see <http://www.gnu.org/licenses/>.
; 
; Licensees holding a valid commercial license may use this file in
; accordance with the commercial license agreement provided with the
; software.
; 
; Author: Pier Paolo Ciarravano
; $Id$
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; LarmorVoronoiMayaPlugin Nullsoft installer script (see http://nsis.sourceforge.net)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

!include "MUI2.nsh"
;http://nsis.sourceforge.net/Environmental_Variables:_append,_prepend,_and_remove_entries
!include "EnvVarUpdate.nsh" 

!define VERSION "1.1.0Beta-Build77"
!define SHORT_NAME "LarmorVoronoi Plugin for Maya"
!define MED_NAME "LarmorVoronoiMayaPlugin"
!define FULL_NAME "LarmorVoronoi Plugin for Maya 2012/2013/2014 64bit"
!define RELEASE_DATE "26/10/13"

Name "${SHORT_NAME}"

Var MAYA_VERSION_NAME
Var MODULE_FILE_LOCATION
Var MAYA_USER_LOCATION


; stop scripts that need the 8192 NSIS_MAX_STRLEN special build from compiling.
; http://nsis.sourceforge.net/SetReqStrLen:_Allow_compile_w/_8192_special_build_only
!macro SetReqStrLen Req_STRLEN
	!define "Check_${NSIS_MAX_STRLEN}"
	!ifndef "Check_${Req_STRLEN}"
		!error "You're not using the ${Req_STRLEN} string length special build! \
			${NSIS_MAX_STRLEN} is no good!"
	!else
		!undef "Check_${NSIS_MAX_STRLEN}"
		!undef "SetReqStrLen"
	!endif
!macroend
!define SetReqStrLen "!insertmacro SetReqStrLen"
${SetReqStrLen} 8192


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
		FileWrite $0 " $INSTDIR\$MAYA_VERSION_NAME$\n"
		FileClose $0

		SetShellVarContext all
		;SetRegView 64
		WriteRegStr HKLM "SOFTWARE\Larmor.com\${MED_NAME}" "$MAYA_VERSION_NAME" "$MODULE_FILE_LOCATION\LarmorVoronoi_MayaPlugin.txt"
		
		; copy plug-in files
		CreateDirectory "$INSTDIR\$MAYA_VERSION_NAME"
		CreateDirectory "$INSTDIR\$MAYA_VERSION_NAME\plug-ins"
		CreateDirectory "$INSTDIR\$MAYA_VERSION_NAME\scripts"
		
		${Switch} "$MAYA_VERSION_NAME"
			${Case} "2012-x64"
				SetOutPath "$INSTDIR\$MAYA_VERSION_NAME\plug-ins"
				File "2012-x64\plug-ins\*.*"
				SetOutPath "$INSTDIR\$MAYA_VERSION_NAME\scripts"
				File "2012-x64\scripts\*.*"
			${Break}
			${Case} "2013-x64"
				SetOutPath "$INSTDIR\$MAYA_VERSION_NAME\plug-ins"
				File "2013-x64\plug-ins\*.*"
				SetOutPath "$INSTDIR\$MAYA_VERSION_NAME\scripts"
				File "2013-x64\scripts\*.*"
			${Break}
			${Case} "2014-x64"
				SetOutPath "$INSTDIR\$MAYA_VERSION_NAME\plug-ins"
				File "2014-x64\plug-ins\*.*"
				SetOutPath "$INSTDIR\$MAYA_VERSION_NAME\scripts"
				File "2014-x64\scripts\*.*"
			${Break}
			${Default}
			${Break}
		${EndSwitch}
 
FunctionEnd


Function un.removeModule

	ClearErrors
	ReadRegStr $MODULE_FILE_LOCATION HKLM "SOFTWARE\Larmor.com\${MED_NAME}" "$MAYA_VERSION_NAME"
	IfErrors 0 ModuleFound
	Return

	ModuleFound:
		; remove the module
		Delete "$MODULE_FILE_LOCATION"
		
		; remove files
		Delete "$INSTDIR\$MAYA_VERSION_NAME\plug-ins\*.*"
		Delete "$INSTDIR\$MAYA_VERSION_NAME\scripts\*.*"
		Delete "$INSTDIR\$MAYA_VERSION_NAME\*.*"
		RMDir "$INSTDIR\$MAYA_VERSION_NAME\plug-ins"
		RMDir "$INSTDIR\$MAYA_VERSION_NAME\scripts"
		RMDir "$INSTDIR\$MAYA_VERSION_NAME"
	 
FunctionEnd


;;;;;;;;;;;;;;;;;;;;;;;;;
; Sections
;;;;;;;;;;;;;;;;;;;;;;;;;

InstType "Maya 2012 64bit"
InstType "Maya 2013 64bit"
InstType "Maya 2014 64bit"

Section "!Common libs" score
	SectionIn RO
	AddSize 1
	SectionIn 1 2 3

	SetShellVarContext all

	CreateDirectory "$INSTDIR\doc"
	CreateDirectory "$INSTDIR\shared"

	SetOutPath "$INSTDIR\doc"
	File "doc\*.*"

	SetOutPath "$INSTDIR\shared"
	File "dll\*.*"

	SetOutPath "$INSTDIR"
	File "License_gpl-3.0.txt"

	; write the installation path into the registry
	;SetRegView 64
	WriteRegStr HKLM "SOFTWARE\Larmor.com\${MED_NAME}" "INSTALLDIR" "$INSTDIR"
	WriteRegStr HKLM "SOFTWARE\Larmor.com\${MED_NAME}" "VERSION" "${VERSION}"

	; add plugin dlls path to env variable
	${EnvVarUpdate} $0 "PATH" "A" "HKLM" "$INSTDIR\shared"  

	; install shortcuts
	CreateDirectory "$SMPROGRAMS\${SHORT_NAME}\"
	CreateShortCut "$SMPROGRAMS\${SHORT_NAME}\Documentation.lnk" "$INSTDIR\doc\LarmorVoronoiMayaPlugin.html"
	CreateShortCut "$SMPROGRAMS\${SHORT_NAME}\Online project documentation.lnk" "http://www.larmor.com/LarmorVoronoiMayaPlugin/"
	CreateShortCut "$SMPROGRAMS\${SHORT_NAME}\Open Larmor-Physx project page.lnk" "http://code.google.com/p/larmor-physx/"
	CreateShortCut "$SMPROGRAMS\${SHORT_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe"

	; write the uninstall keys & uninstaller for Windows
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MED_NAME}" "DisplayName" "${FULL_NAME} (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MED_NAME}" "UninstallString" "$INSTDIR\uninstall.exe"

	; write unistaller
	SetOutPath $INSTDIR
	WriteUninstaller "uninstall.exe"

	;MessageBox MB_OK "INSTDIR: $INSTDIR" 

SectionEnd


Section /o "Maya 2012 64bit" s2012
	AddSize 3200
	SectionIn 1

	StrCpy $MAYA_VERSION_NAME "2012-x64"
	Call installModule
SectionEnd


Section /o "Maya 2013 64bit" s2013
	AddSize 3200
	SectionIn 2

	StrCpy $MAYA_VERSION_NAME "2013-x64"
	Call installModule
SectionEnd


Section "Maya 2014 64bit" s2014
	AddSize 3200
	SectionIn 3

	StrCpy $MAYA_VERSION_NAME "2014-x64"
	Call installModule
SectionEnd


!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${score} "LarmorVoronoi core common DLLs"
	!insertmacro MUI_DESCRIPTION_TEXT ${s2012} "LarmorVoronoi Plugin for Maya 2012 64bit (stable Beta version)"
	!insertmacro MUI_DESCRIPTION_TEXT ${s2013} "LarmorVoronoi Plugin for Maya 2013 64bit (stable Beta version)"
	!insertmacro MUI_DESCRIPTION_TEXT ${s2014} "LarmorVoronoi Plugin for Maya 2014 64bit (stable Beta version)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END


Section "Uninstall"
	SetShellVarContext all
	SetAutoClose false

	; recover install dir
	;SetRegView 64
	ReadRegStr $INSTDIR HKLM "SOFTWARE\Larmor.com\${MED_NAME}" "INSTALLDIR"
	;MessageBox MB_OK "INSTDIR: $INSTDIR" 

	; remove plugin dlls path to env variable
	${un.EnvVarUpdate} $0 "PATH" "R" "HKLM" "$INSTDIR\shared"  

	; remove install directory
	Delete "$INSTDIR\doc\*.*"
	Delete "$INSTDIR\shared\*.*"
	Delete "$INSTDIR\*.*"
	RMDir "$INSTDIR\doc"
	RMDir "$INSTDIR\shared"
	
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
  
	RMDir "$INSTDIR"
	
SectionEnd
