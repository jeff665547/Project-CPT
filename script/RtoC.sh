# shell script to convert rows to columns from a file
# Usage: RtoC.sh <filename>

(head -n1 && tail -n1) < $1 | awk '{ for (i=1; i<=NF; i++) RtoC[i]= (i in RtoC?RtoC[i] OFS :"") $i; } END{ for (i=1; i<=NF; i++) print RtoC[i] }' | sed 's/^ //g' | sed 's/ /\t/g'