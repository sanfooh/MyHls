目前支持功能：
1、快速切片。
2、拉取rtsp。
3、推rtsp。
后续支持：
1、rtsp分发。

API:
Play/tag
播放某个tag

AddPull/tag/pullAddress
添加拉模式，执行此API以后，服务会自主去拉取pullAddress所指向的rtsp地址
例如
http://localhost:50010/AddPull/aa/rtsp:%%192.168.1.11
即向服务器添加了一个标签为aa的输入流，它的拉流地址是rtsp:%%192.168.1.11，其中/用%替换

AddPush/tag/
添加推模式，执行此API以后，服务会返回一个push地址，会侦听30秒，在30秒内使用MyPush给它推流
例如
http://localhost:50010/AddPush/aa
返回
{"ret":0,"data":{"type":push,"pushUrl":"rtsp://localhost:40000/live.sdp","pullUrl":"http://localhost:50010/Play/0"},"message":"connect in 30 seconds"}
在30秒内使用MyPush对rtsp://localhost:40000/live.sdp进行推流即可


GetPullList
获取所有客户端连接信息

GetUsage
获取服务器的CPU，内存，IO，句柄，线程等信息
