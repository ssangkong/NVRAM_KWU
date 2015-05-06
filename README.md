1. 리눅스 커널을 다운로드 받는다. https://github.com/analogdevicesinc/linux
2. 이 레포지터리에 있는 .config 파일을 다운받은 커널 디렉토리에 넣고, 컴파일한다.
http://wiki.analog.com/resources/tools-software/linux-drivers/platforms/zynq#linux_with_hdmi_video_output_on_the_zed_and_zc702_zc706_boards
3. qt-everywhere-opensource-src-4.7.4 디렉토리에 들어가서 컴파일을 진행한다.
  ./configure -opensource --confirm-license
  make
4. rootfs에 필요한 것들을 포함시킨다.
  - qt, python, strace, blktrace, debugfs, e2fsprogs, automation.py
5. 타겟 장치에서 부팅한 후, automation.py에 인자로 browser의 경로를 넣고 실행한다.
  ./automation.py /usr/local/Trolltech/Qt-4.7.4/demos/browser/browser
