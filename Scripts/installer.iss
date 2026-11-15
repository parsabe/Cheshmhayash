; Cheshm Hayash - Windows installer (Inno Setup 6)
; Build it with Scripts\make_installer.bat after Scripts\package.bat.
; Output: Packaged\CheshmHayash-Setup.exe
;
; Installs per user into %LOCALAPPDATA%\Programs\Cheshm Hayash (no admin rights needed, and the
; game can write its saves and settings next to itself). Installs the Microsoft Visual C++ runtime
; only if it is missing (that one step asks for admin rights).

#define AppName "Cheshm Hayash"
#define AppNameFa "چشم‌هایش"
#define AppVersion "1.0"
#define AppPublisher "Parsa Besharat"
#define AppURL "https://parsabe.com"
#define GameDir "..\Packaged\Windows"
#define Redist "C:\Program Files\Epic Games\UE_5.8\Engine\Extras\Redist\en-us\vc_redist.x64.exe"

[Setup]
AppId={{6C2F4E7A-1401-4F0D-9A1E-4B5E5A0C1401}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
DefaultDirName={localappdata}\Programs\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
OutputDir=..\Packaged
OutputBaseFilename=CheshmHayash-Setup
SetupIconFile={#GameDir}\Application.ico
UninstallDisplayIcon={app}\Cheshmhayash.exe
UninstallDisplayName={#AppName}
Compression=lzma2/normal
SolidCompression=no
WizardStyle=modern
ShowLanguageDialog=auto

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "de"; MessagesFile: "compiler:Languages\German.isl"

[CustomMessages]
en.DesktopIcon=Create a desktop shortcut
de.DesktopIcon=Eine Verknüpfung auf dem Desktop erstellen
en.Runtime=Installing the Microsoft Visual C++ runtime...
de.Runtime=Microsoft Visual C++ Runtime wird installiert...
en.Play=Play {#AppName}
de.Play={#AppName} spielen

[Tasks]
Name: "desktopicon"; Description: "{cm:DesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "{#GameDir}\*"; DestDir: "{app}"; Excludes: "Manifest_*.txt,\Cheshmhayash\Saved\*"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#Redist}"; DestDir: "{tmp}"; Flags: deleteafterinstall; Check: NeedsVCRuntime

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\Cheshmhayash.exe"; WorkingDir: "{app}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\Cheshmhayash.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/install /quiet /norestart"; StatusMsg: "{cm:Runtime}"; Flags: shellexec waituntilterminated runascurrentuser; Verb: "runas"; Check: NeedsVCRuntime
Filename: "{app}\Cheshmhayash.exe"; Description: "{cm:Play}"; WorkingDir: "{app}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; the game writes its saves, settings and logs here
Type: filesandordirs; Name: "{app}\Cheshmhayash\Saved"

[Code]
function NeedsVCRuntime: Boolean;
var
  Installed: Cardinal;
begin
  Result := True;
  if RegQueryDWordValue(HKLM64, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Installed', Installed) then
    Result := Installed <> 1;
end;
