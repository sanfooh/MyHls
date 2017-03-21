# MyHls
MyHls是一个基于windows的轻量级hls服务，可拉取摄像头rtsp流以hls进行分发，其特点是内存切片且进行session处理，网络好的情况延时在1秒以内，它是基于ffmpeg库和mongoose进行开发的所以代码量很少,工程使用c++11 在visual studio2015运行,经过少量修改即可移植到linux。运行程序以后，用支持html5浏览器打开http://localhost:8070,添加拉流rtsp地址，要注意的是斜杠需用%替换如：rtsp://192.168.1.11 应写成rtsp:%%192.168.1.11,添加完以后点play按钮即可播放
QQ群：569895682
