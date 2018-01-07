from nbgmaction import *
from utilty import *
from nbgmscripttask import NBGMQuitSignal

#
#-----------------------------------------------------------------------------------------------------------------------------------------#
# TILE OPERATION
#-----------------------------------------------------------------------------------------------------------------------------------------#
#

# Load common material file which will be used to render map. 
# Material file MUST be loaded before load tile file, and use nbas_set_current_common_material () to switch common material between loaded materials.
def nbas_load_common_material(material_name, file_path):
    if not isinstance(material_name, str) or not isinstance(file_path, str):
        raise NBGMQuitSignal('nbas_load_common_material: Incorrect Parameter Type!')
    abs_path = file_path
    if not os.path.isabs(file_path):
        abs_path = os.path.join(os.path.join(os.environ['nbgm_tiles'], file_path))
    if not os.path.isfile(abs_path):
        raise NBGMQuitSignal('nbas_load_common_material: Cannot find the file %s'%abs_path)
    LoadCommonMaterial(material_name, abs_path)

# Set current common material used by rendering nbm map.
# Eg, day & night model may has different common material file. material_name shoud be loaded by nbas_load_common_material before use
def nbas_set_current_common_material(material_name):
    if not isinstance(material_name, str):
        raise NBGMQuitSignal('nbas_set_current_common_material: Incorrect Parameter Type!')
    SetCurrentCommonMaterial(material_name)

# Load a NBM tile to render
def nbas_load_tile(tile_name, file_path, set_view_port = False):
    if not isinstance(tile_name, str) or not isinstance(file_path, str):
        raise NBGMQuitSignal('nbas_load_tile: Incorrect Parameter Type!')
    abs_path = file_path
    if not os.path.isabs(file_path):
        abs_path = os.path.join(os.path.join(os.environ['nbgm_tiles'], file_path))
    if not os.path.isfile(abs_path):
        raise NBGMQuitSignal('nbas_load_tile: Cannot find the file %s'%abs_path)
    LoadTile(tile_name, abs_path, set_view_port)

# Unload a NBM tile
def nbas_unload_tile(tile_name):
    if not isinstance(tile_name, str):
        raise NBGMQuitSignal('nbas_unload_tile: Incorrect Parameter Type!')
    UnLoadTile(tile_name)

# Load all NBM tiles under the specified folder to render
def nbas_load_tile_folder(folder_path):
    if not isinstance(folder_path, str):
        raise NBGMQuitSignal('nbas_load_tile_folder: Incorrect Parameter Type!')
    abs_path = os.path.abspath(folder_path)
    if not os.path.isdir(abs_path):
        raise NBGMQuitSignal('nbas_load_tile_folder: Cannot find the folder %s'%abs_path)
    LoadTiles(FindAllFilesInFolder(abs_path))

# Unload all NBM tiles that have beed loaded
def nbas_unload_all_tiles():
    UnloadAllTiles()

#
#-----------------------------------------------------------------------------------------------------------------------------------------#
# CAMERA CONTROL AND ANIMATION
#-----------------------------------------------------------------------------------------------------------------------------------------#
#

# Set camera viewport. Generally, the viewport is same as map center.
def nbas_camera_set_viewport(latitude, longitude):
    if not isinstance(latitude, float) or not isinstance(longitude, float):
        raise NBGMQuitSignal('nbas_camera_set_viewport: Incorrect Parameter Type!')
    mx, my = LatLon2Mercator(latitude, longitude)
    SetViewCenter(mx, my)

# Set camera viewport by mercator
def nbas_camera_set_viewport_by_mercator(mercatorX, mercatorY):
    if not isinstance(mercatorX, float) or not isinstance(mercatorY, float):
        raise NBGMQuitSignal('nbas_camera_set_viewport_by_mercator: Incorrect Parameter Type!')
    SetViewCenter(mercatorX, mercatorY)

# Set camera viewport by tilename
def nbas_camera_set_viewport_by_tilename(tileName):
    if not isinstance(tileName, str):
        raise NBGMQuitSignal('nbas_camera_set_viewport_by_tilename: Incorrect Parameter Type!')
    info = NBMTileInfo()
    ret = GetNBMTileInfo(tileName, info)
    if not ret:
        raise NBGMQuitSignal('nbas_camera_set_viewport_by_tilename: GetNBMTileInfo from %s failed!', tileName)
    SetViewPortToTileCenter(info.x, info.y, info.z)

# Set camera viewport by tile center
def nbas_camera_set_viewport_by_tilecenter(tileX, tileY, tileZ):
    if not isinstance(tileX, (float,int)) or not isinstance(tileY, (float,int)) or not isinstance(tileZ, (float,int)):
        raise NBGMQuitSignal('nbas_camera_set_viewport_by_tilecenter: Incorrect Parameter Type!')
    SetViewPortToTileCenter(tileX, tileY, tileZ)

# Set all camera attribute, distance from camera to viewport in meters, tile_angle in degree, rotate_angle in degree
def nbas_camera_set_attitude(viewpointdistance, tilt_angle, rotate_angle):
    if not isinstance(viewpointdistance, (float,int)) or not isinstance(tilt_angle, (float,int)) or not isinstance(rotate_angle, (float,int)):
        raise NBGMQuitSignal('nbas_camera_set_attitude: Incorrect Parameter Type!')
    if viewpointdistance <= 0 or tilt_angle < 0 or tilt_angle > 90:
        raise NBGMQuitSignal('nbas_camera_set_attitude: Invalid Parameter Values!')
    SetViewPointDistance(viewpointdistance)
    SetTiltAngle(tilt_angle)
    SetRotateAngle(rotate_angle)

# Zoom camera distance, new_distance = distance*scale, if repeat == -1, means animation is an endless animation
def nbas_camera_zoom(scale, duration, repeat = 1):
    if not isinstance(scale, (float,int)) or not isinstance(duration, (float,int)) or not isinstance(repeat, int):
        raise NBGMQuitSignal('nbas_camera_zoom: Incorrect Parameter Type!')
    if duration < 1 or repeat == 0:
        raise NBGMQuitSignal('nbas_camera_zoom: Invalid Parameter Values!')
    ZoomCamera(scale, duration, repeat)

# Tile camera, new_tilt_angle = tilt_angle + delta_angle, if repeat == -1, means animation is an endless animation
def nbas_camera_tilt(delta_angle, duration, repeat = 1):
    if not isinstance(delta_angle, (float,int)) or not isinstance(duration, (float,int)) or not isinstance(repeat, int):
        raise NBGMQuitSignal('nbas_camera_tilt: Incorrect Parameter Type!')
    if duration < 1 or repeat == 0:
        raise NBGMQuitSignal('nbas_camera_tilt: Invalid Parameter Values!')
    TiltCamera(delta_angle, duration, repeat)

# Rotate camera, new_roate_angle = rotate_angle + delta_angle, if repeat == -1, means animation is an endless animation
def nbas_camera_rotate(delta_angle, duration, repeat = 1):
    if not isinstance(delta_angle, (float,int)) or not isinstance(duration, (float,int)) or not isinstance(repeat, int):
        raise NBGMQuitSignal('nbas_camera_rotate: Incorrect Parameter Type!')
    if duration < 1 or repeat == 0:
        raise NBGMQuitSignal('nbas_camera_rotate: Invalid Parameter Values!')
    RotateCamera(delta_angle, duration, repeat)

# Make camera slid on the track provided by position_array, if repeat == -1, means animation is an endless animation. 
# During the animation, camera distance & tilt angle do not change, but viewport and rotate will change based on current position
# The points of postion_list is (latitude, longitude)
def nbas_camera_make_linear_animation (postion_list, duration, repeat = 1):
    if not isinstance(postion_list, (list,tuple)) or not isinstance(duration, (float,int)) or not isinstance(repeat, int):
        raise NBGMQuitSignal('nbas_camera_make_linear_animation: Incorrect Parameter Type!')
    if duration < 1 or repeat == 0:
        return NBGMQuitSignal('nbas_camera_make_linear_animation: Invalid Parameter Values!')
    mercator_pos_list = []
    for point in postion_list:
        mercatorX, mercatorY = nbas_lat_long_to_mercator(point.x, point.y)
        mercator_pos_list.append(Point2D(mercatorX, mercatorY))
    nbas_camera_make_linear_animation_by_mercatorpoints(mercator_pos_list, duration, repeat)

# Make camera slid on the track provided by a list of mercator points
def nbas_camera_make_linear_animation_by_mercatorpoints (postion_list, duration, repeat = 1):
    if not isinstance(postion_list, (list,tuple)) or not isinstance(duration, (float,int)) or not isinstance(repeat, int):
        raise NBGMQuitSignal('nbas_camera_make_linear_animation_by_mercatorpoints: Incorrect Parameter Type!')
    if duration < 1 or repeat == 0:
        return NBGMQuitSignal('nbas_camera_make_linear_animation_by_mercatorpoints: Invalid Parameter Values!')
    MoveCamera(postion_list, duration, repeat)

# Make camera slid on the track provided by a list of tile names
def nbas_camera_make_linear_animation_by_tilenames (tile_list, duration, repeat = 1):
    if not isinstance(tile_list, (list,tuple)) or not isinstance(duration, (float,int)) or not isinstance(repeat, int):
        raise NBGMQuitSignal('nbas_camera_make_linear_animation_by_tilenames: Incorrect Parameter Type!')
    if duration < 1 or repeat == 0:
        return NBGMQuitSignal('nbas_camera_make_linear_animation_by_tilenames: Invalid Parameter Values!')
    position_list = []
    info = NBMTileInfo()
    for tile in tile_list:
        ret = GetNBMTileInfo(tile, info)
        if not ret:
            raise NBGMQuitSignal('nbas_camera_make_linear_animation_by_tilenames: GetNBMTileInfo from %s failed!', tileName)
        mercatorX, mercatorY = nbas_tile_to_mercator(info.x, info.y, info.z)
        position_list.append(Point2D(mercatorX, mercatorY))
    nbas_camera_make_linear_animation_by_mercatorpoints(position_list, duration, repeat)

#
#-----------------------------------------------------------------------------------------------------------------------------------------#
# OTHER OPERATIONS
#-----------------------------------------------------------------------------------------------------------------------------------------#
#

# Wait for seconds to update NBGM view completely, usually 2 seconds is enough. Please Must Not use sleep for replacing this function.
def nbas_wait_for_seconds(seconds):
    # ensure the minimum time is one second
    if not isinstance(seconds, (int, float)) or seconds < 1:
        raise NBGMQuitSignal('nbas_wait_for_seconds: Invalid Parameter Values!')
    WaitForSeconds(seconds)

# Snap a screenshot and save it to a bmp file.If the folder is not sepcified, screenshots will be saved to default 'screenshots' folder
def nbas_snap_screenshot(outputfile, info = ''):
    if not isinstance(outputfile, str) or not isinstance(info, str):
        raise NBGMQuitSignal('nbas_snap_screenshot: Incorrect Parameter Type!')
    path, filename = os.path.split(outputfile)
    if not path:
        outputfile = os.path.join(os.environ['nbgm_screenshots'], filename)
    SnapScreenShot(outputfile, info)

def nbas_set_horizon_distance(horizonDistance):
    if not isinstance(horizonDistance, (int, float)) or horizonDistance < 0:
        raise NBGMQuitSignal('nbas_set_horizon_distance: Invalid Parameter Values!')
    SetHorizonDistance(horizonDistance)

# a very simple tirck which is used to abort the execution of the script in time
def nbas_quit_if_aborted():
    WaitForSeconds(0.1)
    
def nbas_calc_view_distance_by_zoomlevel(zoomlevel):
    if not isinstance(zoomlevel, (int, float)) or zoomlevel < 0 or zoomlevel > 22:
        raise NBGMQuitSignal('nbas_calc_view_distance_by_zoomlevel: Invalid Parameter Values!')
    return CalcHeightByZoomLevel(zoomlevel)

def nbas_tile_to_lat_long(tileX, tileY, tileZ):
    mercatorX = TileToMercatorX(tileX, 0.5, tileZ)
    mercatorY = TileToMercatorY(tileY, 0.5, tileZ)
    return Mecator2LatLon(mercatorX, mercatorY)

def nbas_lat_long_to_mercator(latitude, longitude):
    return LatLon2Mercator(latitude, longitude)

def nbas_mercator_to_lat_long(mercatorX, mercatorY):
    return Mecator2LatLon(mercatorX, mercatorY)

def nbas_tile_to_mercator(tileX, tileY, tileZ):
    mercatorX = TileToMercatorX(tileX, 0.5, tileZ)
    mercatorY = TileToMercatorY(tileY, 0.5, tileZ)
    return mercatorX, mercatorY