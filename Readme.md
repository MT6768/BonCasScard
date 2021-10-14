
# BonCasScard

BonCasScard v1.0.0 (http://www2.wazoku.net/2sen/dtvvup/source/BonCasScard.zip) に同梱されているソースコードのミラー。

仮想カードリーダー名を変更したこと、ビルド環境を Visual Studio 2019 (VS2019) に更新したこと、この Readme.md を作成したこと以外はオリジナルのままとなっている。  

### 概要

BonCasClient の機能を組み込んだ WinSCard.dll 。

winscard.dll と winscard.ini を TVTest などの利用するソフトと同じフォルダに配置し、winscard.ini を適宜編集して BonCasLink サーバーへの接続先を設定する。  
あとは他の各種 WinSCard.dll 同様に、仮想カードリーダーとして動作する。

### ダウンロード

[BonCasScard v1.0.0-patch](https://github.com/mizunoko/BonCasScard/releases/download/v1.0.0-patch/BonCasScard_v1.0.0-patch.zip)

### 利用時の注意

BonCasScard のビット数 (32bit or 64bit) は、各種 WinSCard.dll 同様に TVTest などの BonCasScard を利用するソフトに合わせる必要がある。  
さもなければ、ライブラリの読み込みに失敗し、スクランブルの解除に失敗する。

ただし、BonCasScard と BonCasLink (BonCasServer, BonCasService) が異なるビット数になっている事は問題ない。  
BonCasScard と BonCasLink が TCP で通信しているためで、たとえば TVTest と BonCasScard が 64bit 、BonCasLink が 32bit の場合でも正常に読み込める。
