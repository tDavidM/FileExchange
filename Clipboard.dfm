object fClipboard: TfClipboard
  Left = 0
  Top = 0
  Caption = 'Clipboard'
  ClientHeight = 279
  ClientWidth = 617
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  DesignSize = (
    617
    279)
  PixelsPerInch = 96
  TextHeight = 13
  object bSend: TButton
    Left = 534
    Top = 248
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Send'
    TabOrder = 0
    OnClick = bSendClick
  end
  object mText: TMemo
    Left = 7
    Top = 8
    Width = 602
    Height = 234
    Hint = 'Shift+Clic to erase'
    Anchors = [akLeft, akTop, akRight, akBottom]
    ParentShowHint = False
    ReadOnly = True
    ShowHint = True
    TabOrder = 1
    OnMouseDown = mTextMouseDown
  end
  object eText: TEdit
    Left = 8
    Top = 250
    Width = 520
    Height = 21
    Anchors = [akLeft, akRight, akBottom]
    TabOrder = 2
  end
end
