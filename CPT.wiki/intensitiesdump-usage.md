# Image Process Example

# How to run the program
Usage :

```
-h [ --help ]                    show help message
-c [ --config ] arg              config
-o [ --output ] arg              output
-i [ --image_args ] arg          input images
-f [ --output_format ] arg (=il) output format (cel/cen/il)[il]
-d [ --debug ] arg (=0)          turn on debug mode [off]
```

The simplest usage can be like as follows:
```
intensities_dump -i parameter.json -c option.json -o out.tsv
```

# Detail

## **--config option ( option.json )**
This file specifies the parameters of this image process pipeline including information such as image size, marker pattern, stitch position etc.
In general, each type of chips has a predefined option.json provide by Centrillion, and it should not be modified unless necessary.

## **--image\_args option ( parameter.json )**
This file specifies the paths and related information of the images desired to process.

The file should follow the format below:

```
{
    "run_name" : "xxx",
    "images" : {
        "0_0" : {
            "x_marker_num" : 2,
            "y_marker_num" : 2,
            "path" : "20170330_1NGI690-01C27H_20X_SampA_ExposureTest/1NGI690-01C27H_SampA20X_f610_TG_exp6_FOV0_0.tif",
            "grid_img_path" : "20170330_1NGI690-01C27H_20X_SampA_ExposureTest/1NGI690-01C27H_SampA20X_f610_TW_exp0p5_FOV0_0.tif",
            "x_axis_direction" : "|",
            "origin_position" : "LT"
        },
        ...
    }
}
```


### **run\_name**

This is a label for this task.

### **images**

This is the image list going to be processed. Every node in the list require 5 items :

* **image position ("0_0")**

The index of the node, which specifies the stitch position of the images.

* **marker number ("x_marker_num","y_marker_num")**

The marker numbers along x and y axises of the image. For Denali 20X image, it should be 2 * 2.

* **image path ("path")**

The path to image.

* **grid file path ("grid_image_path")**

The mesh grid image under bright light.

* **image rotation information ( "origin\_position", "x\_axis\_direction" )**

This pair of parameters specifies the orientation of the chip when scanning. 
The *origin_positin* can be "LT" or "LB", which represents the origin position (0,0) is at the "top left" or "bottom left" of the first image.
The *x_axis_direction* can be "\_" (horizontal) or "|" (vertical), since the chip might be rotated by 90 degree while loading. In general, if the chip is loaded correctedly, this paramter should be always "\_".

## **--output\_format**

This option specifies the output format of intensity data. There are 3 format can be specified now:

### **il**
The .tsv format which include position and intensity mean value, 

### **cen** 
Centrillion chip sample format

### **cel**
Affymetrix chip sample format.

# Output format details

## intensities list ( --output_format il )

This is the plain text output and formatted as tab separated value.

* x, y 

The first two column(x,y) is probe position. 

* mean

The third column is the extracted intensity mean. 

* overlap_detail 

( deprecated )

* pixel_detail

The selected pixel used to process the intensities (mean column). 

* cv 

The CV value used in auto margin detection. 

## CEN file

See [here](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/wikis/CEN-file) and [here](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/blob/master/doc/hdf5_schema_details.json) for detail.

## CEL file

See Affymetrix offical site for more detail.


# Full Example

## Denali chip example

### 1. Download example package from [FTP site](http://ftp.centrilliontech.com.tw)

* The file is at **CPT_release/example_improc.tar.gz**

### 2. Extract the file, the content would be

```
example_improc/
    20170621_1NGI690C22-26D/    # All example images are here
    README.md                   # A simple document
    config_data/                # The program configure used data
        am1.mk                  # am*.mk is the candidate chip marker pattern
        am2.mk                  
        am3.mk
        am4.mk
        clariom_s_xy.tsv        # ClariomS compatible image location file ( not used here )
        file_3_xy.tsv           # Denali image location file ( used to stitching )
        probe_info.tsv          # The probe id mapping
    option.json                 # The program configure file
    parameter.json              # The program image parameter file
    parameter.json.php          # The image parameter file generator
```

### 3. Dowload the CPT release package from [FTP site](http://ftp.centrillion.com.tw)

### 4. Run the program

```
~/example_improc$ ./CPT/bin/intensities_dump -c option.json -i parameter.json -o output.tsv
```

The result will be generate in output.tsv


## Zion chip example

### 1. Download example package from [FTP site](http://ftp.centrilliontech.com.tw)

* The file is at **CPT_release/example_zion.tar.gz**

### 2. Extract the file, the content will be

```
example_zion/
    C018_2017_11_30_18_14_23/    # All example images are here
    README.md                   # A simple document
    config_data/                # The program configure used data
        am1.mk                  # am*.mk is the candidate chip marker pattern
        am2.mk                  
        am3.mk
        am4.mk
        clariom_s_xy.tsv        # ClariomS compatible image location file ( not used here )
        file_3_xy.tsv           # Denali image location file ( used to stitching )
        probe_info.tsv          # The probe id mapping
    option.json                 # The program configure file
    parameter.json              # The program image parameter file
    parameter.json.php          # The image parameter file generator
```

### 3. Dowload the CPT release package from [FTP site](http://ftp.centrillion.com.tw)

### 4. Run the program

```
~/example_zion$ ./CPT/bin/intensities_dump -c option.json -i parameter.json -o output.tsv
```

The result will be generate in output.tsv


# Full workflow to run a set of chip images

In this section, we describe the full workflow from a example directory to run a new sample scanned from reader, here we start from the aforementioned ZION example directory. 

## Step 1. Edit the parameter.json

The content of parameter.json is like this:

```
{
    "run_name": "sample", 
    "images": {
        "0_0" : {
            "x_marker_num":3,
            "y_marker_num":3,
            "path":"C018_2017_11_30_18_14_23\/0-0-2.tiff",
            "grid_img_path":"",
            "x_axis_direction":"_",
            "origin_position":"LT"
        },
        "1_0" : {
            "x_marker_num":3,
            "y_marker_num":3,
            "path":"C018_2017_11_30_18_14_23\/1-0-2.tiff",
            "grid_img_path":"",
            "x_axis_direction":"_",
            "origin_position":"LT"
        },
        "0_1" : {
            "x_marker_num":3,
            "y_marker_num":3,
            "path":"C018_2017_11_30_18_14_23\/0-1-2.tiff",
            "grid_img_path":"",
            "x_axis_direction":"_",
            "origin_position":"LT"
        },
        "1_1" : {
            "x_marker_num":3,
            "y_marker_num":3,
            "path":"C018_2017_11_30_18_14_23\/1-1-2.tiff",
            "grid_img_path":"",
            "x_axis_direction":"_",
            "origin_position":"LT"
        }
    }
    , "output_dir" : "output/C018_2017_11_30_18_14_23/"
}
```

Please modify **path** properties in every ```images.<x>_<y>``` nodes to the images you desired to process, for example if my new scanned image output is in directory ```D:\experiment\DEMO5-800-TW-6\C001_2018_01_30_11_2_42``` then the parameter.json should be:
```
{
    "run_name": "sample", 
    "images": {
        "0_0" : {
            "x_marker_num":3,
            "y_marker_num":3,
            "path":"D:\/experiment\/DEMO5-800-TW-6\/C001_2018_01_30_11_2_42\/0-0-2.tiff",
            "grid_img_path":"",
            "x_axis_direction":"_",
            "origin_position":"LT"
        },
        "1_0" : {
            "x_marker_num":3,
            "y_marker_num":3,
            "path":"D:\/experiment\/DEMO5-800-TW-6\/C001_2018_01_30_11_2_42\/1-0-2.tiff",
            "grid_img_path":"",
            "x_axis_direction":"_",
            "origin_position":"LT"
        },
        "0_1" : {
            "x_marker_num":3,
            "y_marker_num":3,
            "path":"D:\/experiment\/DEMO5-800-TW-6\/C001_2018_01_30_11_2_42\/0-1-2.tiff",
            "grid_img_path":"",
            "x_axis_direction":"_",
            "origin_position":"LT"
        },
        "1_1" : {
            "x_marker_num":3,
            "y_marker_num":3,
            "path":"D:\/experiment\/DEMO5-800-TW-6\/C001_2018_01_30_11_2_42\/1-1-2.tiff",
            "grid_img_path":"",
            "x_axis_direction":"_",
            "origin_position":"LT"
        }
    }
    , "output_dir" : "output\/C001_2018_01_30_11_2_42"
}
```
Note that the file name is not necessary but position label is considered, the "0_0" section should put the chip's left top image, and the "0_1" section should put right top etc.

## Step 2. Run the program

Just like things what you do in example, type the command:

```intensities_dump -c option.json -i parameter.json -o output.tsv```