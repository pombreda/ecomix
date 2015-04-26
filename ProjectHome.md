## Ecomix ##
Image viewer from archives (tar, zip, rar, etc) with minimalist GUI. Use Evas and Ecore from EFL (http://trac.enlightenment.org/e/wiki) for Gui. Libarchive or external programs (unar, lha, 7z) to extract data.

Image are read from memory using fmemopen (glibc only) and duplicate code from Evas or using libGraphicsMagick/libGraphicsMagickWand.

## Dependances ##
  * libevas
  * libecore
  * libecore-evas
  * libecore-file
  * libarchive
  * libmagic

and libGraphicsMagic if using libGraphicsMagick or libGraphicsMagickWand branches.

## External Programs ##
We check if they are in the PATH at runtime and we can use :
  * unrar
  * lha
  * 7z (7z can be a remplacement for lha)

## Usage ##
```
$ ecomix -h
A mimimalist image viewer from archive (tar, zip, cpio, rar, 7z) without extracting file to disk.

Usage: ecomix [-h] [-bg (r,g,b|image)] [-g wxh] [-v] file|dir

Options:
-h or --help or -? or --?	Show this help
-bg (r,g,b|image)               Set background color or image
-g widthxheight                 Set window size
-v                              verbose

Basic keys:
q         quit
s         show/hide filename / position
r or R    rotate image by 90° clockwise or anti-clockwise
f         (un)fullscreen
v         scale image to max vertical
h         scale image to max horizontal
n         original image size
left      go to prev file in dir
right     go to next file in dir
up        prev image or prev file if first image or up image if top not seen
down      next image or next file if last image or down image if bottom not seen
Home      go to fisrt of archive or top of image or first file in dir
End       go to end of archive or bottom of image or last file in dir
PgDown    skip 10 image/file backwrad
PgRight   skip 10 image/file forward

```