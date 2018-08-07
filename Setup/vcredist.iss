#define vcredist_name			      "Microsoft Visual C++ 2017 Redistributable 14.14.26429.4"
#define vcredist_file			      "mfc141u.dll"

#define vcredist32_exe          "VC_redist.x86.exe"
#define vcredist32_title        vcredist_name + " (x86)"
#define vcredist32_productcode  "{7753EC39-3039-3629-98BE-447C5D869C09}"
#define vcredist32_url          "https://aka.ms/vs/15/release/VC_redist.x86.exe"

#define vcredist64_exe          "VC_redist.x64.exe"
#define vcredist64_title        vcredist_name + " (x64)"
#define vcredist64_productcode  "{03EBF679-E886-38AD-8E70-28658449F7F9}"
#define vcredist64_url          "https://aka.ms/vs/15/release/VC_redist.x64.exe"

[Code]
function InstallVCRedist(): Boolean;
begin
  Result := False;

  #if Platform == "Win32"
    if not MsiProduct( '{#vcredist32_productcode}' ) and not FileExists( ExpandConstant( '{syswow64}\{#vcredist_file}' ) ) then begin
      AddProduct( '{#vcredist32_exe}', '/quiet /norestart', '{#vcredist32_title}', '{#vcredist32_url}', false, false, true );
      Result := True;
    end;
  #endif
  #if Platform == "x64"
    if IsWin64 and not MsiProduct( '{#vcredist64_productcode}' ) and not FileExists( ExpandConstant( '{sys}\{#vcredist_file}' ) ) then begin
      AddProduct( '{#vcredist64_exe}', '/quiet /norestart', '{#vcredist64_title}', '{#vcredist64_url}', false, false, true );
      Result := True;
    end;
  #endif

end;
