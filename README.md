## :rocket: 编码不易，点个star！ ##


**本项目使用正点原子硬件与基础代码编写**

#### 显示屏只针对ILI9341编写，如果是其它屏驱动请自行调整ILI93xx.h宏定义

当前编辑版本与编译环境：

- Windows 10 1903
- 战舰STM32F103ZET6 v2.0
- Keil MDK 5.27
- STemWin 5.28
- uCOS III 3.03
- LCD为240 x 320分辨率

- 说明：由于现在应用代码太多了，任务堆栈调不了太大。估计是堆栈的原因，有可能在关闭窗口时有小概率会异常。代码太多MCU Flash已经装不下了(开了3级优化)，目前暂停升级。
- 注意：LCD代码只支持ILI9341，其它型号注释了

* :memo:日志:<br/>
   > 1. :art:修改设置界面为SwipeList控件<br/>
        :art:修改图片浏览器小bug，还有bug<br/>
        :art:增加IAP运行器<br/>
		:art:增加Nes游戏，近期将更新全速有声音版本<br/>
		:art:资源监视器全局开启<br/>
		:art:增加HardFault_Handle错误显示界面并倒计时重启<br/>
		:art:增加开机检测功能<br/>
		:art:增加日期与时间设置功能（界面太丑，还有小bug）<br/>
   > 2. :art:Nes游戏改为汇编版，有声音，可能是STemWin的原因速度会比原版慢点<br/>
   > 3. :art:无法忍受emWin5.32触屏bug，降到5.28<br/>
   > 4. :art:增加时钟<br/>
   > 5. :art:增加音乐播放，支持后台播放，但如果打开应用有读取SD时会稍微停顿下<br/>
   > 6. :art:增加视频播放，无声音。仅用来学习因为帧数感人。<br/>
   
* :memo:可能的计划:<br/>
   > 1. 暗黑模式


## 先将目录中的"WZTSYSTEM"文件夹复制到SD卡根目录，并运行工程然后下载程序。

* emWin字库需要横向取模，如果运行程序后字体倒了是正常现象。需要更新字库为横向取模的就行。但频繁更新字库会损坏硬件，因此直接修改LCD相关代码把之前竖向取模显示改为横向取模显示即可。
* 请自行确认所有硬件与线路连接正确，目前代码不检查相关内容。

:art: 部分截图<br/>
<div>
<img width="240" height="320" src="./PRTSC/PRTSC (01).bmp"/>
<img width="240" height="320" src="./PRTSC/PRTSC (02).bmp"/>
<img width="240" height="320" src="./PRTSC/PRTSC (03).bmp"/>
</div>
<div>
<img width="240" height="320" src="./PRTSC/PRTSC (04).bmp"/>
<img width="240" height="320" src="./PRTSC/PRTSC (05).bmp"/>
<img width="240" height="320" src="./PRTSC/PRTSC (06).bmp"/>
</div>
<div>
<img width="240" height="320" src="./PRTSC/PRTSC (07).bmp"/>
<img width="240" height="320" src="./PRTSC/PRTSC (08).bmp"/>
<img width="240" height="320" src="./PRTSC/PRTSC (09).bmp"/>
</div>
<div>
<img width="240" height="320" src="./PRTSC/PRTSC (10).bmp"/>
<img width="240" height="320" src="./PRTSC/PRTSC (11).bmp"/>
<img width="240" height="320" src="./PRTSC/PRTSC (25).bmp"/>
</div>
<div>
<img width="240" height="320" src="./PRTSC/PRTSC (13).bmp"/>
<img width="240" height="320" src="./PRTSC/PRTSC (14).bmp"/>
<img width="240" height="320" src="./PRTSC/PRTSC (15).bmp"/>
</div>
<div>
<img width="240" height="320" src="./PRTSC/PRTSC (16).bmp"/>
<img width="240" height="320" src="./PRTSC/PRTSC (17).bmp"/>
<img width="240" height="320" src="./PRTSC/PRTSC (18).bmp"/>
</div>
<div>
<img width="240" height="320" src="./PRTSC/PRTSC (19).bmp"/>
<img width="240" height="320" src="./PRTSC/PRTSC (20).bmp"/>
<img width="240" height="320" src="./PRTSC/PRTSC (21).bmp"/>
</div>
<div>
<img width="240" height="320" src="./PRTSC/PRTSC (22).bmp"/>
<img width="240" height="320" src="./PRTSC/PRTSC (23).bmp"/>
<img width="240" height="320" src="./PRTSC/PRTSC (24).bmp"/>
</div>


