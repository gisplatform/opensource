# Install dependencies (Ubuntu/Debian)

 sudo apt install libgtk-3-dev valac libgeotiff-dev gobject-introspection libgirepository1.0-dev libpulse-dev

# Build

 mkdir build
 cd build
 cmake .. && make

# Demos

## Tiles demo

 ../libs/libgpstapler/bin/staplertest

## Sound demo

 ../libs/libgpcore/bin/rumorista

## Text transfer client-server demo (chat-like)

 ../libs/libgpcore/bin/landle_server &

## Menu tree widget demo

 ../libs/libgpcore/bin/menu_tree_test

# Run tests

 make test

