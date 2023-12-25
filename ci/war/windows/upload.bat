7z a CPT_binary_win.7z install
7z rn CPT_binary_win.7z install CPT
echo "compress finished start upload file"
winscp /ini=nul /script=upload.txt
echo "upload file done"
