#define MyAppName "MicPlus"
#define MyAppVersion "1.5"
#define MyAppPublisher "Nivemoon Labs"
#define MyAppExeName "MicPlus.exe"

[Setup]
AppId={{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
AppName={#MyAppName}
AppPublisher={#MyAppPublisher}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputDir=Output
OutputBaseFilename=MicPlus-Setup
SetupIconFile=assets\micplus.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
LanguageDetectionMethod=locale
UsePreviousLanguage=no

[Languages]
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Создать ярлык на рабочем столе"; GroupDescription: "Дополнительные ярлыки:"; Flags: unchecked
Name: "autostart"; Description: "Добавить в автозагрузку"; GroupDescription: "Дополнительные ярлыки:"; Flags: unchecked

[Files]
Source: "{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "micctl.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "assets\*"; DestDir: "{app}\assets"; Flags: ignoreversion recursesubdirs
Source: "sounds\*"; DestDir: "{app}\sounds"; Flags: ignoreversion recursesubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userstartup}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: autostart

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Запустить MicPlus"; Flags: nowait postinstall skipifsilent

[Code]
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  AppDataPath: String;
  MsgResult: Integer;
begin
  if CurUninstallStep = usUninstall then
  begin
    MsgResult := MsgBox('Удалить все файлы программы, включая настройки?' + #13#10 + #13#10 +
                        'Нажмите «Да», чтобы удалить всё.' + #13#10 +
                        'Нажмите «Нет», чтобы удалить только ярлыки и записи реестра.',
                        mbConfirmation, MB_YESNO or MB_DEFBUTTON2);
    
    if MsgResult = idYes then
    begin
      DelTree(ExpandConstant('{app}'), True, True, True);
      
      AppDataPath := ExpandConstant('{userappdata}') + '\MicPlus';
      DeleteFile(AppDataPath + '\micplus.ini');
      DelTree(AppDataPath, True, False, False);
    end;
  end;
end;
