set PATH=C:\Python24;%PATH%
@rem Compile the tpslib.txt file into tesla.tps
python ..\tools\libcom.py -d . -o tesla.tpl tpslib.txt
@rem Create a version of the file with the hex id name
python .\rename_tpslib.py tesla.tpl
copy tesla.tpl .\compiled\tesla.tpl

pause