### Function Documentation 
#### **Module** affy2hdf5
* **file_convert_to ( affycel, hdf5 )**

    Given the paths to the Affy CEL input file and CEN hdf5 output file and this function will convert the .cel file format to hdf5.
    * **Parameters**
        * **affycel** The .cel file path
        * **hdf5** The .cen file path
        * **return** *none*
    * **Example Usage**

    ```python
    import py3affy2hdf5
    py3affy2hdf5.file_convert_to( 
        "example/GSM2066668_206-001_CHB.CEL"
        , "example/test_cenfile.cen" 
    )
    ```    

---

#### **Module** cadtool
* **json2cad_file_convert_to ( input_file_path, output_file_path )**

    Given the input JSON file path and output CAD file path, This function will convert the JSON file which is the human-readable text file to the CAD binary data.

    Note that the JSON file format should follow [this schema] ( CAD-format-spec ).
    * **Parameters**
        * **input_file_path** The input (.json) file path.
        * **output_file_path** The output (.cad) file path.
        * **return** *none*
    * **Example Usage**

    ```python
    import py3cadtool

    py3cadtool.json2cad_file_convert_to(
        "example/example_out.json"
        , "example/example_out2.cad"
    );
    ```    

* **cad2json_file_convert_to ( input_file_path, output_file_path )**
    
    Given the input CAD file path and output JSON file path. This function will convert the CAD file which is binary data to the human-readable JSON file.

    Note that the JSON file format will follow [this schema] ( CAD-format-spec ).
    * **Parameters**
        * **input_file_path** The input .cad file path.
        * **output_file_path** The output .json file path
        * **return** *none*
    * **Example Usage**

    ```python
    import py3cadtool

    py3cadtool.cad2json_file_convert_to(
        "example/example_out.cad"
        , "example/example_out.json"
    );
    ```


* **file_convert_to ( input_file_path, output_file_path )**
    
    This function defined a wrapper of 2 functions, json2cad_file_convert_to and cad2json_file_convert_to.

    It provides a string parameter to switch the user required conversion.
    * **Parameters**
        * **input_file_path** The input (.json) file path.
        * **output_file_path** The output (.cad) file path.
        * **mode** The string used to swtich internal function, can be json2cad or cad2json
        * **return** *none*
    * **Example Usage**

    ```python
    import py3cadtool

    py3cadtool.file_convert_to(
        "example/example_out.cad"
        , "example/example_out2.json"
        , "cad2json"
    );
    ```

---


#### **Module** cdf2cad 
* **file_convert_to ( input, output, probe_tab, annot_csv )**
   
    Given the input CDF, output CAD file path, probe table (.probe_tab) and annotation file (.annot.csv), and the function will convert the CDF which is Affymetrix spec to CAD which is Centrillion spec.

    The Affymetrix file format description can be found at: [HERE](http://media.affymetrix.com/support/developer/powertools/changelog/FILE-FORMATS.html)
    
    Note that the CDF is Axiom platform spec.
    * **Parameters**
        * **input** The input (.cdf) file path.
        * **output** The output (.cad) file path.
        * **probe_tab** The probe table (.prob_tab) file path.
        * **annot_csv** The annotation (.annot.csv) file path.
        * **return** *none* 
    * **Example Usage**

    ```python
    import py3cdf2cad

    py3cdf2cad.file_convert_to(
        "example/example.cdf"
        , "example/example_out.cad"
        , "example/example.probe_tab"
        , "example/example.annot.csv"
    );
    ```

---

#### **Module** hdf5_schema_builder
* **build( schema_json, result_cenfile_hdf5 )**

    This function read a human-readable schema description JSON file and generate the Centrillion sample file (.cen).

    Note that the JSON file format will follow [this schema] ( DocumentForHDF5Builder ).
    * **Parameters**
        * **schema_json** the schema description (.json) file path
        * **result_cenfile_hdf5** the result (.cen) file path
        * **return** *none* 
    * **Example Usage**

    ```python
    import py3cdf2cad

    py3hdf5_schema_builder.build(
        "example/hdf5_schema_example.json"
        , "example/test_cenfile.cen"
    );
    ```


---

#### **ALL MODULE NAMES NEED TO ADD py2[3] PREFIX FOR python2[3] VERSION**
