#!/bin/bash
pftp -n 60.250.196.14 << EOF
quote USER cen.us
quote PASS xyz@cen
binary
cd CPT_release
put download.php
put LATEST_VERSION
quit
EOF
echo "upload file done"
