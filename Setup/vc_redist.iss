[CustomMessages]
vcredist_title_x86=Microsoft Visual C++ 2015 Redistributable (x86)
vcredist_title_x64=Microsoft Visual C++ 2015 Redistributable (x64)
vcredist_url_x86=http://download.microsoft.com/download/9/3/F/93FCF1E7-E6A4-478B-96E7-D4B285925B00/vc_redist.x86.exe
vcredist_url_x64=http://download.microsoft.com/download/9/3/F/93FCF1E7-E6A4-478B-96E7-D4B285925B00/vc_redist.x64.exe
vcredist_productcode_x86={A2563E55-3BEC-3828-8D67-E5E8B9E8B675}
vcredist_productcode_x64={0D3E9E15-DE7A-300B-96F1-B4AF12B96488}

en.depdownload_msg=The following applications are required before setup can continue:%n%n%1%nDownload and install now?
en.depdownload_memo_title=Download dependencies
en.depinstall_memo_title=Install dependencies
en.depinstall_title=Installing dependencies
en.depinstall_description=Please wait while Setup installs dependencies on your computer.
en.depinstall_status=Installing %1...
en.depinstall_missing=%1 must be installed before setup can continue. Please install %1 and run Setup again.
en.depinstall_error=An error occured while installing the dependencies. Please restart the computer and run the setup again or install the following dependencies manually:%n
en.isxdl_langfile=

ru.depdownload_msg=Следующая программа должна быть предустановлена перед продолжением установки:%n%n%1%nЗагрузить и установить её сейчас?
ru.depdownload_memo_title=Загрузить программы
ru.depinstall_memo_title=Установить программы
ru.depinstall_title=Установка программы
ru.depinstall_description=Пожалуйста подождите пока происходит установка программы на ваш компьютер.
ru.depinstall_status=Установка %1...
ru.depinstall_missing=Программа %1 должна быть предустановлена пред продолжением установки. Пожалуйста установите %1 и запустите эту установку снова.
ru.depinstall_error=Произошла ошибка при установке программы. Пожалуйста перезагрузите компьютер и запустите эту установку снова, либо установите следующие программы вручную:%n
ru.isxdl_langfile=russian.ini

[Files]
Source: "isxdl\isxdl.dll";   DestDir: "{tmp}"; Flags: dontcopy deleteafterinstall
Source: "isxdl\russian.ini"; DestDir: "{tmp}"; Flags: dontcopy deleteafterinstall

[Code]
#IFDEF UNICODE
    #DEFINE AW "W"
#ELSE
    #DEFINE AW "A"
#ENDIF

type
    INSTALLSTATE = Longint;
const
    INSTALLSTATE_INVALIDARG = -2;  // An invalid parameter was passed to the function.
    INSTALLSTATE_UNKNOWN = -1;     // The product is neither advertised or installed.
    INSTALLSTATE_ADVERTISED = 1;   // The product is advertised but not installed.
    INSTALLSTATE_ABSENT = 2;       // The product is installed for a different user.
    INSTALLSTATE_DEFAULT = 5;      // The product is installed for the current user.

function MsiQueryProductState(szProduct: string): INSTALLSTATE;
external 'MsiQueryProductState{#AW}@msi.dll stdcall';

function msiproduct(const ProductID: string): boolean;
begin
    Result := MsiQueryProductState(ProductID) = INSTALLSTATE_DEFAULT;
end;

procedure isxdl_AddFile(URL, Filename: PAnsiChar);
external 'isxdl_AddFile@files:isxdl.dll stdcall';

function isxdl_DownloadFiles(hWnd: Integer): Integer;
external 'isxdl_DownloadFiles@files:isxdl.dll stdcall';

function isxdl_SetOption(Option, Value: PAnsiChar): Integer;
external 'isxdl_SetOption@files:isxdl.dll stdcall';

type
	TProduct = record
		File: String;
		Title: String;
		Parameters: String;
		InstallClean : boolean;
		MustRebootAfter : boolean;
	end;

	InstallResult = (InstallSuccessful, InstallRebootRequired, InstallError);

var
	installMemo, downloadMemo, downloadMessage: string;
	products: array of TProduct;
	delayedReboot: boolean;
	DependencyPage: TOutputProgressWizardPage;

procedure AddProduct(FileName, Parameters, Title, URL: string; InstallClean : boolean; MustRebootAfter : boolean);
var
	path: string;
	i: Integer;
begin
	installMemo := installMemo + '%1' + Title + #13;

  path := ExpandConstant('{tmp}{\}') + FileName;

  isxdl_AddFile(URL, path);

  downloadMemo := downloadMemo + '%1' + Title + #13;
  downloadMessage := downloadMessage + '	' + Title + #13;

	i := GetArrayLength(products);
	SetArrayLength(products, i + 1);
	products[i].File := path;
	products[i].Title := Title;
	products[i].Parameters := Parameters;
	products[i].InstallClean := InstallClean;
	products[i].MustRebootAfter := MustRebootAfter;
end;

function SmartExec(prod : TProduct; var ResultCode : Integer) : Boolean;
begin
	if (LowerCase(Copy(prod.File,Length(prod.File)-2,3)) = 'exe') then begin
		Result := Exec(prod.File, prod.Parameters, '', SW_SHOWNORMAL, ewWaitUntilTerminated, ResultCode);
	end else begin
		Result := ShellExec('', prod.File, prod.Parameters, '', SW_SHOWNORMAL, ewWaitUntilTerminated, ResultCode);
	end;
end;

function PendingReboot : Boolean;
var	names: String;
begin
	if (RegQueryMultiStringValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Session Manager', 'PendingFileRenameOperations', names)) then begin
		Result := true;
	end else if ((RegQueryMultiStringValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Session Manager', 'SetupExecute', names)) and (names <> ''))  then begin
		Result := true;
	end else begin
		Result := false;
	end;
end;

function InstallProducts: InstallResult;
var
	ResultCode, i, productCount, finishCount: Integer;
begin
	Result := InstallSuccessful;
	productCount := GetArrayLength(products);

	if productCount > 0 then begin
		DependencyPage := CreateOutputProgressPage(CustomMessage('depinstall_title'), CustomMessage('depinstall_description'));
		DependencyPage.Show;

		for i := 0 to productCount - 1 do begin
			if (products[i].InstallClean and (delayedReboot or PendingReboot())) then begin
				Result := InstallRebootRequired;
				break;
			end;

			DependencyPage.SetText(FmtMessage(CustomMessage('depinstall_status'), [products[i].Title]), '');
			DependencyPage.SetProgress(i, productCount);

			if SmartExec(products[i], ResultCode) then begin
				//setup executed; ResultCode contains the exit code
				if (products[i].MustRebootAfter) then begin
					//delay reboot after install if we installed the last dependency anyways
					if (i = productCount - 1) then begin
						delayedReboot := true;
					end else begin
						Result := InstallRebootRequired;
					end;
					break;
				end else if (ResultCode = 0) then begin
					finishCount := finishCount + 1;
				end else if (ResultCode = 3010) then begin
					//ResultCode 3010: A restart is required to complete the installation. This message indicates success.
					delayedReboot := true;
					finishCount := finishCount + 1;
				end else begin
					Result := InstallError;
					break;
				end;
			end else begin
				Result := InstallError;
				break;
			end;
		end;

		//only leave not installed products for error message
		for i := 0 to productCount - finishCount - 1 do begin
			products[i] := products[i+finishCount];
		end;
		SetArrayLength(products, productCount - finishCount);

		DependencyPage.Hide;
	end;
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
var
	i: Integer;
	s: string;
begin
	delayedReboot := false;

	case InstallProducts() of
		InstallError: begin
			s := CustomMessage('depinstall_error');
			for i := 0 to GetArrayLength(products) - 1 do begin
				s := s + #13 + '	' + products[i].Title;
			end;
			Result := s;
			end;

		InstallRebootRequired: begin
			Result := products[0].Title;
			NeedsRestart := true;
			//write into the registry that the installer needs to be executed again after restart
			RegWriteStringValue(HKEY_CURRENT_USER, 'SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce', 'InstallBootstrap', ExpandConstant('{srcexe}'));
			end;
	end;
end;

function NeedRestart : Boolean;
begin
	if (delayedReboot) then
		Result := true;
end;

function UpdateReadyMemo(Space, NewLine, MemoUserInfoInfo, MemoDirInfo, MemoTypeInfo, MemoComponentsInfo, MemoGroupInfo, MemoTasksInfo: String): String;
var
	s: string;
begin
	if downloadMemo <> '' then
		s := s + CustomMessage('depdownload_memo_title') + ':' + NewLine + FmtMessage(downloadMemo, [Space]) + NewLine;
	if installMemo <> '' then
		s := s + CustomMessage('depinstall_memo_title') + ':' + NewLine + FmtMessage(installMemo, [Space]) + NewLine;
	s := s + MemoDirInfo + NewLine + NewLine + MemoGroupInfo
	if MemoTasksInfo <> '' then
		s := s + NewLine + NewLine + MemoTasksInfo;
	Result := s
end;

function vcNextButtonClick(): Boolean;
begin
	Result := true;
  if downloadMemo <> '' then begin
    if (ActiveLanguage() <> 'en') then begin
      ExtractTemporaryFile(CustomMessage('isxdl_langfile'));
      isxdl_SetOption('language', ExpandConstant('{tmp}{\}') + CustomMessage('isxdl_langfile'));
    end;
    if SuppressibleMsgBox(FmtMessage(CustomMessage('depdownload_msg'), [downloadMessage]), mbConfirmation, MB_YESNO, IDYES) = IDNO then
      Result := false
    else if isxdl_DownloadFiles(StrToInt(ExpandConstant('{wizardhwnd}'))) = 0 then
      Result := false;
  end;
end;

function IsX64(): Boolean;
begin
	Result := Is64BitInstallMode and (ProcessorArchitecture = paX64);
end;

function GetArchitectureString(): String;
begin
	if IsX64() then begin
		Result := 'x64';
	end else begin
		Result := 'x86';
	end;
end;

function vcInitializeSetup(): Boolean;
begin
  Result := true;
  if ( not msiproduct( CustomMessage( 'vcredist_productcode_' + GetArchitectureString() ) ) ) then
    AddProduct( 'vc_redist.' + GetArchitectureString() + '.exe',
      '/passive /norestart',
      CustomMessage( 'vcredist_title_' + GetArchitectureString() ),
      CustomMessage( 'vcredist_url_' + GetArchitectureString() ),
      false, false );
end;
