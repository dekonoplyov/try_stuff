# WIN_MAN
Simple windows manager project to understand xlib internals 

Build
```
mkdir build
cd build
cmake ..
make
```

Run
```
./run.sh
```

Requirements  
- libx11-dev
- libgoogle-glog-dev
- Xephyr
- xinit
- Random X utilities such as xclock, xeyes, and xterm to play with

Install on Ubuntu/Debian
```
sudo apt-get install libx11-dev libgoogle-glog-dev xserver-xephyr xinit x11-apps xterm
```

Useful links  
[How X window managers work](https://jichu4n.com/posts/how-x-window-managers-work-and-how-to-write-one-part-i/)
