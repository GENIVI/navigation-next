from userscript import *

def walk_along_some_tiles():
    nbas_camera_set_viewport_by_tilename('T_B3D_1_NBM_23_9914_12121')
    nbas_wait_for_seconds(2)
    nbas_camera_zoom(0.12, 3)
    nbas_camera_tilt(50.0, 3)
    nbas_wait_for_seconds(2)
    test_tiles = ['T_B3D_1_NBM_23_9914_12121',\
                  'T_B3D_1_NBM_23_9915_12121',\
                  'T_B3D_1_NBM_23_9916_12120',\
                  'T_B3D_1_NBM_23_9915_12119']
    nbas_camera_make_linear_animation_by_tilenames(test_tiles, 16)

# the entry point
def Run():
    # load common material and use 'day' material
    nbas_load_common_material('day', 'T_DMAT_1_NBM_23_-1_-1')
    nbas_set_current_common_material('day')

    # load all tiles under the folder '../tiles/iphonedata'
    nbas_load_tile_folder('D:\\nbgm_replayer\\testcases\\mobius\\Input_EU\\new_area_Island_Island\\Line0_ZL13')
    nbas_camera_set_viewport(50.801971,5.700012)
    #nbas_camera_set_viewport_by_tilename('D:\\nbgm_replayer\\testcases\\mobius\\Input_EU\\new_area_Island_Island\\Line0_ZL2\\000020000102RAST.nbm')
    nbas_camera_set_attitude(CalcHeightByZoomLevel(13),0.0, 0.0)
    #nbas_set_horizon_distance()
    # Set camera height to 10000m, tilt angle to 0 degree, rotate angle to 40 degree
    #nbas_camera_set_attitude(10000.0, 0.0, 40.0)

    # walk along some tiles
    #walk_along_some_tiles()
    #nbas_wait_for_seconds(2)

    # sanp screenshots on 4 directions
    #nbas_camera_tilt(22.0, 3)
    #nbas_wait_for_seconds(2)
    #for i in xrange(4):
    #    nbas_camera_rotate(90.0, 3)
    #    # wait for 3 seconds, incaste of view is still being updated
    #    nbas_wait_for_seconds(3)
    #    nbas_snap_screenshot('nbgm_demo_%d.png'%i)