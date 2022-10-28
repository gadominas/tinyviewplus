# Tiny View Plusのビルド

## 開発環境のセットアップ

### macOSの場合

1. Xcodeをインストールする
   - [App Store](https://itunes.apple.com/ca/app/xcode/id497799835?mt=12)からXcodeをインストールする÷
2. openFrameworks 0.11.2をインストールする
   - [openFrameworks for osx (zipファイル)](https://openframeworks.cc/download/)をダウンロードする
   - 拡張属性を取り除く
     - $ xattr -cr of\_\*\_osx_release.zip
   - 適当な場所に展開する

### Windowsの場合

1. Visual Studioをインストールする
   - [こちらのドキュメント](https://openframeworks.cc/setup/vs/)を参考とする
     - Visual Studio Community 2017を前提に書かれているが、2019でも問題無い
     - 「openFrameworks plugin for Visual Studio」は実施しない
2. openFrameworks 0.11.2をインストールする
   - 適当な場所に[openFrameworks for visual studio](https://openframeworks.cc/download/)を展開する
3. Git for Windowsをインストールする
   - [こちらのドキュメント](https://qiita.com/elu_jaune/items/280b4773a3a66c7956fe)を参考にインストールする
   - これに含まれるGit Bashを使って、GitHubからのファイルの取得や、ローカルでのファイル操作を行うことになる

## プロジェクトの作成

1. 必要なアドオンをインストールしておく
   
   ```
   $ cd addons/
   $ git clone -b fix-of_v0.10 https://github.com/t-asano/ofxTrueTypeFontUC.git
   $ git clone https://github.com/t-asano/ofxAruco.git
   $ git clone https://github.com/kylemcdonald/ofxCv.git
   $ git clone https://github.com/t-asano/ofxZxing.git
   $ git clone https://github.com/t-asano/ofxJoystick.git
   ```

2. プロジェクトジェネレーターで、以下のアドオンを含むプロジェクトを作成する。プロジェクト名は「tinyviewplus」とする。
   
   - ofxOsc
   - ofxTrueTypeFontUC
   - ofxAruco
   - ofxCv
   - ofxOpenCv
   - ofxPoco
   - ofxZxing
   - ofxJoystick
   - ofxXmlSettings

3. 作成したプロジェクトに、Tiny View Plusのファイル一式をマージする
   
   ```
   $ cd apps/myApps
   $ mv tinyviewplus tvptemp
   $ git clone https://github.com/t-asano/tinyviewplus.git
   $ cp tvptemp/* tinyviewplus/
   $ cp -r tvptemp/*.xcodeproj tinyviewplus/ # macOSの場合のみ
   $ rm -r tvptemp
   ```

## ビルドの設定

### macOSの場合

1. プロジェクトファイル(tinyviewplus.xcodeproj)をXcodeで開く

### Windowsの場合

1. ソリューションファイル(tinyviewplus.sln)をVisualStudioで開く

2. 文字コードを固定する
   
   - tinyviewplusプロジェクトのプロパティを開く
   - [構成プロパティ] -> [C/C++] -> [コマンドライン]  と進む
   - 「全ての構成」「すべてのプラットフォーム」に対して、「追加のオプション」として「/source-charset:utf-8」を追加する

3. オブジェクトファイルの競合を回避する
   
   - tinyviewplusプロジェクトのプロパティを開く
   - [構成プロパティ] -> [C/C++] -> [出力ファイル] と進む
   - 「オブジェクトファイル名」を「$(IntDir)/%(RelativeDir)/」に変更する

4. openFrameworks の ofSystemUtils.cpp を改造する(文字化け対策)

```cpp
static void narrow(const std::wstring &src, std::string &dest) {
    setlocale(LC_CTYPE, "");
    char *mbs = new char[src.length() * MB_CUR_MAX + 1];
    wcstombs(mbs, src.c_str(), src.length() * MB_CUR_MAX + 1);
    dest = mbs;
    delete[] mbs;
}

std::string convertWideToNarrow( const wchar_t *s, char dfault = '?', const std::locale& loc = std::locale() )
{
#if 0
    std::ostringstream stm;
    while (*s != L'\0') {
        stm << std::use_facet< std::ctype<wchar_t> >(loc).narrow(*s++, dfault);
    }
    return stm.str();
#else
    std::wstring wstr(s);
    std::string ret;
    narrow(wstr, ret);
    return ret;
#endif
}
```

5. アプリ起動時にコンソールが表示されるのを禁止する
   - tinyviewplusプロジェクトのプロパティを開く
   - [構成プロパティ] -> [リンカ] -> [詳細] と進む
   - 「全ての構成」「すべてのプラットフォーム」に対して、「エントリポイント」として「mainCRTStartup」を指定する

### Linuxの場合(実験的)

カメラが二重に検出されてしまうバグがあるので、openFrameworksの修正が必要。  
https://github.com/t-asano/tinyviewplus/pull/22

> For the correct definition of cameras, is required error correction in
> openFrameworks. Locate file "../libs/openFrameworks/video/
> ofGstVideoGrabber.cpp". Find the line:
> 
> guint cap = v2cap.capabilities;
> 
> Replace it with:
> 
> guint cap = v2cap.device_caps;
> 
> Rebuild openFrameworks. See here for more info:
> [openframeworks/openFrameworks#6556](https://github.com/openframeworks/openFrameworks/issues/6556)

## ビルドと起動

### macOSの場合

1. ビルドターゲットを「tinyviewplus Release」に設定する
2. [Product] -> [Build] でビルドする
3. ビルドが成功したら [Product] -> [Run] で起動する

### Windowsの場合

1. ビルドターゲットを「Release」「x64」に設定する
2. [ビルド] -> [ソリューションのビルド] でビルドする
3. ビルドが成功したら [デバッグ] -> [デバッグの開始] で起動する

## アップデート

1. Tiny View Plusとアドオンを更新する
   
   ```
   $ cd apps/myApps/tinyviewplus
   $ git pull
   $ cd ../../../addons/ADDON_NAME
   $ git pull
   ```

2. アドオンを追加する(必要な場合のみ)
   
   - Gitコマンドで新規のアドオンを取得する
   - プロジェクトジェネレーターでアドオンを追加する

以上