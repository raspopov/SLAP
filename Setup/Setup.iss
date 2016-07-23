#define MyAppName		    "SLAP"
#define MyAppURL		    "http://www.cherubicsoft.com/projects/slap"
#define MyAppExe		    "SLAP.exe"
#define MyAppSource		  "..\Release\" + MyAppExe
#define MyAppId			    GetStringFileInfo( MyAppSource, INTERNAL_NAME )
#define MyAppVersion	  GetFileProductVersion( MyAppSource )
#define MyAppCopyright	GetFileCopyright( MyAppSource )
#define MyAppPublisher	GetFileCompany( MyAppSource )
#define MyOutput		    LowerCase( StringChange( MyAppName + " " + MyAppVersion, " ", "_" ) )

#ifndef WIN64
  #define vcredist_exe          "vc_redist.x86.exe"
  #define vcredist_title        "Microsoft Visual C++ 2015 Redistributable (x86)"
  #define vcredist_url          "http://download.microsoft.com/download/9/3/F/93FCF1E7-E6A4-478B-96E7-D4B285925B00/" + vcredist_exe
  #define vcredist_productcode  "{A2563E55-3BEC-3828-8D67-E5E8B9E8B675}"
#else
  #define vcredist_exe          "vc_redist.x64.exe"
  #define vcredist_title        "Microsoft Visual C++ 2015 Redistributable (x64)"
  #define vcredist_productcode  "{0D3E9E15-DE7A-300B-96F1-B4AF12B96488}"
  #define vcredist_url          "http://download.microsoft.com/download/9/3/F/93FCF1E7-E6A4-478B-96E7-D4B285925B00/" + vcredist_exe
#endif

#include "idp\lang\russian.iss"
#include "idp\idp.iss"

#include "dep\lang\russian.iss"
#include "dep\dep.iss"

[Setup]
AppId={#MyAppId}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
VersionInfoVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
AppMutex=Global\{#MyAppId}
AppCopyright={#MyAppCopyright}
DefaultDirName={pf}\{#MyAppPublisher}\{#MyAppId}
DefaultGroupName={#MyAppName}
OutputDir=..\Release
OutputBaseFilename={#MyOutput}
Compression=lzma2/ultra64
SolidCompression=yes
InternalCompressLevel=ultra64
LZMAUseSeparateProcess=yes
PrivilegesRequired=admin
ChangesAssociations=yes
UninstallDisplayIcon={app}\{#MyAppExe},0
DirExistsWarning=no
WizardImageFile=compiler:WizModernImage-IS.bmp
WizardSmallImageFile=compiler:WizModernSmallImage-IS.bmp
SetupIconFile=Setup.ico
SetupMutex=Global\Setup_{#MyAppId}
OutputManifestFile=Setup-Manifest.txt

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "..\Release\WinSparkle.dll"; DestDir: "{app}"; Flags: replacesameversion uninsrestartdelete

Source: "{#MyAppSource}"; DestDir: "{app}"; Flags: replacesameversion uninsrestartdelete
Source: "..\ReadMe.md"; DestName: "ReadMe.txt"; DestDir: "{app}"; Flags: replacesameversion uninsrestartdelete
Source: "..\LICENSE"; DestName: "License.txt"; DestDir: "{app}"; Flags: replacesameversion uninsrestartdelete

[Icons]
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExe}"; Tasks: desktopicon
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExe}"
Name: "{group}\ReadMe.txt"; Filename: "{app}\ReadMe.txt"
Name: "{group}\License.txt"; Filename: "{app}\License.txt"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}";	Filename: "{uninstallexe}"

[Run]
Filename: "{app}\{#MyAppExe}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall runasoriginaluser

[Registry]
Root: HKCU; Subkey: "Software\{#MyAppPublisher}\{#MyAppName}"; Flags: dontcreatekey uninsdeletekey
Root: HKCU; Subkey: "Software\{#MyAppPublisher}"; Flags: dontcreatekey uninsdeletekeyifempty

[UninstallDelete]
Name: "{userstartup}\{#MyAppName}.lnk"; Type: files
Name: "{commonstartup}\{#MyAppName}.lnk"; Type: files
Name: "{app}"; Type: dirifempty
Name: "{pf}\{#MyAppPublisher}"; Type: dirifempty
Name: "{localappdata}\{#MyAppPublisher}\{#MyAppName}"; Type: filesandordirs
Name: "{localappdata}\{#MyAppPublisher}"; Type: dirifempty

[Code]
procedure InitializeWizard();
var
  bWork: Boolean;
begin
  bWork := False;

  if ( not MsiProduct( '{#vcredist_productcode}' ) ) then begin
    AddProduct( '{#vcredist_exe}', '/quiet /norestart', '{#vcredist_title}', '{#vcredist_url}', false, false );
    bWork := True;
  end;

  if bWork then begin
    idpDownloadAfter( wpReady );
    idpSetDetailedMode( True );
  end;
end;
