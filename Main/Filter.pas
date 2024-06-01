unit Filter;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.StdCtrls, IWVCLBaseControl,
  IWBaseControl, IWBaseHTMLControl, IWControl, IWCompCheckbox, Vcl.Buttons;

type
  TFilterForm = class(TForm)
    CheckBoxApplyFilter: TCheckBox;
    GroupBoxAction: TGroupBox;
    CheckBoxActionSend: TCheckBox;
    CheckBoxActionRecv: TCheckBox;
    CheckBoxActionConnect: TCheckBox;
    CheckBoxActionClose: TCheckBox;
    GroupBoxPacket: TGroupBox;
    EditFilterCmd: TEdit;
    Label1: TLabel;
    Label2: TLabel;
    EditFilterLength: TEdit;
    ButtonApplyFilter: TBitBtn;
    ButtonOk: TBitBtn;
    ButtonCancel: TBitBtn;
    GroupBox1: TGroupBox;
    EditFilterData: TEdit;
    procedure CheckBoxApplyFilterClick(Sender: TObject);
    procedure UpdateFilterToUI(Sender: TObject);
    function UpdateUIToFilter(Sender: TObject):Boolean;
    procedure FormShow(Sender: TObject);
    procedure ButtonOkClick(Sender: TObject);
    procedure ButtonApplyFilterClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  FilterForm: TFilterForm;

implementation

{$R *.dfm}

uses   Main;

procedure TFilterForm.ButtonApplyFilterClick(Sender: TObject);
begin
   if UpdateUIToFilter(Sender) then
      Main.MainForm.UpdateAllMsgByFilter;
end;

procedure TFilterForm.ButtonOkClick(Sender: TObject);
begin
  if UpdateUIToFilter(Sender) then
    begin
      Main.MainForm.UpdateAllMsgByFilter;
      CloseModal;
    end;
end;

procedure TFilterForm.CheckBoxApplyFilterClick(Sender: TObject);
var
Enabled: Boolean;
begin
  Enabled := CheckBoxApplyFilter.Checked;
  CheckBoxActionSend.Enabled := Enabled;
  CheckBoxActionRecv.Enabled := Enabled;
  CheckBoxActionConnect.Enabled := Enabled;
  CheckBoxActionClose.Enabled := Enabled;
  EditFilterCmd.Enabled := Enabled;
  EditFilterLength.Enabled := Enabled;
  EditFilterData.Enabled := Enabled;
end;

procedure TFilterForm.FormShow(Sender: TObject);
begin
   UpdateFilterToUI(Sender);
end;

procedure TFilterForm.UpdateFilterToUI(Sender: TObject);
var
  PB: ^Byte;
  B0: Byte;
  B1: Byte;
  i: Integer;
  Str: string;
begin
   CheckBoxApplyFilter.Checked := Main.MsgFilter.Enabled;
   CheckBoxApplyFilterClick(Sender);

  CheckBoxActionSend.Checked := Main.MsgFilter.FilterSend;
  CheckBoxActionRecv.Checked := Main.MsgFilter.FilterRecv;
  CheckBoxActionConnect.Checked := Main.MsgFilter.FilterConnect;
  CheckBoxActionClose.Checked := Main.MsgFilter.FilterClose;

  if Main.MsgFilter.FilterCmd<>0 then
  begin
    PB := @(Main.MsgFilter.FilterCmd);
    B0 := PB^; Inc(PB);
    B1 := PB^;
    EditFilterCmd.Text := Format('%.2X%.2X', [B0, B1]);
  end
  else
     EditFilterCmd.Text := '';
  if Main.MsgFilter.FilterLength<>-1 then
    EditFilterLength.Text := Format('%X',[Main.MsgFilter.FilterLength])
  else
    EditFilterLength.Text := '';


    if Length(Main.MsgFilter.FilterData)>0 then
      begin
       for i := 0 to Length(Main.MsgFilter.FilterData)-1 do
          Str := Str + Format('%.2X ',[Main.MsgFilter.FilterData[i]]);
      EditFilterData.Text := Str
      end
      else
      EditFilterData.Text := '';
end;

function TFilterForm.UpdateUIToFilter(Sender: TObject):Boolean;
var
  Tmp:Integer;
  PB: ^Byte;
  B0: Byte;
  B1: Byte;
  Hexs: TStringList;
  Str: string;
  i: Integer;
begin

  UpdateUIToFilter := true;
   Main.MsgFilter.Enabled := CheckBoxApplyFilter.Checked;
   Main.MsgFilter.FilterSend :=CheckBoxActionSend.Checked;
   Main.MsgFilter.FilterRecv :=CheckBoxActionRecv.Checked;
   Main.MsgFilter.FilterConnect :=CheckBoxActionConnect.Checked;
   Main.MsgFilter.FilterClose :=CheckBoxActionClose.Checked;

   if Length(EditFilterCmd.Text)>0 then
      begin
       if TryStrToInt('0x'+EditFilterCmd.Text, Tmp) and (Tmp>0) and (Tmp<$FFFF) then
       begin
          PB := @Tmp;
          B0 := PB^; Inc(PB);
          B1 := PB^;
          Main.MsgFilter.FilterCmd := B1 + (B0 shl 8);
       end
       else
       begin
         showMessage('过滤的命令号不正确，请重新输入。 '+#13+#10+'过滤的序号应该为2个字节16进制，如D24E。');
         UpdateUIToFilter := false;
         Exit;
       end;
      end
      else
        Main.MsgFilter.FilterCmd := 0;



  if Length(EditFilterLength.Text)>0 then
    if TryStrToInt('0x'+EditFilterLength.Text, Tmp) and (Tmp > 0) and
        (Tmp < $FFFFFF) then
        Main.MsgFilter.FilterLength := Tmp
      else
      begin
        showMessage('过滤的封包长度不正确，请重新输入。' + #13 + #10 + '过滤的长度应该为封包数据的长度，16进制，如4A。');
        UpdateUIToFilter := false;
        Exit;
      end
    else
     Main.MsgFilter.FilterLength := -1;

    if Length(EditFilterData.Text)>0 then
    begin
      Hexs := TStringList.Create;
      Str := EditFilterData.Text;
      ExtractStrings([' '], [' '], PWideChar(Str), Hexs);
      SetLength(Main.MsgFilter.FilterData, Hexs.Count);
     i := 0;
     for Str in Hexs do
        begin
         if TryStrToInt('0x'+Str, Tmp) and (Tmp > 0) and (Tmp<$FF) then
         begin
              Main.MsgFilter.FilterData[i] := Tmp;
              i := i+1;
         end
         else
         begin
           SetLength(Main.MsgFilter.FilterData, 0);
           showMessage('过滤的封包数据格式不正确，请重新输入。'+#13 + #10+
            '输入需要过滤显示的数据，每个字节之间用空格隔开，每个字节用16进制表示，如 7E 3A');
           break;
         end;
        end;
    end

     else
      SetLength(Main.MsgFilter.FilterData, 0);

    Main.MainForm.ToolButtonApplyFilter.Down := Main.MsgFilter.Enabled;
    Main.MainForm.SubMenuApplyFilter.Checked := Main.MsgFilter.Enabled;
end;

end.
