# bonjour-websocket
A [Bonjour](https://developer.apple.com/bonjour/) multicast DNS example using [QMdnsEngine](https://github.com/nitroshare/qmdnsengine) and [Qt WebSockets](https://doc.qt.io/qt-5/qtwebsockets-index.html).

### Dependencies
```bash
sudo apt install qt5-default libqt5websockets5-dev
```

### Sources
```bash
git clone https://github.com/WoutProvost/bonjour-websocket
```

### Building
```bash
cd bonjour-websocket
mkdir build
cd build
cmake ..
make
```

### Running
```bash
./server
./client
```