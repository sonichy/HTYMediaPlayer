# Qt 海天鹰媒体播放器
![alt](preview.jpg)  
Linux 平台基于 Qt 的 QMultiMedia 的媒体播放程序，支持某些直播。  
已编译好的 HTYMediaPlayer 程序适用64位 Linux 系统 Qt5 环境，双击运行。  

### 1.4版 解决有视频无声音的问题。
### 1.3版 直播列表做到窗体里面，支持显隐，记忆显隐，根据媒体信息设置视频大小。
### 1.2版 增加截图和剧情连拍。
![alt](summary.jpg)  

### 1.1版 增加直播列表。

### 错误排除
没有权限 ：  
程序右键属性，勾选“允许以程序执行”。  
  
深度文件管理器直接运行程序无法载入直播列表：  
方法1：从终端启动。  
方法2：修改desktop文件中的路径，从desktop文件启动。  
  
Project ERROR: Unknown module(s) in QT: multimedia  
sudo apt-get install qtmultimedia5-dev  

有声音无视频：  
https://bugreports.qt.io/browse/QTBUG-23761  
卸载: sudo apt-get remove gstreamer1.0-vaapi  
感谢 liujianfeng@deepin.org 。
