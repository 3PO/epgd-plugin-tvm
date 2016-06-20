
file="/tmp/datainfo.txt"

wget http://wwwa.tvmovie.de/static/tvghost/html/onlinedata/cftv520/datainfo.txt -q -O $file

cat $file | dos2unix -l | dos2unix | while read line; do 

   i=$[ $i + 1 ]

   if [ $i -gt 3 ]; then 

     if [ $[ $i % 2 ] == 0 ]; then 
        chan=$line 
     else 
        echo "tvm:$line    // $chan"
     fi  

   fi;

done | sort
