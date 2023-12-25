Zion Image Stitcher
===

# Run example package

## 1. Download the example package *CPT_release/zion_stitcher_improc.tar.gz* from [FTP site](http://ftp.centrilliontech.com.twh.com.tw/) 

## 2. Extract the file, the content should be:
```
zion_stitch_improc/
    C018_2017_11_30_18_14_23/ # image data
    marker.tsv # zion marker pattern
    img_list.tsv # the image path list to image data
```

## 3. Download the CPT latest release package from [FTP site](http://ftp.centrilliontech.com.tw/)
* For windows, download CPT_release/CPT_binary_win.7z
* For linux, download CPT_release/CPT_binary_linux.tar.gz

## 4. Run the program
```
CPT/bin/zion_stitcher -x 2 -y 2 -v 528 -z 1201 -l img_list.tsv -m marker.tsv -o output.tiff
```
The result file is output.tiff.

### Parameters explained
* -x -y specify the row and col number of the image grid to be stitched.
For summit reader generated result, it's always 2 for row and col stitch number.

* -v -z are the theroetical values of image vertical and horizontal FOV offset when stitching, respectively.
For summit reader generated result, they are always -v 528 and -z 1201.

* -m is the marker pattern file given by Centrillion, each type of chip has a corresponding marker pattern. 

* -l is the image file list and the marker pattern file.
* -o specifies the output file.

# Usage
```
Allowed options:
  -h [ --help ]         shows help message
  -l [ --im_list ] arg  image list going to be stitiched, the order must follow
                        Z shape walk.
  -x [ --x_axis ] arg   number of images along x axis
  -y [ --y_axis ] arg   number of images along y axis
  -v [ --ver_off ] arg  vertical offset between images
  -z [ --hor_off ] arg  horizontal offset between images
  -o [ --output ] arg   output image path
  -m [ --marker ] arg   marker image path
```