
file="/tmp/datainfo.txt"

wget http://www.clickfinder.de/daten/onlinedata/cftv520/datainfo.txt -q -O $file

cat $file | mac2unix | while read line; do 

   i=$[ $i + 1 ]

   if [ $i -gt 3 ]; then 

     if [ $[ $i % 2 ] == 0 ]; then 
        chan=$line 
     else 
        echo "tvm:$line    // $chan"
     fi  

   fi;

done | sort -t: -k2 -n
