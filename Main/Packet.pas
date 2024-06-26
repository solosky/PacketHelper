unit Packet;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.StdCtrls, Vcl.Buttons, Vcl.Grids,
  dcrHexEditor, dcrHexEditorEx, Main, Vcl.Menus, Clipbrd, Data.Bind.EngExt,
  Vcl.Bind.DBEngExt, Data.Bind.Components;

type
  TPacketForm = class(TForm)
    Label1: TLabel;
    BitBtnOk: TBitBtn;
    BitBtnCancel: TBitBtn;
    BitBtnReset: TBitBtn;
    PopupMenuHexEditor: TPopupMenu;
    PopMenuSelectionCopyToClipboard: TMenuItem;
    PopMenuPasteFromClipboard: TMenuItem;
    PopMenuInsertFromClipboard: TMenuItem;
    MPHexEditor: TMPHexEditorEx;
    CheckBoxInsertMode: TCheckBox;
    procedure BitBtnResetClick(Sender: TObject);
    procedure MPHexEditorChange(Sender: TObject);
    procedure BitBtnOkClick(Sender: TObject);
    procedure SetPacketData(Data:Pointer; Size:Integer);
    procedure FormCreate(Sender: TObject);
    procedure PopMenuSelectionCopyToClipboardClick(Sender: TObject);
    procedure BitBtnCancelClick(Sender: TObject);
    procedure PopMenuPasteFromClipboardClick(Sender: TObject);

    procedure PasteFromClipboard(IsInsert:Boolean);
    procedure PopMenuInsertFromClipboardClick(Sender: TObject);
    procedure CheckBoxInsertModeClick(Sender: TObject);
  private
    Updating: Boolean;
    { Private declarations }
  public
    { Public declarations }
  end;

var
  PacketForm: TPacketForm;
  LastSendPid: Integer;
  HasSendPacket: Boolean;

implementation

{$R *.dfm}

procedure TPacketForm.BitBtnCancelClick(Sender: TObject);
begin
  Hide;
end;

procedure TPacketForm.BitBtnOkClick(Sender: TObject);
var
  Msg: Pointer;
  PB: ^Byte;
  i: Integer;
  Ret : Integer;
begin
   if MPHexEditor.DataSize<6 then
      showMessage('要发送的封包大小至少是6个字节！')
   else
      begin
        Msg := allocmem(MPHexEditor.DataSize);
        PB  := Msg;
        for i := 0 to MPHexEditor.DataSize-1 do
        begin
           PB^ :=  MPHexEditor.Data[i];
           Inc(PB);
        end;
          LastSendPid := Main.CapturePid;
          HasSendPacket:= true;
          Ret := Main.SendPacket(Msg);
          if Ret=RET_OK then
          begin
              Main.MainForm.StatusBar.Panels[0].Text := '封包发送成功！';
              Hide;
          end
           else
            showMessage('封包发送失败！！');
      end;
end;

procedure TPacketForm.BitBtnResetClick(Sender: TObject);
var
i: Integer;
begin
  MPHexEditor.DataSize := 6;
   for i := 0 to 5 do
      MPHexEditor.SetMemory(i, Chr(0));
end;

procedure TPacketForm.CheckBoxInsertModeClick(Sender: TObject);
begin
    MPHexEditor.InsertMode := CheckBoxInsertMode.Checked;
end;

procedure TPacketForm.FormCreate(Sender: TObject);
begin
    Updating := false;
    LastSendPid := 0;
    HasSendPacket := false;
    BitBtnResetClick(Sender);
end;

procedure TPacketForm.MPHexEditorChange(Sender: TObject);
var
  Length: Integer;
begin
  if (MPHexEditor.DataSize>6) and not Updating then
     begin
         Length := MPHexEditor.Data[2]+  (MPHexEditor.Data[3] Shl 8) +
             (MPHexEditor.Data[4] Shl 16);
         if Length<>MPHexEditor.DataSize-6 then
         begin
             Length := MPHexEditor.DataSize-6;
             Updating := true;
             MPHexEditor.SetMemory(2, Chr(Length));
             MPHexEditor.SetMemory(3, Chr(Length Shr 8));
             MPHexEditor.SetMemory(4, Chr(Length Shr 16));
             Updating := false;
         end;

     end;
end;

procedure TPacketForm.PopMenuInsertFromClipboardClick(Sender: TObject);
begin
   PasteFromClipboard(true);
end;

procedure TPacketForm.PopMenuPasteFromClipboardClick(Sender: TObject);
begin
  PasteFromClipboard(false);
end;

procedure TPacketForm.PasteFromClipboard(IsInsert:Boolean);
var
  Tmp:Integer;
  Str: PWideChar;
  Part: string;
  i: Integer;
  Len: Integer;
  Bytes: array of Byte;
begin
  Str:= allocMem(1024);
  Clipboard.GetTextBuf(Str, 1024);
  if (Length(Str)>0) and ((Length(Str) mod 2)=0) then
    begin
      Len  := Length(Str) div 2;
      SetLength(Bytes, Len);
     for i:=0 to Len-1 do
        begin
        Part := copy(Str, i*2+1, 2);
         if TryStrToInt('0x'+Part, Tmp) and (Tmp >= 0) and (Tmp<=$FF) then
              Bytes[i] := Tmp
         else
         begin
           SetLength(Bytes, 0);
           Exit;
         end;
        end;

        Len := Length(Bytes);
        if IsInsert then
        begin
           MPHexEditor.DataSize := MPHexEditor.DataSize + Len;
           for i := MPHexEditor.DataSize-Len-1 downto MPHexEditor.GetCursorPos do
            MPHexEditor.SetMemory(i+Len,Chr( MPHexEditor.Data[i]) );
        end
        else
        begin
          if MPHexEditor.DataSize<MPHexEditor.GetCursorPos+Len+1 then
            MPHexEditor.DataSize := MPHexEditor.GetCursorPos+Len+1;
        end;

        for i := 0 to Len-1 do
          MPHexEditor.SetMemory(MPHexEditor.GetCursorPos+i, Chr(Bytes[i]));
        MPHexEditor.Invalidate;
    end;
    FreeMem(Str);
end;

procedure TPacketForm.PopMenuSelectionCopyToClipboardClick(Sender: TObject);
var
Str: string;
I: Integer;
StartIdx: Integer;
EndIdx: Integer;
begin
    if MPHexEditor.SelCount>0 then
    begin
    if MPHexEditor.SelStart<MPHexEditor.SelEnd then
    begin
       StartIdx := MPHexEditor.SelStart;
       EndIdx   := MPHexEditor.SelEnd;
    end
    else
    begin
       StartIdx := MPHexEditor.SelEnd;
       EndIdx   := MPHexEditor.SelStart;
    end;

    for I := StartIdx to EndIdx do
      Str := Str + Format('%.2X', [MPHexEditor.Data[I]]);

     Clipboard.SetTextBuf(PChar(Str));
     Main.MainForm.StatusBar.Panels[0].Text := '已复制 '+IntToStr(MPHexEditor.SelCount)+' 字节到剪切板。';
    end;
end;

procedure TPacketForm.SetPacketData(Data:Pointer; Size:Integer);
var
  b: ^Byte;
  i: Integer;
begin
  Updating := true;
   MPHexEditor.DataSize := Size;
   b := Data;
   for i := 0 to Size-1 do
   begin
     MPHexEditor.SetMemory(i, Chr(b^));
     Inc(b);
   end;
   Updating := false;
end;

end.
