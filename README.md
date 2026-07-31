# BK - tiny backup utility

This is a small backup utility for large chunks of files.

I figured `rsync` was too large for my use case,
and that it runs very slow on USBs. That's why I made my own.

## Status

**It doesn't check if files are different**.
That's because it's directed towards the preservation of gallery/music files.
A new flag may be added soon.

## Usage

```sh
bk origin destination

# examples:
bk Documents /media/Documents

# those two do the same thing
bk Music /media/Music
bk Music /media/
```

You have a small confirmation prompt before applying changes,
so don't worry about messing up the command.

## Installation

```sh
make install
```
