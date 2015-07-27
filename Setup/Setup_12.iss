#define MyAppName		    "SLAP"
#define MyAppURL		    "http://www.cherubicsoft.com/projects/slap"
#define MyAppExe		    "SLAP.exe"
#define MyAppSource		  "..\Win32\Release\" + MyAppExe
#define MyAppId			    GetStringFileInfo( MyAppSource, INTERNAL_NAME )
#define MyAppVersion	  GetFileProductVersion( MyAppSource )
#define MyAppCopyright	GetFileCopyright( MyAppSource )
#define MyAppPublisher	GetFileCompany( MyAppSource )
#define MyOutput		    LowerCase( StringChange( MyAppName + " " + MyAppVersion, " ", "_" ) )
#define MyLocks			    "*" + MyAppExe

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
OutputDir=Release
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
#ifdef WIN64
  ; http://download.microsoft.com/download/9/3/F/93FCF1E7-E6A4-478B-96E7-D4B285925B00/vc_redist.x64.exe
	Source: "vc_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall; AfterInstall: ExecTemp( 'vc_redist.x64.exe', '/quiet' );
#else
  ; http://download.microsoft.com/download/9/3/F/93FCF1E7-E6A4-478B-96E7-D4B285925B00/vc_redist.x86.exe
	Source: "vc_redist.x86.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall; AfterInstall: ExecTemp( 'vc_redist.x86.exe', '/quiet' );
#endif

Source: "IssProc.dll"; DestDir: "{tmp}"; Flags: dontcopy deleteafterinstall
Source: "IssProc.dll"; DestDir: "{app}"
Source: "..\WinSparkle\WinSparkle.dll"; DestDir: "{app}"; Flags: replacesameversion uninsrestartdelete

Source: "{#MyAppSource}"; DestDir: "{app}"; Flags: replacesameversion uninsrestartdelete
Source: "..\ReadMe.md"; DestName: "ReadMe.txt"; DestDir: "{app}"; Flags: replacesameversion uninsrestartdelete
Source: "..\LICENSE"; DestName: "License.txt"; DestDir: "{app}"; Flags: replacesameversion uninsrestartdelete

[Icons]
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExe}"; Tasks: desktopicon
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExe}"
Name: "{group}\ReadMe.txt"; Filename: "{app}\ReadMe.txt"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}";	Filename: "{uninstallexe}"

[Run]
Filename: "{app}\{#MyAppExe}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall runasoriginaluser

[Registry]
Root: HKCU; Subkey: "Software\{#MyAppPublisher}\{#MyAppName}"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\{#MyAppPublisher}"; Flags: uninsdeletekeyifempty

[UninstallDelete]
Name: "{userstartup}\{#MyAppName}.lnk"; Type: files
Name: "{commonstartup}\{#MyAppName}.lnk"; Type: files
Name: "{app}"; Type: dirifempty
Name: "{pf}\{#MyAppPublisher}"; Type: dirifempty
Name: "{localappdata}\{#MyAppPublisher}\{#MyAppName}"; Type: filesandordirs
Name: "{localappdata}\{#MyAppPublisher}"; Type: dirifempty

[Code]
function IssFindModule(hWnd: Integer; Modulename: String; Language: PAnsiChar; Silent: Boolean; CanIgnore: Boolean ): Integer;
external 'IssFindModuleW@files:IssProc.dll stdcall setuponly';

function IssFindModuleU(hWnd: Integer; Modulename: String; Language: PAnsiChar; Silent: Boolean; CanIgnore: Boolean ): Integer;
external 'IssFindModuleW@{app}\IssProc.dll stdcall uninstallonly';

function IssEnableAnyFileInUseCheck(Folder: String): Integer;
external 'IssEnableAnyFileInUseCheckW@files:IssProc.dll stdcall setuponly';

function IssEnableAnyFileInUseCheckU(Folder: String): Integer;
external 'IssEnableAnyFileInUseCheckW@{app}\IssProc.dll stdcall uninstallonly';

function NextButtonClick(CurPageID: integer): Boolean;
var
	nCode: Integer;
begin
	Result := True;
	if CurPageID = wpWelcome then begin
		Result := False;
		nCode := IssFindModule( StrToInt( ExpandConstant( '{wizardhwnd}' ) ), '{#MyLocks}', 'en', False, True );
		if ( nCode = 1 ) then begin
			PostMessage( WizardForm.Handle, $0010, 0, 0 );
		end else if ( nCode = 0 ) or ( nCode = 2 ) then begin
			Result := True;
		end;
	end;

end;

function InitializeUninstall(): Boolean;
var
	nCode: Integer;
begin
	Result := False;
	nCode := IssFindModuleU( 0, '{#MyLocks}', 'en', False, False );
	if ( nCode = 0 ) or ( nCode = 2 ) then begin
		Result := True;
	end;
	UnloadDLL( ExpandConstant( '{app}\IssProc.dll' ) );
end;

procedure ExecTemp(File, Params : String);
var
	nCode: Integer;
begin
	Exec( ExpandConstant( '{tmp}' ) + '\' + File, Params, ExpandConstant( '{tmp}' ), SW_SHOW, ewWaitUntilTerminated, nCode );
end;
