**[__A Guideline for Manipulating CEN File by using HDFView__](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/uploads/0f93874e753081a99ac28471e96bf6ee/A_Guideline_for_Manipulating_CEN_File_by_using_HDFView.pptx)**

### Table of Contents
1. [**Installation**](#installation)
2. [**Open File**](#open-file) 
3. [**Modify Attributes**](#modify-attributes) 
4. [**Modify Datasets**](#modify-datasets) 

### Installation
***

#### __Windows__ 

1. Dowload HDFView+Object 2.13
  * Download [__HDFView+Object 2.13__](http://support.hdfgroup.org/ftp/HDF5/hdf-java/current/bin/HDFView-2.13-win32.zip) for 32bit version of Windows
  * Download [__HDFView+Object 2.13__](http://support.hdfgroup.org/ftp/HDF5/hdf-java/current/bin/HDFView-2.13-win64.zip) for 64bit version of Windows
2. Extract the "__HDFView-2.13.cbz__" file
3. Install "__HDFView-2.13.msi__"
4. Execute the "__Star Menu -> Programs -> HDFView-2.13.0 -> HDFView-2.13.0__"

#### __Linux__

###### __Before the installation, you must be sure there is Xwindows service in your computer; if you are using ssh protocol to connect a Linux server__
###### If you don't have Xwindows services, please follow [__Xwindows__](XwindowsGuide) guide to setup your Xwindows environment

1. Dowload [__HDFView+Object 2.13__](http://support.hdfgroup.org/ftp/HDF5/hdf-java/current/bin/HDFView-2.13.0-centos6-x64.tar.gz) via `wget`
2. Extract the "__HDFView-2.13.0-centos6-x64.tar.gz__" file via `tar -zxvf HDFView-2.13.0-centos6-x64.tar.gz`
3. Execute "__HDFView-2.13.0-Linux.sh__" via `sh HDFView-2.13.0-Linux.sh`
4. Read and agree with "__License Terms__", and the installation will begin
5. Execute "__hdfview__" via `./HDFView-2.13.0-Linux/HDFView/2.13.0/hdfview.sh`
6. The HDFView will show up with Xwindows 

#### __MacOS__

1. Dowload [__HDFView+Object 2.13__](http://support.hdfgroup.org/ftp/HDF5/hdf-java/current/bin/HDFView-2.13.dmg)
2. Execute the dmg file of "__HDFView-2.13.dmg__"
3. Copy the "__HDFView-2.13__" to the system application folder
4. Execute the "__HDFView-2.13__" from application folder or lunch panel
  * If there is any security issue, please check "__trust__" to continue

### Open File
***

![OpenCenFile](http://gitlab.centrilliontech.com.tw:10088/centrillion/CPT/uploads/3f786d64b6f0e8c2c7d60b5a5b4bd9cf/OpenCenFile.PNG)

### Modify Attributes
***

![ModifyAttribute](http://gitlab.centrilliontech.com.tw:10088/centrillion/CPT/uploads/3f0f9bedf3b38cfc810116f5fdf48cb6/ModifyAttribute.PNG)

### Modify Datasets
***
1. [**Open Dataset**](#1-open-dataset)
2. [**Delete Dataset**](#2-delete-dataset) 
3. [**Recreate a Dataset**](#3-recreate-a-dataset) 
4. [**Modify the Schema**](#4-modify-the-schema) 
5. [**Result of Recreated Dataset**](#5-result-of-recreated-dataset)
6. [**Import External Data Values**](#6-import-external-data-values)
7. [**Result of the Dataset**](#7-result-of-the-dataset)

#### 1 Open Dataset
![ModifyDatasets1](http://gitlab.centrilliontech.com.tw:10088/centrillion/CPT/uploads/db4b4505bf77ad94490d622d2b43aca3/ModifyDatasets1.PNG)

#### 2 Delete Dataset
![ModifyDatasets2](http://gitlab.centrilliontech.com.tw:10088/centrillion/CPT/uploads/b425338912354c563695fde0c0fd4f85/ModifyDatasets2.PNG)

#### 3 Recreate a Dataset
![ModifyDatasets3](http://gitlab.centrilliontech.com.tw:10088/centrillion/CPT/uploads/b6ad556332bfaf1656590f58395e1b35/ModifyDatasets3.PNG)

#### 4 Modify the Schema
![ModifyDatasets4](http://gitlab.centrilliontech.com.tw:10088/centrillion/CPT/uploads/c587905be55827b8f5cdf155ae828ede/ModifyDatasets4.PNG)

#### 5 Result of Recreated Dataset
![ModifyDatasets5](http://gitlab.centrilliontech.com.tw:10088/centrillion/CPT/uploads/a81f97f668ee5d555cd0af9f0248ae44/ModifyDatasets5.PNG)

#### 6 Import External Data Values
![ModifyDatasets6](http://gitlab.centrilliontech.com.tw:10088/centrillion/CPT/uploads/aef42b27b863771ad384ec5d6da435cf/ModifyDatasets6.PNG)

#### 7 Result of the Dataset
![ModifyDatasets7](http://gitlab.centrilliontech.com.tw:10088/centrillion/CPT/uploads/5a7be8ca74068a78490b3815d2b3bad4/ModifyDatasets7.PNG)