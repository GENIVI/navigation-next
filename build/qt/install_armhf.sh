tar -xvf 4.8.3.armhf.tar.gz -C /usr/local
sudo rm /usr/local/QT/4.8.3.armhf/plugins/phonon_backend/*
sudo tar zxvf libphonon.tar.gz -C  /usr/local/QT/4.8.3.armhf/lib
sudo tar zxvf phonon-backend.tar.gz -C  /usr/local/QT/4.8.3.armhf/plugins
sudo tar zxvf vlc.tar.gz -C /usr
printf "Install has completed.\n"
