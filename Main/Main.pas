unit Main;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.Menus, Vcl.ComCtrls, Vcl.ToolWin,
  Vcl.ExtCtrls, Vcl.ImgList, Vcl.Grids, dcrHexEditor, dcrHexEditorEx,
  Vcl.StdCtrls,Clipbrd, Vcl.ExtDlgs, Data.Bind.EngExt, Vcl.Bind.DBEngExt,
  Data.Bind.Components, Filter, Tools;

  Const
    WM_EVENT = WM_USER+100;
    APP_NAME = '天龙封包助手 v1.2 [BY solosky]';

    ACTION_SEND = 0;
    ACTION_RECV = 1;
    ACTION_CONNECT = 2;
    ACTION_CLOSE = 3;

    RET_OK = 0;
    RET_ERR = 1;

    EXTRA_FIXED_SEQ = 1;
    EXTRA_USER_MSG  = 2;
    EXTRA_ERR_SEQ   = 4;

    DUMP_MAGIC = $19F8F7F6;

  //游戏数据包结构
(* #pragma pack(1)
typedef struct __TAG_MESSAGE
{
	WORD	uCmd;
	WORD	uLength;
	BYTE	cLengthEx;
	BYTE	cSequece;
	BYTE    pData[1];
} MESSAGE, *PMESSAGE;
#pragma pack() *)

type TMESSAGE = packed record
  Cmd:      Word;
  Length:   Word;
  LengthEx: Byte;
  Sequece:  Byte;
  Data:     array[1..1] of Byte;
end;
(*
//连接信息结构
typedef struct __TAG_CONNECTION
{
	DWORD		dwPid;
	DWORD		dwSocket;
	DWORD		dwAddr;
	USHORT		uPort;
	DWORD		dwAction;
} CONNECTION, *PCONNECTION;   *)

type TCONNECTION = record
  Pid:    Integer;
  Socket: Integer;
  Addr:   Integer;
  Port:   Word;
  Action: Integer;
  Extra:  Integer;
end;

(*
//同时保存连接和数据结构
typedef struct __TAG_MSG_ENTRY
{
	CONNECTION Connection;
	MESSAGE	Message;
} MSG_ENTRY, *PMSG_ENTRY;    *)

type TMSG_ENTRY = record
  Cnn: TCONNECTION;
  Msg:    TMESSAGE;
end;

type TEVENT_ARGS = record
  EventType:Integer;
  Param1: Integer;
  Param2: Integer;
end;

type TMSG_FILTER = record
  Enabled: Boolean;
  FilterCmd:  Word;
  FilterLength: Integer;
  FilterConnect: Boolean;
  FilterClose: Boolean;
  FilterSend: Boolean;
  FilterRecv: Boolean;
  FilterData: array of Byte;
end;

type TFnEventHandler =  procedure(EventType:Integer; Param1:Integer; Param2:Integer);
  cdecl;
procedure MyEventHandler(EventType:Integer; Param1:Integer; Param2:Integer);
  cdecl;

type
  TMainForm = class(TForm)
    MainMenu: TMainMenu;
    MenuCapture: TMenuItem;
    SubMenuOpenProcess: TMenuItem;
    SubMenuQuitApp: TMenuItem;
    ImageListMain: TImageList;
    StatusBar: TStatusBar;
    SplitterMain: TSplitter;
    ListViewMessages: TListView;
    C1: TMenuItem;
    SubMenuAutoScroll: TMenuItem;
    SubMenuWindowOnTop: TMenuItem;
    N7: TMenuItem;
    SubMenuAboutApp: TMenuItem;
    N9: TMenuItem;
    SubMenuClearGrid: TMenuItem;
    MPHexEditorData: TMPHexEditorEx;
    MainControlBar: TControlBar;
    ToolBar1: TToolBar;
    ToolButtonOpenProcess: TToolButton;
    ToolButtonToggleCapture: TToolButton;
    ToolButtonSplit1: TToolButton;
    ToolButtonClearGrid: TToolButton;
    ToolButtonSendPacket: TToolButton;
    ToolButtonSplit2: TToolButton;
    ToolButtonWindowOnTop: TToolButton;
    ToolButtonSplit3: TToolButton;
    ToolButtonAbout: TToolButton;
    ToolButtonSaveAllPackets: TToolButton;
    Panel1: TPanel;
    ToolButtonToggleHexEditor: TToolButton;
    ToolButtonCloseProcess: TToolButton;
    ToolButtonAutoScroll: TToolButton;
    SubMenuShowHexEditor: TMenuItem;
    SubMenuSendPacket: TMenuItem;
    PopupMenuMessages: TPopupMenu;
    PopMenuCopyToClipboard: TMenuItem;
    PopMenuSelectedMsgSaveToFile: TMenuItem;
    PopMenuCopyToSendDialog: TMenuItem;
    SaveTextFileDialog: TSaveTextFileDialog;
    E1: TMenuItem;
    SubMenuCopyToClipboard: TMenuItem;
    SubMenuSaveAllToFile: TMenuItem;
    SubMenuCopyToSendDialog: TMenuItem;
    SubMenuSelectedSaveToFile: TMenuItem;
    L1: TMenuItem;
    SubMenuApplyFilter: TMenuItem;
    SubMenuApplySetting: TMenuItem;
    ToolButton1: TToolButton;
    ToolButtonApplyFilter: TToolButton;
    ToolButtonFilterSetting: TToolButton;
    PopupMenuHexEditor: TPopupMenu;
    PopMenuHexEditorCopyToClipboard: TMenuItem;
    N1: TMenuItem;
    SubMenuSaveDumpFile: TMenuItem;
    SubMenuOpenDumpFile: TMenuItem;
    DumpFileOpenDialog: TOpenDialog;
    DumpFileSaveDialog: TSaveDialog;
    PopMenuHexEditorCopyToClipboardWithSpace: TMenuItem;
    ToolButton2: TToolButton;
    ToolButton3: TToolButton;

    procedure SubMenuOpenProcessClick(Sender: TObject);
    procedure SubMenuAboutAppClick(Sender: TObject);
    procedure ListViewMessagesSelectItem(Sender: TObject; Item: TListItem;
      Selected: Boolean);
    procedure FormCreate(Sender: TObject);
    procedure OnEvent(var   msg:TMSG); message   WM_EVENT;

    //处理抓包事件的接口
    procedure OnEventGameMsg(Param1:Integer; Param2:Integer);
    procedure OnEventConnNew(Param1:Integer; Param2:Integer);
    procedure OnEventConnClosed(Param1:Integer; Param2:Integer);
    procedure ToolButtonClearGridClick(Sender: TObject);
    procedure ToolButtonToggleCaptureClick(Sender: TObject);
    procedure ToolButtonCloseProcessClick(Sender: TObject);
    procedure ToolButtonWindowOnTopClick(Sender: TObject);
    procedure ToolButtonToggleHexEditorClick(Sender: TObject);
    procedure SubMenuAutoScrollClick(Sender: TObject);
    procedure SubMenuShowHexEditorClick(Sender: TObject);
    procedure SubMenuWindowOnTopClick(Sender: TObject);
    procedure ToolButtonAutoScrollClick(Sender: TObject);
    procedure ToolButtonSendPacketClick(Sender: TObject);
    procedure PopMenuCopyToSendDialogClick(Sender: TObject);
    procedure PopMenuCopyToClipboardClick(Sender: TObject);

    function GetPacketDataString(All:Boolean): string;
    function IsShouldShowMsg(MsgPtr:Pointer): Boolean;
    function CheckCloseProcess(): Boolean;
    function FormatByteSize(Size: Integer): string;

    procedure ToolButtonSaveAllPacketsClick(Sender: TObject);
    procedure SubMenuSelectedSaveToFileClick(Sender: TObject);
    procedure ToolButtonFilterSettingClick(Sender: TObject);
    procedure FormCloseQuery(Sender: TObject; var CanClose: Boolean);
    procedure ToolButtonApplyFilterClick(Sender: TObject);
    procedure SubMenuApplyFilterClick(Sender: TObject);

    procedure UpdateAllMsgByFilter();
    procedure InsertOneMsgToListView(MsgPtr:Pointer);
    procedure SubMenuQuitAppClick(Sender: TObject);
    procedure PopMenuHexEditorCopyToClipboardClick(Sender: TObject);
    procedure MPHexEditorDataSelectionChanged(Sender: TObject);
    procedure ListViewMessagesCustomDrawItem(Sender: TCustomListView;
      Item: TListItem; State: TCustomDrawState; var DefaultDraw: Boolean);
    procedure SubMenuSaveDumpFileClick(Sender: TObject);
    procedure SubMenuOpenDumpFileClick(Sender: TObject);
    procedure PopMenuHexEditorCopyToClipboardWithSpaceClick(Sender: TObject);
    procedure ToolButton3Click(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);

  private
    { Private declarations }
  public
    { Public declarations }
  end;





//VOID MY_API SetEventHandler(FnEventHandler pfEventHandler);
//DWORD MY_API StartCapture(DWORD dwPid);
//DWORD MY_API StopCapture(DWORD dwPid);
procedure SetEventHandler(Handler:TFnEventHandler);
  cdecl; external 'Receiver.dll';
function StartCapture(Pid: Integer; HookType: Integer; FixSeq: Integer): Integer;
  cdecl; external 'Receiver.dll';
function StopCapture(Pid: Integer): Integer;
  cdecl; external 'Receiver.dll';
function SendPacket(Msg: Pointer): Integer;
  cdecl; external 'Receiver.dll';

var
  MainForm: TMainForm;
  MsgList: TList;
  MsgSize: Integer;
  IsCapturing: Boolean;
  IsPausing: Boolean;
  CapturePid: Integer;
  CaptureExe: string;
  ListViewHeight: Integer;
  MsgFilter: TMSG_FILTER;

implementation

{$R *.dfm}

uses Process, About, Packet, StrUtils;

procedure MyEventHandler(EventType:Integer; Param1:Integer; Param2:Integer); cdecl;
var
  EventArg: TEVENT_ARGS;
begin
  //New(EventArg);
  EventArg.EventType := EventType;
  EventArg.Param1 := Param1;
  EventArg.Param2 := Param2;

  //奇怪， WParam在消息处理函数里面会被替换成Message变量。。
  SendMessage(MainForm.Handle, WM_EVENT, 0, Integer(@EventArg));
end;


procedure TMainForm.FormClose(Sender: TObject; var Action: TCloseAction);
begin
    Application.Terminate;
end;

procedure TMainForm.FormCloseQuery(Sender: TObject; var CanClose: Boolean);
begin
  if CheckCloseProcess then
     CanClose := true
     else
     CanClose := false;
end;

procedure TMainForm.FormCreate(Sender: TObject);
begin
  MsgList := TList.Create;
  MsgFilter.Enabled := false;
  MsgFilter.FilterCmd := 0;
  MsgFilter.FilterLength := -1;
  MsgFilter.FilterConnect := true;
  MsgFilter.FilterClose := true;
  MsgFilter.FilterSend := true;
  MsgFilter.FilterRecv := true;
  SetLength(MsgFilter.FilterData, 0);
  CapturePid := 0;
  MsgSize := 0;
  SetEventHandler(MyEventHandler);
end;

procedure TMainForm.ListViewMessagesCustomDrawItem(Sender: TCustomListView;
  Item: TListItem; State: TCustomDrawState; var DefaultDraw: Boolean);
var
  Msg: ^TMSG_ENTRY;
begin
  Msg:= Item.Data;
  if (Msg.Cnn.Extra and EXTRA_ERR_SEQ)>0 then
    Sender.Canvas.Brush.Color := clRed
  else if (Msg.Cnn.Extra and EXTRA_USER_MSG)>0 then
       Sender.Canvas.Brush.Color := clGreen
    else
     Sender.Canvas.Brush.Color := clWhite;
end;

procedure TMainForm.ListViewMessagesSelectItem(Sender: TObject; Item: TListItem;
  Selected: Boolean);
var
  Msg: ^TMSG_ENTRY;
  i: Integer;
  PByte: ^Byte;
begin
      Msg := Item.Data;
      PByte := @(Msg.Msg);
      MPHexEditorData.DataSize := Msg.Msg.Length + (Msg.Msg.LengthEx Shr 16) + 6;
      for i:=0 to MPHexEditorData.DataSize-1 do
      begin
        MPHexEditorData.SetMemory(i, Chr(PByte^));
        Inc(PByte);
      end;
end;

procedure TMainForm.MPHexEditorDataSelectionChanged(Sender: TObject);
var
  I: Integer;
  ValByte: Byte;
  ValWord: Word;
  ValInt: Integer;
  SinglePtr: ^Single;
begin
  if MPHexEditorData.DataSize>0 then
    begin
      I := MPHexEditorData.SelStart;
      ValByte :=  MPHexEditorData.Data[I];
      if(I+1<MPHexEditorData.DataSize) then
        ValWord := ValByte + (MPHexEditorData.Data[I+1] Shl 8)
      else
        ValWord := ValByte;

      if(I+2<MPHexEditorData.DataSize) then
        ValInt := ValWord + (MPHexEditorData.Data[I+2] Shl 16)
      else
        ValInt := ValWord;

      if(I+3<MPHexEditorData.DataSize) then
         ValInt := ValInt + (MPHexEditorData.Data[I+3] Shl 24);

      SinglePtr := @ValInt;

      StatusBar.Panels[0].Text := Format('Byte:%d, Word:%d, DWord:%d, Float:%.2f, Hex:0x%.8X, Length=%d',
      [ValByte, ValWord, ValInt, SinglePtr^, ValInt, MPHexEditorData.SelCount]);
    end;
end;

procedure TMainForm.SubMenuAboutAppClick(Sender: TObject);
var
  dlg: TAboutForm;
begin
    dlg := TAboutForm.Create(Self);
    dlg.ShowModal;
    dlg.Free;
end;

procedure TMainForm.SubMenuApplyFilterClick(Sender: TObject);
begin
  ToolButtonApplyFilter.Down := not ToolButtonApplyFilter.Down;
  ToolButtonApplyFilterClick(Sender);
end;

procedure TMainForm.SubMenuAutoScrollClick(Sender: TObject);
begin
  SubMenuAutoScroll.Checked := not SubMenuAutoScroll.Checked;
  ToolButtonAutoScroll.Down := SubMenuAutoScroll.Checked;

end;

procedure TMainForm.SubMenuOpenDumpFileClick(Sender: TObject);
var
InStream:TFileStream;
Size: Integer;
i: Integer;
Read: Integer;
Msg: ^TMSG_ENTRY;
begin
 if DumpFileOpenDialog.Execute then
 begin
    Msg := nil;
    Size:= 0;
    InStream :=  TFileStream.Create(DumpFileOpenDialog.FileName, fmOpenRead);
    InStream.Read(Size, 4);
    InStream.Position := 4;
    if Size=DUMP_MAGIC then
      begin
        while True do
          begin
            Read := InStream.Read(Size, 4);
            InStream.Position;
            if Read=0 then break;

            Msg := AllocMem(Size);
            InStream.Read(Msg^, Size);
             InStream.Position;

            MsgList.Add(Msg);
            MsgSize := MsgSize + Size;
            if IsShouldShowMsg(Msg) then
              Self.InsertOneMsgToListView(Msg);
          end;
          StatusBar.Panels[0].Text := '已读取dump文件: '+  DumpFileOpenDialog.FileName;
          StatusBar.Panels[1].Text := Format('封包: %d, 大小: %s', [MsgList.Count, FormatByteSize(MsgSize)]);
      end
    else
      showMessage('无法打开'+DumpFileOpenDialog.FileName+'。文件不是有效的DUMP文件。');

      InStream.Free;
 end;

end;

procedure TMainForm.SubMenuOpenProcessClick(Sender: TObject);
begin
   Process.ProcessForm.ShowModal;
end;



procedure TMainForm.SubMenuQuitAppClick(Sender: TObject);
begin
  Self.Close;
end;

procedure TMainForm.SubMenuSaveDumpFileClick(Sender: TObject);
var
OutStream:TFileStream;
Size: Integer;
i: Integer;
Msg: ^TMSG_ENTRY;
begin
  if DumpFileSaveDialog.Execute then
  begin
     OutStream := TFileStream.Create(DumpFileSaveDialog.FileName, fmCreate);
     Size := DUMP_MAGIC;
     OutStream.Write(Size, 4);
     for i := 0 to MsgList.Count-1 do
     begin
        Msg := MsgList[i];
        Size :=  SizeOf(Msg.Cnn)+ 6 + (Msg.Msg.Length + Msg.Msg.LengthEx Shl 16);
        OutStream.Write(Size, 4);
        OutStream.Write(Msg^, Size);
     end;
     OutStream.Free;

     StatusBar.Panels[0].Text := '已保存dump文件: '+  DumpFileSaveDialog.FileName;
  end;
end;

procedure TMainForm.SubMenuSelectedSaveToFileClick(Sender: TObject);
var
  Str: string;
  myfile:TextFile;
begin
  if (ListViewMessages.SelCount>0) and (Self.SaveTextFileDialog.Execute) then
  begin
      Str := Self.GetPacketDataString(false);
      assignFile(myfile, SaveTextFileDialog.FileName);
      rewrite(myfile);
      writeln(myfile, Str);
      closefile(myfile);

      StatusBar.Panels[0].Text := '已保存到文件: '+  SaveTextFileDialog.FileName;
  end;
end;

procedure TMainForm.SubMenuShowHexEditorClick(Sender: TObject);
begin
   ToolButtonToggleHexEditor.Down := not ToolButtonToggleHexEditor.Down;
   ToolButtonToggleHexEditorClick(Sender);
end;

procedure TMainForm.SubMenuWindowOnTopClick(Sender: TObject);
begin
  ToolButtonWindowOnTop.Down := not ToolButtonWindowOnTop.Down;
  ToolButtonWindowOnTopClick(sender);
end;

procedure TMainForm.ToolButton3Click(Sender: TObject);
begin
    Tools.ToolsForm.Show;
end;

procedure TMainForm.ToolButtonApplyFilterClick(Sender: TObject);
begin
  MsgFilter.Enabled := ToolButtonApplyFilter.Down;
  SubMenuApplyFilter.Checked := MsgFilter.Enabled;
  UpdateAllMsgByFilter;
end;

procedure TMainForm.ToolButtonAutoScrollClick(Sender: TObject);
begin
   SubMenuAutoScroll.Checked := not SubMenuAutoScroll.Checked;
end;

procedure TMainForm.ToolButtonClearGridClick(Sender: TObject);
var
  i: Integer;
  Msg: Pointer;
begin
   ListViewMessages.Clear;
   for i := 0 to MsgList.Count-1 do
   begin
      Msg := MsgList[i];
      freemem(Msg);
   end;
   MsgList.Clear;
   MsgSize := 0;
   StatusBar.Panels[0].Text := '就绪';
   StatusBar.Panels[1].Text := '封包: 0, 大小: 0B';
   MPHexEditorData.DataSize := 0;

end;

procedure TMainForm.ToolButtonCloseProcessClick(Sender: TObject);
begin
    if IsCapturing and CheckCloseProcess then
      StopCapture(CapturePid);
end;

procedure TMainForm.ToolButtonFilterSettingClick(Sender: TObject);
begin
  FilterForm.ShowModal;
end;

procedure TMainForm.ToolButtonSaveAllPacketsClick(Sender: TObject);
var
  Str: string;
  myfile:TextFile;
begin
  if (ListViewMessages.Items.Count>0) and (Self.SaveTextFileDialog.Execute) then
  begin
      Str := Self.GetPacketDataString(true);
      assignFile(myfile, SaveTextFileDialog.FileName);
      rewrite(myfile);
      writeln(myfile, Str);
      closefile(myfile);

      StatusBar.Panels[0].Text := '已保存到文件: '+  SaveTextFileDialog.FileName;
  end;
end;

procedure TMainForm.ToolButtonSendPacketClick(Sender: TObject);
begin
    if IsCapturing then
    PacketForm.Show;
end;

procedure TMainForm.ToolButtonToggleCaptureClick(Sender: TObject);
begin
  if IsPausing then
  begin
    ToolButtonToggleCapture.ImageIndex := 11;
    ToolButtonToggleCapture.Hint := '暂停抓包';
    Self.Caption := APP_NAME + ' - '+ Format('游戏进程:[%d, %s] - 抓包中..', [ CapturePid, CaptureExe]);
    IsPausing := false;
  end
  else
  begin
    ToolButtonToggleCapture.ImageIndex := 1;
    ToolButtonToggleCapture.Hint := '继续抓包';
    Self.Caption := APP_NAME + ' - '+ Format('游戏进程:[%d, %s] - 已暂停抓包', [ CapturePid, CaptureExe]);
    IsPausing := true;
  end;
end;

procedure TMainForm.ToolButtonToggleHexEditorClick(Sender: TObject);
begin
   SubMenuShowHexEditor.Checked := ToolButtonToggleHexEditor.Down;
  if ToolButtonToggleHexEditor.Down then
    begin
    ListViewMessages.Align := alTop;
    ListViewMessages.Height := ListViewHeight;
    SplitterMain.Show;
      MPHexEditorData.Show;
    end
  else
    begin
      SplitterMain.Hide;
      MPHexEditorData.Hide;
      ListViewHeight := ListViewMessages.Height;
        ListViewMessages.Align := alClient;
    end;
end;

procedure TMainForm.ToolButtonWindowOnTopClick(Sender: TObject);
begin
  SubMenuWindowOnTop.Checked := not SubMenuWindowOnTop.Checked;
if ToolButtonWindowOntop.Down then
    FormStyle := fsStayOnTop
  else
    FormStyle := fsNormal;
end;

procedure TMainForm.OnEvent(var msg: tagMSG);
var
  EventArg: ^TEVENT_ARGS;
  EventType: Integer;
begin
  EventArg := Pointer(msg.wParam);
  EventType := EventArg.EventType;
  if EventType=0 then   //游戏数据包
    MainForm.OnEventGameMsg(EventArg.Param1, EventArg.Param2)
  else if EventType=1 then
    MainForm.OnEventConnNew(EventArg.Param1, EventArg.Param2)
  else if EventType=2 then
    MainForm.OnEventConnClosed(EventArg.Param1, EventArg.Param2);
end;

procedure TMainForm.OnEventGameMsg(Param1:Integer; Param2:Integer);
var
  Msg: ^TMSG_ENTRY;
begin
  //Writeln(PWChar(Format('GAME MSG:%x, %d: BEGIN--->>', [Param1, Param2])));
    if not IsPausing then
    begin
      if Param2=0 then   //怎么可能会有0呢。。。
          Exit;

      Msg := allocmem(Param2);
      CopyMemory(Msg, Pointer(Param1), Param2);
      MsgList.Add(Msg);
      if IsShouldShowMsg(Msg) then
        Self.InsertOneMsgToListView(Msg);

        MsgSize := MsgSize + Param2;
       StatusBar.Panels[1].Text := Format('封包: %d, 大小: %s', [MsgList.Count, FormatByteSize(MsgSize)]);
   //Writeln(PWChar(Format('GAME MSG:%x, %d: END-----<<', [Param1, Param2])));
    end;
end;


procedure TMainForm.PopMenuCopyToClipboardClick(Sender: TObject);
begin
  if ListViewMessages.SelCount>0 then
    begin
        Clipboard.SetTextBuf(PChar(Self.GetPacketDataString(false)));
        StatusBar.Panels[0].Text := '已复制到剪切板: '+ IntToStr(ListViewMessages.SelCount)+ '封包';
    end;

end;

procedure TMainForm.PopMenuCopyToSendDialogClick(Sender: TObject);
var
  Msg:^TMSG_ENTRY;
begin
    if ListViewMessages.SelCount>0 then
    begin
       Msg := ListViewMessages.Selected.Data;
       PacketForm.SetPacketData(@(Msg.Msg),
       Msg.Msg.Length+(Msg.Msg.LengthEx Shl 16)+6);
       PacketForm.Show;
    end;
end;

procedure TMainForm.PopMenuHexEditorCopyToClipboardClick(Sender: TObject);
var
Str: string;
I: Integer;
StartIdx: Integer;
EndIdx: Integer;
begin
    if MPHexEditorData.SelCount>0 then
    begin
    if MPHexEditorData.SelStart<MPHexEditorData.SelEnd then
    begin
       StartIdx := MPHexEditorData.SelStart;
       EndIdx   := MPHexEditorData.SelEnd;
    end
    else
    begin
       StartIdx := MPHexEditorData.SelEnd;
       EndIdx   := MPHexEditorData.SelStart;
    end;

    for I := StartIdx to EndIdx do
      Str := Str + Format('%.2X', [MPHexEditorData.Data[I]]);

     Clipboard.SetTextBuf(PChar(Str));
     StatusBar.Panels[0].Text := '已复制 '+IntToStr(MPHexEditorData.SelCount)+' 字节到剪切板。';
    end;
end;

procedure TMainForm.PopMenuHexEditorCopyToClipboardWithSpaceClick(
  Sender: TObject);
var
Str: string;
I: Integer;
StartIdx: Integer;
EndIdx: Integer;
begin
    if MPHexEditorData.SelCount>0 then
    begin
    if MPHexEditorData.SelStart<MPHexEditorData.SelEnd then
    begin
       StartIdx := MPHexEditorData.SelStart;
       EndIdx   := MPHexEditorData.SelEnd;
    end
    else
    begin
       StartIdx := MPHexEditorData.SelEnd;
       EndIdx   := MPHexEditorData.SelStart;
    end;

    for I := StartIdx to EndIdx do
      Str := Str + Format('%.2X ', [MPHexEditorData.Data[I]]);

     Clipboard.SetTextBuf(PChar(Str));
     StatusBar.Panels[0].Text := '已复制 '+IntToStr(MPHexEditorData.SelCount)+' 字节到剪切板。';
    end;
end;

procedure TMainForm.OnEventConnNew(Param1:Integer; Param2:Integer);
begin
    StatusBar.Panels[0].Text := '客户端已经成功连接！';
    IsCapturing := true;
    IsPausing := false;
    ToolButtonCloseProcess.Enabled := true;
    ToolButtonToggleCapture.Enabled := true;
    ToolButtonToggleCapture.ImageIndex := 11;
    ToolButtonToggleCapture.Hint := '暂停抓包';
    Self.Caption := APP_NAME + ' - '+ Format('游戏进程:[%d, %s] - 抓包中..', [ CapturePid, CaptureExe]);
end;

procedure TMainForm.OnEventConnClosed(Param1:Integer; Param2:Integer);
begin
    StatusBar.Panels[0].Text := '客户端已经断开连接！';
    ToolButtonCloseProcess.Enabled := false;
    ToolButtonToggleCapture.Enabled := false;
    ToolButtonToggleCapture.ImageIndex := 1;
    ToolButtonToggleCapture.Hint := '继续抓包';
    IsCapturing := false;
    IsPausing := false;
    CapturePid := 0;
    Self.Caption :=  APP_NAME + ' - 未打开游戏进程';
end;


function TMainForm.IsShouldShowMsg(MsgPtr:Pointer): Boolean;
var
Msg: ^TMSG_ENTRY;
DataPtr: ^Byte;
FilterPtr: ^Byte;
DataIdx: Integer;
FilterIdx: Integer;
TmpDataPtr: ^Byte;
IsDataFound: Boolean;
begin
  Msg := MsgPtr;
  IsShouldShowMsg := true;
  if MsgFilter.Enabled then
    begin
      if ((Msg.Cnn.Action=ACTION_SEND) and not MsgFilter.FilterSend ) or
      ((Msg.Cnn.Action=ACTION_RECV) and not MsgFilter.FilterRecv )  or
      ((Msg.Cnn.Action=ACTION_CONNECT) and not MsgFilter.FilterConnect )  or
      ((Msg.Cnn.Action=ACTION_CLOSE) and not MsgFilter.FilterClose )
       then
        begin
          IsShouldShowMsg:= false;
          Exit;
        end;

        if ((MsgFilter.FilterCmd<>0) and (MsgFilter.FilterCmd<> Msg.Msg.Cmd)) or
          ((MsgFilter.FilterLength<>-1) and (MsgFilter.FilterLength<>(Msg.Msg.Length+(Msg.Msg.LengthEx Shl 16))))
        then
        begin
          IsShouldShowMsg:= false;
          Exit;
        end;

        if Length(MsgFilter.FilterData)>0 then
        begin
            DataPtr := @(Msg.Msg.Data);
            for DataIdx := 0 to ((Msg.Msg.Length)+(Msg.Msg.LengthEx Shl 16))-1 do
            begin
              TmpDataPtr := Pointer(DataPtr);
              IsDataFound := true;
              for FilterIdx := 0 to Length(MsgFilter.FilterData)-1 do
                begin
                  if MsgFilter.FilterData[FilterIdx]<> TmpDataPtr^ then
                      IsDataFound := false
                  else
                    Inc(TmpDataPtr);
                end;
                if IsDataFound then
                begin
                  IsShouldShowMsg := true;
                  Exit;
                end
                else
                  Inc(DataPtr);
            end;
            IsShouldShowMsg := false;
            Exit;
        end;
    end
end;

 function TMainForm.GetPacketDataString(All: Boolean): string;
 var
  Msg: ^TMSG_ENTRY;
  subItem: TListItem;
  PByte: ^Byte;
  i: Integer;
  j: Integer;
  Str: string;
  Ip: Integer;
begin
    Str := '';
    for j := 0 to ListViewMessages.Items.Count-1 do
    begin
      subItem := ListViewMessages.Items[j];
      if All or subItem.Selected then
        begin
           Msg := subItem.Data;
           Str :=Str + '===============================================' + #13 + #10;
           if Msg.Cnn.Action=ACTION_SEND then
              Str := Str + 'SEND'
           else if Msg.Cnn.Action=ACTION_RECV then
              Str := Str + 'RECV'
           else if Msg.Cnn.Action=ACTION_CONNECT then
              Str := Str + 'CONNECT'
           else
              Str := Str + 'CLOSE';

           Ip := Msg.Cnn.Addr;
           Str:= Str + Format(', %d.%d.%d.%d:%d',[
              (Ip Shr 0) and $FF, (Ip Shr 8) and $FF,
              (Ip Shr 16) and $FF, (Ip Shr 24) and $FF, Msg.Cnn.Port]);

           Str := Str + Format(', CMD=0x%.2X%.2X', [
           Msg.Msg.Cmd and $00FF, (Msg.Msg.Cmd Shl 8) and $00FF]);

           Str := Str + Format(', LEN=0x%X',[Msg.Msg.Length+(Msg.Msg.LengthEx Shl 16)]);
           Str := Str + #13 + #10+'-----------------------------------------------'+#13 + #10;

           PByte := @(Msg.Msg.Data);
           for i:=0 to (Msg.Msg.Length+(Msg.Msg.LengthEx Shl 16))-1 do
           begin
              Str := Str + Format('%.2X ', [PByte^]);
              Inc(PByte);
              if ((i+1) mod 16)=0 then
                Str := Str + #13 + #10;
           end;
              Str := Str + #13 + #10+#13 + #10;

        end;
      end;
      GetPacketDataString := Str;
 end;

function TMainForm.FormatByteSize(Size: Integer): string;
var
  Formated: string;
begin
  if Size<1024 then
    Formated := Format('%dB', [Size])
  else if Size<1024*1024 then
    Formated := Format('%.2fKB', [size/1024])
  else if size<1024*1024*1024 then
    Formated := Format('%.2fMB', [size/(1024*1024)])
  else
    Formated := Format('%.2fKB', [size/(1024*1024*1024)]);

    Result :=  Formated;
end;

procedure TMainForm.UpdateAllMsgByFilter();
var
  i: Integer;
  Msg: ^TMSG_ENTRY;
begin
   ListViewMessages.Clear;
   for i := 0 to MsgList.Count-1 do
     begin
        Msg := MsgList[i];
        if IsShouldShowMsg(msg) then
          InsertOneMsgToListView(Msg);
     end;
end;

 procedure TMainForm.InsertOneMsgToListView(MsgPtr:Pointer);
 var
 Msg:^TMSG_ENTRY;
  subItem: TListItem;
  PByte: ^Byte;
  B0: Byte;
  B1: Byte;
  B2: Byte;
  B3: Byte;
  Str: string;
  i: Integer;
 begin
   Msg := MsgPtr;
   subItem := ListViewMessages.Items.Add;
   subItem.Data := Pointer(Msg);
   subItem.Caption := IntToStr(subItem.Index);
   subItem.SubItems.Add(IntToStr(Msg.Cnn.Pid));
   subItem.SubItems.Add(IntToStr(Msg.Cnn.Socket));
   PByte := @(Msg.Cnn.Addr);
   B0 := PByte^; Inc(PByte);
   B1 := PByte^; Inc(PByte);
   B2 := PByte^; Inc(PByte);
   B3 := PByte^; Inc(PByte);
   subItem.SubItems.Add(Format('%d.%d.%d.%d:%d',[
        B0, B1, B2, B3, Msg.Cnn.Port]));
   if Msg.Cnn.Action=ACTION_SEND then
     subItem.SubItems.Add('SEND')
   else if Msg.Cnn.Action=ACTION_RECV then
      subItem.SubItems.Add('RECV')
   else if Msg.Cnn.Action=ACTION_CONNECT then
      subItem.SubItems.Add('CONNECT')
   else
      subItem.SubItems.Add('CLOSE');

   if (Msg.Cnn.Action=ACTION_SEND) or (Msg.Cnn.Action=ACTION_RECV) then
   begin
     PByte := @(Msg.Msg.Cmd);
     B0 := PByte^; Inc(PByte);
     B1 := PByte^; Inc(PByte);
     subItem.SubItems.Add(Format('%.2x %.2x', [B0, B1]));
     subItem.SubItems.Add(Format('%.2x',[Msg.Msg.Sequece]));
     subItem.SubItems.Add(Format('%.2x',[Msg.Msg.Length+(Msg.Msg.LengthEx Shl 16)]));

     Str := '';
     PByte := @(Msg.Msg.Data);
     for i:=0 to min(Msg.Msg.Length, 64)-1 do
     begin
        Str := Str + Format('%.2X ', [PByte^]);
        Inc(PByte);
     end;
     subItem.SubItems.Add(Str);

     Str := '';
     if ((Msg.Cnn.Extra and EXTRA_FIXED_SEQ)>0) then
         Str := Str + 'FIXED_SEQ ';

     if ((Msg.Cnn.Extra and EXTRA_USER_MSG)>0) then
         Str := Str + 'USER_SEND ';

     subItem.SubItems.Add(Str);


     if ToolButtonAutoScroll.Down then
        subItem.MakeVisible(true);

   end;
 end;

 function TMainForm.CheckCloseProcess: Boolean ;
 begin
   begin
  if (CapturePid>0) and IsCapturing and (Packet.HasSendPacket) and (Packet.LastSendPid=CapturePid) then
    if (MessageDlg(
    '当前打开的进程已经手动发送过封包，强制停止抓包可能会导致游戏掉线，确认退出吗？',
    mtConfirmation, [mbYes, mbNo], 0
    )=mrYes) then
      CheckCloseProcess := true
      else
     CheckCloseProcess := false
   else
    CheckCloseProcess := true;
 end;
 end;

 end.
