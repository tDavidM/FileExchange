object fSelPassKey: TfSelPassKey
  Left = 0
  Top = 0
  BorderIcons = []
  BorderStyle = bsSingle
  Caption = 'Enter a Key'
  ClientHeight = 72
  ClientWidth = 292
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object lKey: TLabel
    Left = 8
    Top = 11
    Width = 72
    Height = 13
    Caption = 'Encryption Key'
    OnDblClick = lKeyDblClick
  end
  object bOk: TButton
    Left = 209
    Top = 40
    Width = 75
    Height = 25
    Caption = 'Ok'
    TabOrder = 0
    OnClick = bOkClick
  end
  object bCancel: TButton
    Left = 8
    Top = 40
    Width = 75
    Height = 25
    Caption = 'Cancel'
    TabOrder = 1
    OnClick = bCancelClick
  end
  object eKey: TEdit
    Left = 91
    Top = 8
    Width = 157
    Height = 21
    MaxLength = 24
    PasswordChar = '*'
    TabOrder = 2
  end
  object bbViewKey: TBitBtn
    Left = 255
    Top = 7
    Width = 29
    Height = 23
    Hint = 'Show Key'
    Glyph.Data = {
      B6080000424DB608000000000000360400002800000030000000180000000100
      0800000000008004000000000000000000000001000000000000000000000000
      80000080000000808000800000008000800080800000C0C0C000C0DCC000F0CA
      A6000020400000206000002080000020A0000020C0000020E000004000000040
      20000040400000406000004080000040A0000040C0000040E000006000000060
      20000060400000606000006080000060A0000060C0000060E000008000000080
      20000080400000806000008080000080A0000080C0000080E00000A0000000A0
      200000A0400000A0600000A0800000A0A00000A0C00000A0E00000C0000000C0
      200000C0400000C0600000C0800000C0A00000C0C00000C0E00000E0000000E0
      200000E0400000E0600000E0800000E0A00000E0C00000E0E000400000004000
      20004000400040006000400080004000A0004000C0004000E000402000004020
      20004020400040206000402080004020A0004020C0004020E000404000004040
      20004040400040406000404080004040A0004040C0004040E000406000004060
      20004060400040606000406080004060A0004060C0004060E000408000004080
      20004080400040806000408080004080A0004080C0004080E00040A0000040A0
      200040A0400040A0600040A0800040A0A00040A0C00040A0E00040C0000040C0
      200040C0400040C0600040C0800040C0A00040C0C00040C0E00040E0000040E0
      200040E0400040E0600040E0800040E0A00040E0C00040E0E000800000008000
      20008000400080006000800080008000A0008000C0008000E000802000008020
      20008020400080206000802080008020A0008020C0008020E000804000008040
      20008040400080406000804080008040A0008040C0008040E000806000008060
      20008060400080606000806080008060A0008060C0008060E000808000008080
      20008080400080806000808080008080A0008080C0008080E00080A0000080A0
      200080A0400080A0600080A0800080A0A00080A0C00080A0E00080C0000080C0
      200080C0400080C0600080C0800080C0A00080C0C00080C0E00080E0000080E0
      200080E0400080E0600080E0800080E0A00080E0C00080E0E000C0000000C000
      2000C0004000C0006000C0008000C000A000C000C000C000E000C0200000C020
      2000C0204000C0206000C0208000C020A000C020C000C020E000C0400000C040
      2000C0404000C0406000C0408000C040A000C040C000C040E000C0600000C060
      2000C0604000C0606000C0608000C060A000C060C000C060E000C0800000C080
      2000C0804000C0806000C0808000C080A000C080C000C080E000C0A00000C0A0
      2000C0A04000C0A06000C0A08000C0A0A000C0A0C000C0A0E000C0C00000C0C0
      2000C0C04000C0C06000C0C08000C0C0A000F0FBFF00A4A0A000808080000000
      FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00FFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFF07A49B9BA407FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF08F7F7F7F7
      07FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFED9B40009BA4A49B00409BF7FFFFFFFF
      FFFFFFFFFFFFFFFF07F79B9BF70707F7A45BF707FFFFFFFFFFFFFFFFFFFFFF52
      490052FFFFFFFFFFFF49004952FFFFFFFFFFFFFFFFFFFFA4A49BA4FFFFFFFFFF
      FFF79BA4A4FFFFFFFFFFFFFFFFF700494952FFFFFFF7F7FFFFFF52494900F7FF
      FFFFFFFFFF079BA4A4A4FFFFFF07F7FFFFFFF7A4A49BF7FFFFFFFFFFA4004949
      49EEFFFF49404049FFFFED49494900F7FFFFFFFFF79BA4A4A407FFFFA49BA49B
      07FF08A4A4A49BF7FFFFFF9B4049494949FFFF9B4049494052FFFF4949494940
      A4FFFFF79BA4A4A4A4FFFFF7A4A4A4A4A4FFFFA4A4A4A4A4F7FFFF0049494949
      49FFFF494949494949FFFF494949494949FFFF9BA4A4A4A4A4FFFFF7A4A4A4A4
      5BFFFFA4A4A4A4A49BFFFFFF494949494908FFF700494900F7FF074949494949
      FFFFFFFFA4A4A4A4A408FF079BA4A49BF7FFFFA4A4A4A49BFFFFFFFFFF404049
      40F7FFFFF70000A4FFFFA440494000FFFFFFFFFFFFA49BA4A4F7FFFF079B9B07
      FFFF07A4A4A45BFFFFFFFFFFFFFFA4494040EDFFFFFFFFFFFFED4049409BFFFF
      FFFFFFFFFFFF07A4A49B07FFFFFFFFFFFF079BA4A4A4FFFFFFFFFFFFFFFFFFFF
      9B4000A408FFFF08A4000049F7FFFFFFFFFFFFFFFFFFFFFFF7A45BF708FFFF08
      079B9BA4FFFFFFFFFFFFFFFFFFFFFFFFFFFFF74900004000009B07FFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFF07A45B5B9B5B5BF7FFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF}
    NumGlyphs = 2
    TabOrder = 3
    OnClick = bbViewKeyClick
  end
  object bTiger: TButton
    Left = 109
    Top = 40
    Width = 75
    Height = 25
    Caption = 'Tiger'
    TabOrder = 4
    Visible = False
    OnClick = bTigerClick
  end
end
