#!/bin/bash
mv install CPT
ln -s ./CPT install
tar zcvf CPT_binary_linux.tar.gz CPT
/bin/rm install
mv CPT install
echo "compress finished start upload file"
scp -P 49024 CPT_binary_linux.tar.gz ci_bot@192.168.1.11:~/
# pftp 192.168.1.11 49021 << EOF
# quote USER ci_bot
# quote PASS qsefthuk90
# binary
# put CPT_binary_linux.tar.gz
# quit
# EOF
echo "upload file done"
