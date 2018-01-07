
1.package app


blackberry-pythonpackager -package nbgm_repalyer_bb10.bar -devMode bar-descriptor.xml replay.py env.py asset/*.* ./bin/*.* ./library/*.* ./resource/*.* ./tiles/*.* ./screenshots/*.* ./user/*.*




2.install app


blackberry-deploy -installApp -device 192.168.83.90 -password qwer -package nbgm_repalyer_bb10.bar
