# xwinsize

xwinsize is a simple program that prints a window's position and size. It also has a monitor option that keep listening the window and prints the window's position and size everytime the window is moved/resized.

## Installation

```sh
make
sudo make install # Default install location is /usr/local
# make PREFIX=<directory> install # If you want to install at another directory
```

## Usage
```
usage: xwinsize <window option> [options ...]
You must specify one of the following window options:
    --name <window_name>: Select the window with the specified name
    --id <window_id>: Select the window with the specified id
    --root: Select the root window

The following options are optional:
    --monitor|-m: Keep monitoring the window size forever, printing it everytime it changes
    --help|-h: Print the command line usage
```
