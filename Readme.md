# Install dependencies (Ubuntu/Debian)

```bash
sudo apt install libgtk-3-dev valac libgeotiff-dev gobject-introspection libgirepository1.0-dev libpulse-dev
```

# Build

```bash
mkdir build
cd build
cmake .. && make
```

# Demos

## Tiles demo

```bash
../libs/libgpstapler/bin/staplertest
```
![Tiles demo](/images/stapler.png)


## Sound demo

```bash
../libs/libgpcore/bin/rumorista
```

## Text transfer client-server demo (chat-like)

```bash
../libs/libgpcore/bin/landle_server &
../libs/libgpcore/bin/landle_client
```
![Chat demo](/images/landle.png)

## Menu tree widget demo

```bash
../libs/libgpcore/bin/menu_tree_test
```

# Run tests

```bash
make test
```

