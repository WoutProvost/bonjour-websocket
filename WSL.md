```
wsl --set-version Ubuntu-18.04 1

sudo apt install build-essential cmake x11-apps
echo "export DISPLAY=$(route.exe print | grep 0.0.0.0 | head -1 | awk '{print $4}'):0.0" >> ~/.bashrc
. ~/.bashrc
xeyes

https://github.com/WoutProvost/bonjour-websocket
```
