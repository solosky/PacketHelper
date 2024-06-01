unit Process;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.StdCtrls, Vcl.ComCtrls, Vcl.ImgList, TLHelp32,
  Vcl.ExtCtrls;

type
  TProcessForm = class(TForm)
    ListViewProcess: TListView;
    EditKey: TEdit;
    ButtonOk: TButton;
    ButtonCancel: TButton;
    ImageListDlg: TImageList;
    Label1: TLabel;
    ButtonRefresh: TButton;
    RadioButtonHeadJmp: TRadioButton;
    RadioButtonPushRet: TRadioButton;
    RadioButtonEaxJmp: TRadioButton;
    RadioButtonAutoFixSeq: TRadioButton;
    RadioButtonNoFixSeq: TRadioButton;
    GroupBox1: TGroupBox;
    GroupBox2: TGroupBox;
    Label2: TLabel;
    procedure ButtonOkClick(Sender: TObject);
    procedure ButtonRefreshClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  ProcessForm: TProcessForm;

implementation

{$R *.dfm}

uses Main;

procedure TProcessForm.ButtonOkClick(Sender: TObject);
var
  HookType: Integer;
  SelectedPid: Integer;
  AutoFixSeq: Integer;
begin
  if ListViewProcess.SelCount=1 then
    begin
        SelectedPid := strToInt(ListViewProcess.Selected.Caption);
        if RadioButtonHeadJmp.Checked then
           HookType := 0
        else if RadioButtonPushRet.Checked then
           HookType := 1
        else if RadioButtonEaxJmp.Checked then
           HookType := 2
        else
            HookType := 0;

        if RadioButtonAutoFixSeq.Checked then
            AutoFixSeq := 1
        else
            AutoFixSeq := 0;

      if Main.isCapturing then
       begin
         if Main.MainForm.CheckCloseProcess then
         begin
            StopCapture(Main.CapturePid);
            Main.CapturePid := SelectedPid;
            Main.CaptureExe := ListViewProcess.Selected.SubItems[0];
           StartCapture(SelectedPid, HookType, AutoFixSeq);
         end;
      end
      else
        begin
           Main.CapturePid := SelectedPid;
           Main.CaptureExe := ListViewProcess.Selected.SubItems[0];
           StartCapture(SelectedPid, HookType, AutoFixSeq);
        end;

        Self.ModalResult := mrOk;
        Self.CloseModal;
     end;

end;

procedure TProcessForm.ButtonRefreshClick(Sender: TObject);
var
    OK:Bool;
    subItem: TListItem;
    ProcessListHandle:THandle;
    ProcessStruct:TProcessEntry32;
begin
    ListViewProcess.Clear;
    ProcessListHandle:=CreateToolHelp32SnapShot(TH32CS_SNAPPROCESS,0);
    ProcessStruct.dwSize:=SizeOf(ProcessStruct);
    OK:=Process32First(ProcessListHandle,ProcessStruct);
    while   Integer(OK) <> 0   do
        begin
          if length(EditKey.Text)>0 then
            begin
              if pos(UpperCase(EditKey.Text), UpperCase(ProcessStruct.szExeFile))<>0 then
                begin
                  subItem := ListViewProcess.Items.Add;
                  subItem.Caption := IntToStr(ProcessStruct.th32ProcessID);
                  subItem.subItems.Add(ProcessStruct.szExeFile);
                end;
            end
          else
            begin
              subItem := ListViewProcess.Items.Add;
              subItem.Caption := IntToStr(ProcessStruct.th32ProcessID);
              subItem.subItems.Add(ProcessStruct.szExeFile);
            end;

          OK:=Process32Next(ProcessListHandle,ProcessStruct);
        end;
    closehandle(ProcessListHandle);
end;

end.
