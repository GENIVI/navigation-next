export QT_QPA_EGLFS_KMS_CONFIG=kms_config.json
export QT_XCB_GL_INTEGRATION=xcb_egl
export QT_IM_MODULE=qtvirtualkeyboard
export SVGA_VGPU10=0
export PATH=/opt/Qt5.8.0/bin:$PATH
export LD_LIBRARY_PATH=/opt/locationstudio/libs/png:/opt/locationstudio/libs/ncdb:$LD_LIBRARY_PATH
dir=$(pwd)
cd $dir

#run compositor
cd ../compositor/startScript
sh -x start.sh &
sleep 0.5
#run clusterView
cd ../../clusterView/startScript
sh -x start.sh &
sleep 0.5
#run externalApplication
cd ../../externalApplication/startScript
sh -x start.sh 


