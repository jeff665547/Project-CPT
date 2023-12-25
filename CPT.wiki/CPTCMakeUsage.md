CPT CMake Usage
===
* Build from pre-build libraries ( default )

    ```cmake .. -DDEFAULT_EP_SRC="PRE_BUILD"```
    
    This command will build the CPT project from the pre-build upstream binary libraries.

* Use remote third party source codes

    ```cmake .. -DDEFAULT_EP_SRC="REMOTE_SOURCE"```
    
    This command will let CMake use the remote source code to build the upstream dependent libraries.

* Build with source code release

    ```cmake .. -DSOURCE_CODE_RELEASE=ON```

* Build without source release
    
    ```cmake .. -DSOURCE_CODE_RELEASE=OFF```

The default setting of SOURCE_CODE_RELEASE is ON

<!--

* Use local system third party library 

    ``` cmake .. -DDEFAULT_EP_SRC_EP_SRC="SYSTEM"```
    
    This command will let CMake try to find upstream dependent libraries on the local system if the required libraries are not found, then the build configures process will abort.

-->
