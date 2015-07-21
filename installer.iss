[Setup]
AppName=Zinc IDE
AppVersion=0.1
DefaultDirName={pf}\Zinc IDE
DefaultGroupName=Opturion
Compression=lzma2
SolidCompression=yes

[Files]
Source: "package\*"; DestDir: "{app}"; Flags: recursesubdirs

[Icons]
Name: "{group}\Zinc IDE"; Filename: "{app}\ZincIDE.exe"; IconFilename: "{app}\icon.ico"
