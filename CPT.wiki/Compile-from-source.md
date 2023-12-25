### Build From source code

#### Windows User
* Pre-installations
    * CMake
    * Python2.7
    * Python3.5

* Build Steps
    0. Download **MinGW.7z** and **CPT.7z** from [FTP site](#ftp-site)
    1. Extract **MinGW.7z** to **C:/**.
    2. Add **C:/MinGW/bin** to PATH environment variable.
    3. Extract **CPT.7z** to any path you like.
    4. Make a new directory **build** in CPT.
    5. `cd CPT/build`
    6. `cmake .. -G "MinGW Makefiles"`
    7. `make`
    8. All build result can be found at **CPT/bin/install**

#### Linux User

* Pre-install package 
    * openssl
    * libpython2.7
    * libpython3.5

    Note : These packages are usually built-in the system distribution

* Pre-install tools
    1. GNU CXX compiler ( version >= 6.0 ) 
    2. CMake

* Build Steps 
    0. Download **CPT.7z** from [FTP site](#ftp-site) and extract the file.
    1. `cd CPT && mkdir build` Make a "build" directory under CPT
    2. `cd build`
    3. `cmake ..` Configure the build state.
    4. `make` Build project
    5. All build result can be found at **CPT/bin/install**
    6. The tools manual and using guide can be found at : 
        * [cadtool](CAD-file#cadtool)
        * [cdf2cad](CAD-file#affys-cdf-to-cad)
        * [Birdseed](Birdseed)
        * [BRLMMp](BRLMMp)

---

### Example Data

The example data can be get at [FTP site](ftp-site) named **CPT_release/example.7z**.

And the quick start and format specs are at 

* [cadtool](CAD-file#cadtool)
* [cdf2cad](CAD-file#affys-cdf-to-cad)
* [Birdseed](Birdseed)
* [BRLMMp](BRLMMp)


---

### FTP site
| | |
|---|---|
| addr | 60.250.196.14 |
| port | 21 |
| directory | /CPT_release |
| id | $$$ |
|passwd| $$$ |