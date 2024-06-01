object PacketForm: TPacketForm
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = #21457#36865#28216#25103#23553#21253
  ClientHeight = 264
  ClientWidth = 653
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poOwnerFormCenter
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 480
    Height = 13
    Caption = #35831#22312#19979#38754#30340#32534#36753#22120#20013#36755#20837#23553#21253#20869#23481#65292#21482#38656#36755#20837#21629#20196#21495#21644#25968#25454#21363#21487#65292#24207#21495#21644#38271#24230#20250#33258#21160#35843#25972#12290
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object BitBtnOk: TBitBtn
    Left = 280
    Top = 232
    Width = 75
    Height = 25
    Caption = #21457#36865
    Kind = bkOK
    NumGlyphs = 2
    TabOrder = 0
    OnClick = BitBtnOkClick
  end
  object BitBtnCancel: TBitBtn
    Left = 361
    Top = 232
    Width = 75
    Height = 25
    Caption = #21462#28040
    Kind = bkCancel
    NumGlyphs = 2
    TabOrder = 1
    OnClick = BitBtnCancelClick
  end
  object BitBtnReset: TBitBtn
    Left = 199
    Top = 232
    Width = 75
    Height = 25
    Caption = ' '#37325#32622
    Kind = bkRetry
    NumGlyphs = 2
    TabOrder = 2
    OnClick = BitBtnResetClick
  end
  object MPHexEditor: TMPHexEditorEx
    Left = 8
    Top = 27
    Width = 637
    Height = 199
    Cursor = crIBeam
    BackupExtension = '.bak'
    ClipboardAsHexText = True
    PrintOptions.MarginLeft = 20
    PrintOptions.MarginTop = 15
    PrintOptions.MarginRight = 25
    PrintOptions.MarginBottom = 25
    PrintOptions.Flags = [pfSelectionBold, pfMonochrome]
    PrintFont.Charset = DEFAULT_CHARSET
    PrintFont.Color = clWindowText
    PrintFont.Height = -15
    PrintFont.Name = 'Courier New'
    PrintFont.Style = []
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    PopupMenu = PopupMenuHexEditor
    TabOrder = 3
    BytesPerRow = 16
    BytesPerColumn = 1
    Translation = tkAsIs
    OffsetFormat = '-!10:0x|'
    Colors.Background = clWindow
    Colors.ChangedBackground = 11075583
    Colors.ChangedText = clMaroon
    Colors.CursorFrame = clNavy
    Colors.Offset = clBlack
    Colors.OddColumn = clBlue
    Colors.EvenColumn = clNavy
    Colors.CurrentOffsetBackground = clBtnShadow
    Colors.OffsetBackground = clBtnFace
    Colors.CurrentOffset = clBtnHighlight
    Colors.Grid = clBtnFace
    Colors.NonFocusCursorFrame = clAqua
    Colors.ActiveFieldBackground = clWindow
    FocusFrame = True
    DrawGridLines = True
    Version = 'october 7th, 2010; ?markus stephany, http://launchpad.net/dcr'
    InsertMode = True
    OnChange = MPHexEditorChange
    ShowRuler = True
    BytesPerBlock = -1
  end
  object CheckBoxInsertMode: TCheckBox
    Left = 8
    Top = 232
    Width = 97
    Height = 17
    Caption = #25554#20837#27169#24335
    Checked = True
    State = cbChecked
    TabOrder = 4
    OnClick = CheckBoxInsertModeClick
  end
  object PopupMenuHexEditor: TPopupMenu
    Images = MainForm.ImageListMain
    Left = 240
    Top = 136
    object PopMenuSelectionCopyToClipboard: TMenuItem
      Caption = #22797#21046#36873#25321#21040#21098#20999#26495'(&C)'
      ImageIndex = 14
      OnClick = PopMenuSelectionCopyToClipboardClick
    end
    object PopMenuPasteFromClipboard: TMenuItem
      Caption = #20174#21098#20999#26495#31896#36148#25554#20837'(&P)'
      ImageIndex = 15
      OnClick = PopMenuInsertFromClipboardClick
    end
    object PopMenuInsertFromClipboard: TMenuItem
      Caption = #20174#21098#20999#26495#31896#36148#35206#30422'(&O)'
      ImageIndex = 16
      OnClick = PopMenuPasteFromClipboardClick
    end
  end
end
