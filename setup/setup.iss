#define Name          "RRS"
#define Version       "1.3.0"
#define arch          "x86_64"
#define Publisher     "maisvendoo"

#define RRS_DEV_ROOT      GetEnv("RRS_DEV_ROOT")      
#define BinDir        RRS_DEV_ROOT + "\bin"
#define LibDir        RRS_DEV_ROOT + "\lib"
#define ModulesDir    RRS_DEV_ROOT + "\modules"
#define PluginsDir    RRS_DEV_ROOT + "\plugins"
#define CfgDir        RRS_DEV_ROOT + "\cfg"
#define DataDir       RRS_DEV_ROOT + "\data"
#define RoutesDir     RRS_DEV_ROOT + "\routes"
#define LogsDir       RRS_DEV_ROOT + "\logs"
#define SrcDir        "..\"
#define ResourceDir   "..\resources"
#define FontsDir      RRS_DEV_ROOT + "\fonts"
#define DocsDir       RRS_DEV_ROOT + "\docs"
#define SdkDir        RRS_DEV_ROOT + "\sdk"
#define ThemesDir     RRS_DEV_ROOT + "\themes"
#define ExeName       "launcher.exe" 

[Setup]
AppId={{5BA4EB09-C456-4F39-BFA0-64020141EDCC}}

AppName={#Name}
AppVersion={#Version}
AppPublisher={#Publisher}

DefaultDirName={sd}\{#Name}
DefaultGroupName={#Name}
DisableDirPage=no

OutputDir=..\..\bin-setup
OutputBaseFilename={#Name}-{#arch}-v{#Version}-setup

SetupIconFile={#BinDir}\RRS_logo.ico

Compression=lzma
SolidCompression=yes

WizardImageFile="images\wizard.bmp"
WizardSmallImageFile="images\small-wizard.bmp"
WizardImageAlphaFormat=none

ChangesEnvironment=yes

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Dirs]
Name: "{app}\logs"
Name: "{app}\screenshots"

[Files]

Source: "{#BinDir}\*.*"; DestDir: "{app}\bin"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#BinDir}\RRS_logo.ico"; DestDir: "{app}\bin"; Flags: ignoreversion recursesubdirs createallsubdirsSource: "{#LibDir}\*.*"; DestDir: "{app}\lib"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#ModulesDir}\*.*"; DestDir: "{app}\modules"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#PluginsDir}\*.*"; DestDir: "{app}\plugins"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#CfgDir}\*.*"; DestDir: "{app}\cfg"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#DataDir}\*.*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#RoutesDir}\*.*"; DestDir: "{app}\routes"; Flags: ignoreversion recursesubdirs createallsubdirs  
Source: "{#SdkDir}\*.*"; DestDir: "{app}\sdk"; Flags: ignoreversion recursesubdirs createallsubdirs 
Source: "{#ThemesDir}\*.*"; DestDir: "{app}\themes"; Flags: ignoreversion recursesubdirs createallsubdirs
//Source: "{#FontsDir}\*.*"; DestDir: "{app}\fonts"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#DocsDir}\*.*"; DestDir: "{app}\docs"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#RRS_DEV_ROOT}\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#RRS_DEV_ROOT}\LICENSE-Russian"; DestDir: "{app}"; Flags: ignoreversion


[Icons]
Name: "{group}\{#Name}"; Filename: "{app}\bin\{#ExeName}"; IconFilename: "{app}\bin\RRS_logo.ico"                            
Name: "{commondesktop}\{#Name}"; Filename: "{app}\bin\{#ExeName}"; IconFilename: "{app}\bin\RRS_logo.ico"; Tasks: desktopicon

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"; LicenseFile: "{#RRS_DEV_ROOT}\LICENSE"
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl"; LicenseFile: "{#RRS_DEV_ROOT}\LICENSE-Russian"
Name: "ua"; MessagesFile: "compiler:Languages\Ukrainian.isl"; LicenseFile: "{#RRS_DEV_ROOT}\LICENSE-Russian"

[Registry]
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType:string; ValueName: "RRS_ROOT"; ValueData: "{app}"