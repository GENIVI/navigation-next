from utilty import *
from time import sleep, time
from nbgmscripttask import NBGM_SCRIPT_CONTEX
import logging

IFINIT_LOOP_NUM = 99999999

#
#-----------------------------------------------------------------------------------------------------------------------------------------#
#  NBGM atomic action
#-----------------------------------------------------------------------------------------------------------------------------------------#
#

def RenderFrame(tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('RenderFrame', [], tm)

def LoadCommonMaterial(materialName, filePath, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('LoadCommonMaterial', (materialName.encode('utf-8'), filePath.encode('utf-8')), tm)

def SetCurrentCommonMaterial(materialName, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('SetCurrentCommonMaterial', [materialName.encode('utf-8')], tm)

def LoadNBMTile(nbmName, baseDrawOrder, labelDrawOrder, filePath, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('LoadNBMTile', (nbmName.encode('utf-8'), baseDrawOrder, labelDrawOrder, filePath.encode('utf-8')), tm)
    NBGM_SCRIPT_CONTEX.loaded_tiles.add(nbmName)

def UnLoadTile(nbmName, autoerease = True, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('UnLoadTile', [nbmName.encode('utf-8')], tm)
    if autoerease:
        NBGM_SCRIPT_CONTEX.loaded_tiles.remove(nbmName)

def Tilt(angle, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('Tilt', [angle], tm)

def SetTiltAngle(angle, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('SetTiltAngle', [angle], tm)
    NBGM_SCRIPT_CONTEX.tilt_angle = float(angle)

def Rotate(angle, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('Rotate', [angle], tm)

def SetRotateAngle(angle, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('SetRotateAngle', [angle], tm)
    NBGM_SCRIPT_CONTEX.rotate_angle = float(angle)

def SetRotateCenter(screenX, screenY, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('SetRotateCenter', (screenX, screenY), tm)

def OnTouchEvent(screenX, screenY, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('OnTouchEvent', (screenX, screenY), tm)

def OnPaning(screenX, screenY, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('OnPaning', (screenX, screenY), tm)

def Zoom(deltaH, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('Zoom', [deltaH], tm)

def Move(dx, dy, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('Move', (dx, dy), tm)

def SetViewCenter(mercatorX, mercatorY, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('SetViewCenter', (mercatorX, mercatorY), tm)

def SetHorizonDistance(horizonDistance, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('SetHorizonDistance', [horizonDistance], tm)

def SetViewPointDistance(distance, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('SetViewPointDistance', [distance], tm)
    NBGM_SCRIPT_CONTEX.viewpoint_distance = float(distance)

def SetViewSize(x, y, width, height, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('SetViewPointDistance', (x, y, width, height), tm)

def SetPerspective(fov, aspect, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('SetPerspective', (fov, aspect), tm)

def SetAvatarScale(scaleValue, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('SetAvatarScale', [scaleValue], tm)

def SetAvatarState(state, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('SetAvatarState', [state], tm)

def SetAvatarMode(mode, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('SetAvatarMode', [mode], tm)

def SelectAndTrackAvatar(x, y, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('SelectAndTrackAvatar', (x, y), tm)

def SetBackground(background, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('SetBackground', [background], tm)

def SetSkyDayNight(isDay, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('SetSkyDayNight', [isDay], tm)

def GetTiltAngle():
    return NBGM_SCRIPT_CONTEX.queryInfo('GetTiltAngle')

def GetRotateAngle():
    return NBGM_SCRIPT_CONTEX.queryInfo('GetRotateAngle')

def GetViewPointDistance():
    return NBGM_SCRIPT_CONTEX.queryInfo('GetViewPointDistance')

def GetCameraHeight():
    return NBGM_SCRIPT_CONTEX.queryInfo('GetCameraHeight')

def SnapScreenShot(file, info,  tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('SnapScreenShot', [file.encode('utf-8'), info.encode('utf-8')], tm)

#
#-----------------------------------------------------------------------------------------------------------------------------------------#
#  NBGM compound action
#-----------------------------------------------------------------------------------------------------------------------------------------#
#

def SetCenter(pt, tm = 0.0):
    NBGM_SCRIPT_CONTEX.addTask('SetViewCenter', (pt.x, pt.y), tm)

def LoadTile(name, tile, center = False):
    info = NBMTileInfo()
    ret = GetNBMTileInfo(tile, info)
    if not ret:
        logging.error('GetNBMTileInfo form %s failed!' % tile)
        return False
    if center:
        SetViewPortToTileCenter(info.x, info.y, info.z)
    LoadNBMTile(name, info.layerType, info.labelType, tile)
    return True


def LoadTiles(tiles, center = False):
    assert(isinstance(tiles, (list, tuple)))
    for t in tiles:
        LoadTile(t, t, center)

def UnloadAllTiles():
    for t in NBGM_SCRIPT_CONTEX.loaded_tiles:
        UnLoadTile(t, False)
    NBGM_SCRIPT_CONTEX.loaded_tiles.clear()

def __start_animation__(begin, end, duration, repeat, tm, func, linear = False):
    for i in range(repeat):
        start_time = tm + i*duration
        # KeyFrame0
        beginFrame = Frame(begin, start_time)
        # KeyFrame1
        endFrame = Frame(end, start_time+duration)
        # Use linear interpolation
        iterpolation = linear and LinearInterpolation or CustomInterpolation
        animation = Animation(beginFrame, endFrame, iterpolation)
        # run animation
        animation.start(func)

def ZoomCamera(scale, duration, repeat, tm = 0.0):
    if scale <= 0 or repeat == 0 or duration <= 0:
        return
    if repeat < 0:
        repeat = IFINIT_LOOP_NUM
    oldValue = [NBGM_SCRIPT_CONTEX.queryInfo('GetViewPointDistance')]
    newValue = [oldValue[0]*scale]
    action = [SetViewPointDistance]
    __start_animation__(oldValue, newValue, duration, repeat, tm, action)

def TiltCamera(delta_angle, duration, repeat, tm = 0.0):
    if delta_angle < 0 or delta_angle > 90 or repeat == 0 or duration <= 0:
        return
    if repeat < 0:
        repeat = IFINIT_LOOP_NUM
    oldValue = [NBGM_SCRIPT_CONTEX.queryInfo('GetTiltAngle')]
    newValue = [oldValue[0] + delta_angle]
    action = [SetTiltAngle]
    __start_animation__(oldValue, newValue, duration, repeat, tm, action)

def RotateCamera(delta_angle, duration, repeat, tm = 0.0):
    if repeat == 0 or duration <= 0:
        return
    if repeat < 0:
        repeat = IFINIT_LOOP_NUM
    oldValue = [NBGM_SCRIPT_CONTEX.queryInfo('GetRotateAngle')]
    newValue = [oldValue[0] + delta_angle]
    action = [SetRotateAngle]
    __start_animation__(oldValue, newValue, duration, repeat, tm, action)

def MoveCamera(position_list, duration, repeat, tm  = 0.0):
    ROTATE_TIME = 0.8
    if not position_list or repeat == 0 or duration <= 0:
        return
    count = len(position_list)
    if count < 2:
        return
    if repeat < 0:
        repeat = IFINIT_LOOP_NUM

    rotate_time = ROTATE_TIME*(count-1)
    if duration > rotate_time:
        duration -= rotate_time

    distance_list = []
    roate_list = []
    distance_time_list = []
    for i in range(count-1):
        dis = position_list[i].getDistance(position_list[i+1])
        distance_list.append(dis)
        angle =  position_list[i].getAngle(position_list[i+1])
        roate_list.append(angle)
    total_distance = sum(distance_list)
    unit =  duration/total_distance
    distance_time_list = map(lambda x : x*unit, distance_list)
    RotateCamera(bound_angle((90 - roate_list[0] - GetRotateAngle())), ROTATE_TIME, 1)
    while repeat:
        SetRotateAngle(90 - roate_list[0])
        for i in range(count-1):
            oldValue = [position_list[i]]
            newValue = [position_list[i+1]]
            action = [SetCenter]
            __start_animation__(oldValue, newValue, distance_time_list[i], 1, tm, action)
            if i < count-2:
                RotateCamera(bound_angle(-(roate_list[i+1] - roate_list[i])), ROTATE_TIME, 1)
        WaitForSeconds(1)
        repeat -= 1

def SetViewPortToTileCenter(tileX, tileY, tileZ):
    x = TileToMercatorX(tileX, 0.5, tileZ)
    y = TileToMercatorY(tileY, 0.5, tileZ)
    SetViewCenter(x, y)

def WaitForSeconds(seconds):
    if seconds <= 0:
        return
    NBGM_SCRIPT_CONTEX.WaitForSeconds(seconds)
