# BK - tiny backup utility

Small, fast backup utility for large chunks of files.

...because `rsync` is too complex for simple backups.

## Usage

**This program doesn't check if files are different**.
It only checks if files exists,
then copies missing files and removes old ones.


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
