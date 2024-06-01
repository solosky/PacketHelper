program PacketHelper;

//{$APPTYPE   CONSOLE}

uses
  Vcl.Forms,
  Main in 'Main.pas' {MainForm},
  dcrHexEditor in 'lib\dcrHexEditor.pas',
  dcrHexEditorEx in 'lib\dcrHexEditorEx.pas',
  Process in 'Process.pas' {ProcessForm},
  About in 'About.pas' {AboutForm},
  Packet in 'Packet.pas' {PacketForm},
  Filter in 'Filter.pas' {FilterForm},
  Tools in 'Tools.pas' {ToolsForm};

{$R *.res}

begin
  Application.Initialize;
  Application.MainFormOnTaskbar := True;
  Application.CreateForm(TMainForm, MainForm);
  Application.CreateForm(TPacketForm, PacketForm);
  Application.CreateForm(TFilterForm, FilterForm);
  Application.CreateForm(TProcessForm, ProcessForm);
  Application.CreateForm(TToolsForm, ToolsForm);
  Application.Run;
end.
