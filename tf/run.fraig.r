#! /bin/csh
if ($#argv == 0) then
  echo "Missing aag file. Using -- run.fraig xx (for simxx.aag)"; exit 1
endif

set design=sim$1.aag
if (! -e $design) then
   echo "$design does not exists" ; exit 1
endif

set dofile=do.fraig
rm -f $dofile
echo "cirr $design" > $dofile
echo "cirp" >> $dofile
echo "cirp -pi" >> $dofile
echo "cirp -po" >> $dofile
echo "cirp -n" >> $dofile
echo "cirp -fl" >> $dofile
echo "usage" >> $dofile
echo "ciropt" >> $dofile
echo "cirp" >> $dofile
echo "usage" >> $dofile
echo "cirsw" >> $dofile
echo "cirp" >> $dofile
echo "usage" >> $dofile
echo "cirstrash" >> $dofile
echo "usage" >> $dofile
echo "cirsw" >> $dofile
echo "cirp" >> $dofile
echo "usage" >> $dofile
#echo "cirsim -random -out .rsim$1.log" >> $dofile
echo "cirsim -random" >> $dofile
echo "usage" >> $dofile
echo "cirp -fec" >> $dofile
echo "usage" >> $dofile
echo "cirfraig" >> $dofile
echo "cirp" >> $dofile
echo "usage" >> $dofile
echo "ciropt" >> $dofile
echo "cirp" >> $dofile
echo "usage" >> $dofile
echo "cirsw" >> $dofile
echo "cirp" >> $dofile
echo "usage" >> $dofile
echo "cirp -n" >> $dofile
echo "cirp -fl" >> $dofile
echo "cirp -fec" >> $dofile
echo "usage" >> $dofile
echo "q -f" >> $dofile
../fraigr -f $dofile
