from userscript import *

# the entry point
def Run():
    # load common material and use 'day' material
    nbas_load_common_material('day', 'common_material_day.nbm')
    nbas_set_current_common_material('day')

    # Set camera height to 2000m, tilt angle to 0 degree, rotate angle to 0 degree
    nbas_camera_set_attitude(2000.0, 0.0, 0.0)
    
    # load all tiles under the folder '../tiles/iphonedata'
    nbas_load_tile('1','0562013083AREA.nbm')
    nbas_load_tile('2','0562013083ROAD.nbm')
    nbas_load_tile('3','0562013084AREA.nbm')
    nbas_load_tile('4','0562013084ROAD.nbm', True)
    
    nbas_wait_for_seconds(1)
    nbas_snap_screenshot('nbgm_case1.png')
    nbas_wait_for_seconds(2)
    nbas_camera_tilt(33.0, 3)
    nbas_wait_for_seconds(2)
    nbas_camera_zoom(0.7, 3)

    nbas_wait_for_seconds(2)
    for i in range(4):
        nbas_camera_rotate(90.0, 3.0)
        nbas_wait_for_seconds(1)
