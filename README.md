# WebRTC Janus を動作させるサンプル

WebRTC Janus を動作させるためのサンプルプログラムになります。

|フォルダ|備考|
|:--|:--|
|gstreamer-sample|GStreamer から WebRTC Janus に映像を配信するサンプル|
|webrtc-janus|WebRTC Janus を動作させるサンプル|
|docker-compose-videoroom.yml|VideoRoom Plugin への配信を行う設定ファイル|
|docker-compose.yml|Streaming Plugin への配信を行う設定ファイル|

## WebRTC Janus を HTTPS 化

下記のファイルに WebRTC Janus の設定が記載されています。

```
janus/config/janus.transport.http.jcfg
```

デフォルトの設定から下記の部分を修正してあります。

```
general: {
        https = true         # 変更
        secure_port = 8089   # コメントアウト外す
}
certificates: {
        cert_pem = "/opt/certs/lws-cert.pem"     # 変更
        cert_key = "/opt/certs/private-key.pem"  # 変更
}
```

## Streaming Plugin の設定

下記のファイルに Streaming Plugin の設定が記載されています。

```
janus/config/janus.plugin.streaming.jcfg
```

デフォルトのままになっていますが、配信したい映像、音声に合わせて設定を変更してください。

## local web server を HTTPS 化

local web server は、デフォルトでは実行されたフォルダをドキュメントルートとして HTTP サーバを起動します。

引数に --https を付けることで、HTTPS 化することができます。<br>
デフォルトでは、ws に内包された証明書を使用します。

--cert と --key を付けることで、指定した証明書を使用することができます。

```
$ ws --https --cert /opt/certs/lws-cert.pem --key /opt/certs/private-key.pem
```

VideoRoom Plugin などで、Web カメラを使用する場合には、HTTPS で起動する必要があります。

## ビルド

下記のコマンドを実行することで、WebRTC Janus の環境を構築します。

```
$ docker-compose build
```

## 実行

下記のコマンドで Streaming Plugin への映像配信を実行します。

```
$ docker-compose up
```

下記の URL にアクセスすることで WebRTC Janus のデモサイトを表示することができます。<br>

```
https://{DOCKER_IP}:8000
```

このサイトから、Demos を開き、Streaming Plugin のデモページを開くことで、確認することができます。

## 配信する映像・音声の変更

docker-compose.yml に記載されている command を変更することで、配信する映像・音声を変更することができます。

```yml
    command: >
      gst-launch-1.0 
        audiotestsrc ! 
          audioresample ! audio/x-raw,channels=1,rate=48000 ! 
          opusenc bitrate=20000 ! 
            rtpopuspay ! udpsink host=webrtc-janus port=5002 
        videotestsrc ! 
          video/x-raw,width=640,height=480,framerate=30/1 ! 
          videoconvert ! timeoverlay ! 
          vp8enc error-resilient=1 ! 
            rtpvp8pay mtu=1500 ! udpsink host=webrtc-janus port=5004
```

## VideoRoom Plugin の実行

下記のコマンドで、VideoRoom Plugin への映像配信を実行します。

```
$ docker-compose -f docker-compose-videoroom.yml build
$ docker-compose -f docker-compose-videoroom.yml up
```

同じように WebRTC Janus のデモサイトを表示して、VideoRoom のデモページを開くことで、確認することができます。

