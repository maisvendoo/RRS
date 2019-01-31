#define Name        "RussianRailwaySimulator"
#define Version       "0.0.1"
#define arch          "x86_64"
#define Publisher     "maisvendoo"
#define BinDir        "..\..\bin-win\bin"
#define LibDir        "..\..\bin-win\lib"
#define ModulesDir    "..\..\bin-win\modules"
#define PluginsDir    "..\..\bin-win\plugins"
#define CfgDir        "..\..\bin-win\cfg"
#define DataDir       "..\..\bin-win\data"
#define RoutesDir     "..\..\bin-win\routes"
#define LogsDir       "..\..\bin-win\logs"
#define SrcDir        "..\"
#define ResourceDir   "..\resources"
#define FontsDir      "..\..\fonts"
#define ExeName       "launcher.exe" 

[Setup]
AppId={{78497FC5-5BDD-485A-A0EC-DF0E340651F7}}

AppName={#Name}
AppVersion={#Version}
AppPublisher={#Publisher}

DefaultDirName={pf}\{#Name}
DefaultGroupName={#Name}

OutputDir=..\..\bin-setup
OutputBaseFilename={#Name}-{#arch}-v{#Version}-setup

SetupIconFile={#BinDir}\logo.ico

Compression=lzma
SolidCompression=yes

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]

Source: "{#BinDir}\*.*"; DestDir: "{app}\bin"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#BinDir}\logo.ico"; DestDir: "{app}\bin"; Flags: ignoreversion recursesubdirs createallsubdirsSource: "{#LibDir}\*.*"; DestDir: "{app}\lib"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#ModulesDir}\*.*"; DestDir: "{app}\modules"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#PluginsDir}\*.*"; DestDir: "{app}\plugins"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#CfgDir}\*.*"; DestDir: "{app}\cfg"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#DataDir}\*.*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#RoutesDir}\*.*"; DestDir: "{app}\routes"; Flags: ignoreversion recursesubdirs createallsubdirs  
Source: "{#LogsDir}\*.*"; DestDir: "{app}\logs"; Flags: ignoreversion recursesubdirs createallsubdirs  

[Icons]
Name: "{group}\{#Name}"; Filename: "{app}\bin\{#ExeName}"; IconFilename: "{app}\bin\logo.ico"                            
Name: "{commondesktop}\{#Name}"; Filename: "{app}\bin\{#ExeName}"; IconFilename: "{app}\bin\logo.ico"; Tasks: desktopicon