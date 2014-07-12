set DATESTAMP=%DATE:~10,4%_%DATE:~4,2%_%DATE:~7,2%
set TIMESTAMP=%TIME:~0,2%_%TIME:~3,2%_%TIME:~6,2%
set DATEANDTIME=%DATESTAMP%_%TIMESTAMP%
cd klImageCore 
git add --all .
git commit -m "%DATEANDTIME%"
git push
cd ../
cd klMatrixCore 
git add --all .
git commit -m "%DATEANDTIME%"
git push
cd ../
cd ProjectFiles 
git add --all .
git commit -m "%DATEANDTIME%"
git push
cd ../
cd Packages 
git add --all .
git commit -m "%DATEANDTIME%"
git push
cd ../