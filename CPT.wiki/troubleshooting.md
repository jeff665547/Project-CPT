# ABI issue
When you execute the binary in our portable binary package, there are probably some library version issues need to be resolve.

The error message may looks like: 
```
./pipeline_builder: /lib/x86_64-linux-gnu/libc.so.6: version `GLIBC_2.17' not found (required by ./pipeline_builder)
./pipeline_builder: /lib/x86_64-linux-gnu/libc.so.6: version `GLIBC_2.18' not found (required by /home/CPT/bin/../lib/libstdc++.so.6)
./pipeline_builder: /lib/x86_64-linux-gnu/libc.so.6: version `GLIBC_2.17' not found (required by /home/CPT/bin/../lib/libstdc++.so.6)
```
( The GLIBC_X.XX could be any library, such as GLIBCXX, GOMP etc. )

This problem is cause by the library binary interface version mismatched to the requirement.

The following libraries were known to cause this issue:

* GLIBC_2.18 ( GNU standard C library )
* GLIBCXX_3.4.22  ( GNU standard C++ library )
* GOMP_4.0 ( GNU OpenMP library )

To solve this issue, please update your library to related version.

If the libraries you met the issue are not on the list, please [submit an issue](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/issues/new?issue%5Bassignee_id%5D=&issue%5Bmilestone_id%5D=) to our issue list.
