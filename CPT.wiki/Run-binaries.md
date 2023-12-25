## Windows
### Set environment variable

```txt
C:\Users\USER>
C:\Users\USER>path=%path%;c:\my_test\CPT\bin
C:\Users\USER>cd \my_test\
```

- Extract **CPT_binary_win.7z** 
- All executable and binary libraries are at `CPT\bin`

### Generate CAD file.

```txt
C:\my_test\CPT\example>cadtool -m json2cad -i minimal_example.json -o minimal_example.cad
C:\my_test\CPT\example>cdf2cad -i example.cdf -o cad_example.cad -p example.probe_tab -a example.annot.csv  
```
:fire:`The minimum requirement of RAM for cdf2cad.exe at Windows platform is 8G ( but < 2G at Linux platform ).`

### Run Birdseed and BRLMMp

```txt
C:\my_test\CPT_release\example>pipeline_builder -i birdseed.json -o result.json
C:\my_test\CPT_release\example>pipeline_builder -i brlmmp.json -o result.json
```

:fire:`The pipeline_builder isn't available for reading CAD format now.`


## Linux
- Browse the [FTP site](http://60.250.196.14/) and get into directory **Data/CPT_release/**.
- Download **CPT_binary_linux.tar** and extract to '/home/allen/root/wei_test/'.
- Download **genotyping_data/Axiom_GW_Hu-CHB_SNP_CAD.tar.gz** and extract to '/home/allen/root/wei_test/example/Axiom_GW_Hu-CHB_SNP_CAD.r2.cad'.
- Download **genotyping_data/raw_data_cen.tar.gz** and extract to '/home/allen/root/wei_test/raw_data_cen/'.
- Goto directory **/home/allen/root/wei_test/CPT/bin/**.


```txt
/home/allen/root/wei_test/CPT/bin$ ./pipeline_builder -i ../../example/birdseed_training_example2.json -o output2.json
```
## Convert *.CEL to *.CEN

```txt
$cd /home/allen/root/wei_test/CPT/bin/
/home/allen/root/wei_test/CPT/bin$./cdf2cad -i ../../example/Axiom_GW_Hu-CHB_SNP.r2.cdf -o ../../example/Axiom_GW_Hu-CHB_SNP.r2.cad
$ls ../../example/raw_data/*.CEL | cut -d'/' -f 4 | sed "s/CEL/CEN/g" | awk '{print "./hdf5_schema_builder ../../example/raw_data_cen/hdf5_schema_template.json ../../example/raw_data_cen/" $0}' > run
$vim run
$sh run
```
:fire:`Skip above steps if you download **Axiom_GW_Hu-CHB_SNP_CAD.tar.gz** and **raw_data_cen.tar.gz** below directly.`


## For more detail:

* [cadtool](CAD-file#cadtool)
* [cdf2cad](CAD-file#affys-cdf-to-cad)
* [Birdseed](Birdseed)
* [BRLMMp](BRLMMp)