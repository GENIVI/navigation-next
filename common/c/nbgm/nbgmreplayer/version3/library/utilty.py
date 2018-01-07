import os,sys,math,datetime

LAT_BOUND = 85.05112878
LON_BOUDN = 180.0
ANIMATION_RATE = 38
RADIUS_EARTH_METERS = 6366197.7236758135
EXTREAM_FAR_DISTANCE = 9999999999.0
ONE_METER_IN_MERCATOR = 1.0/RADIUS_EARTH_METERS

class Frame:
     def __init__(self, value, time):
          assert(isinstance(value, list))
          self.value = value
          self.time = time

class Interpolation:
     def __init__(self, beginFrame, endFrame, count):
          assert(count>1)
          self.begin = beginFrame
          self.end = endFrame
          self.detTime = (self.end.time - self.begin.time)/(count-1)
          self.value = self.begin.value
          self.time = self.begin.time
          self.count = count
          self.index = 0

     def __iter__(self):
          return self

     def __next__(self):
          if self.index > self.count:
               raise StopIteration()
          result = Frame(self.value, self.time)
          self.value = list(map(lambda x,y : x+(y-x)*self.custom_map((float(self.index+1)/self.count)), self.begin.value, self.end.value))
          self.time += self.detTime
          self.index += 1
          return result

     def custom_map(self, x):
          pass

class LinearInterpolation(Interpolation):
     def __init__(self, *arg, **argk):
          Interpolation.__init__(self, *arg, **argk)
     def custom_map(self, x):
          return x

class CustomInterpolation(Interpolation):
     def __init__(self, *arg, **argk):
          Interpolation.__init__(self, *arg, **argk)
     def custom_map(self, x):
          return (x-1)**3+1.0

class Animation:
     def __init__(self, beginFrame, endFrame, interpolation_type = None):
          assert( beginFrame and endFrame and endFrame.time > beginFrame.time)
          if interpolation_type is None:
               self.interpolation_type = LinearInterpolation
          else:
               self.interpolation_type = interpolation_type
          self.frames = int((endFrame.time - beginFrame.time)*ANIMATION_RATE)
          assert(self.frames > 0)
          self.beginFrame = beginFrame
          self.endFrame = endFrame

     def __iter__(self):
          return self.interpolation_type(self.beginFrame, self.endFrame, self.frames)

     def start(self, action):
          for frame in self:
               i = 0
               for func in action:
                    func(frame.value[i], frame.time)
                    i+=1


def bound_lon(lon):
     while lon < -LON_BOUDN:
          lon += LON_BOUDN*2
     while lon >= LON_BOUDN:
          lon -= LON_BOUDN*2;
     return lon

def bound_lat(lat):
     while lat < -LAT_BOUND:
          lat =- LAT_BOUND
     while lat > LAT_BOUND:
          lat = LON_BOUDN;
     return lat

def LatLon2Mercator(lat, lon):
     mx = bound_lon(math.radians(lon))
     rlat = bound_lat(math.radians(lat))
     my = math.log(math.tan(rlat)+1.0/math.cos(rlat))
     return (mx, my)

def bound_mercator(x):
     if x < -math.pi:
          x = -math.pi
     if x > math.pi:
          x = math.pi
     return x

def bound_angle(x):
     while x > 360:
          x -= 360
     while x < 0:
          x += 360
     if x >= 180:
          x -= 360
     return x

def Mecator2LatLon(mx, my):
     x = bound_mercator(mx)
     y = bound_mercator(my)
     latitude = math.degree(math.tan(math.sinh(y)))
     longitude = math.degree(x)
     return (latitude, longitude)

def TileToMercatorX(tx, px, tz):
     tScale = 1 << (tz - 1)
     return ((tx + px)/tScale - 1.0)*math.pi

def TileToMercatorY(ty, py, tz):
     tScale = 1 << (tz - 1)
     return (1.0 - (ty + py)/tScale)*math.pi


def FindAllFilesInFolder(folder):
     file_list = []
     for root, dis, files in os.walk(folder):
          for file in files:
               file_list.append(os.path.join(root, file))
     return file_list

class Point2D:
     def __init__(self, x = 0.0, y = 0.0):
          self.x = x
          self.y = y

     def __str__(self):
          return '(%.5f, %.5f)'%(self.x, self.y)

     def __sub__(self, pt):
          return Point2D(self.x - pt.x, self.y - pt.y)

     def __add__(self, pt):
          return Point2D(self.x + pt.x, self.y + pt.y)

     def __div__(self, k):
          return Point2D(self.x/k, self.y/k)

     def __mul__(self, k):
          return Point2D(self.x*k, self.y*k)

     def getDistance(self, point):
          assert(point)
          return math.sqrt(((self.x - point.x)**2 + (self.y - point.y)**2))

     def getAngle(self,point):
          assert(point)
          x = point.x - self.x
          y = point.y - self.y
          tempX = abs(x)
          tempY = abs(y)
          if tempX < 1e-6 and tempY < 1e-6:
               return 0.0
          tan = 0.0
          if tempX > tempY:
               tan = tempY/tempX
          else:
               tan = tempX/tempY
          theta = math.degrees(math.atan(tan))
          if tempX < tempY:
               theta = 90.0 - theta
          if x < 0:
               theta = 180.0 - theta
          if y < 0:
               theta = 360.0 - theta
          return theta

class Point3D:
     def __init__(self, x = 0.0, y = 0.0, z = 0.0):
          self.x = x
          self.y = y
          self.z = z

     def __str__(self):
          return "(%.5f, %.5f, %.5f)"%(self.x, self.y, self.z)

class NBMTileInfo:
     def __init__(self, x = 0, y = 0, z = 0, layerType = 0, labelType = 0):
          self.x = x
          self.y = y
          self.z = z
          self.layerType = layerType
          self.labelType = labelType

def enum(*sequential, **named):
     enums = dict(zip(sequential, range(len(sequential))), **named)
     return type('Enum', (), enums)

LAYER_ID = enum('BR1',\
                'BR2',\
                'BR3',\
                'BR4',\
                'BR5',\
                'BR6',\
                'BR7',\
                'BR8',\
                'BR9',\
                'BR10',\
                'BR11',\
                'BR12',\
                'BR13',\
                'BR14',\
                'BR15',\
                'GVA',\
                'GVR',\
                'DVA',\
                'DVR',\
                'B3D',\
                'LM3D',\
                'DVR_LABEL',\
                'DVA_LABEL',\
                'POINT_LABEL',\
                'TRAFFIC',\
                'PIN',\
                'RASTR',\
                'BUILDING_2D')

def GetNBMTileInfo(filePath, tileInfo):
     fileName = os.path.split(filePath)[1]
     title, ext = os.path.splitext(fileName)
     ret = False
     # 1. try to parse 09371125363DLM.nbm
     if ext == '.nbm':
          fileType = title[10:14]
          if fileType == 'AREA':
               tileInfo.layerType = LAYER_ID.DVA
               tileInfo.labelType = LAYER_ID.DVA_LABEL
               ret = True
          elif fileType == 'ROAD':
               tileInfo.layerType = LAYER_ID.DVR
               tileInfo.labelType = LAYER_ID.DVR_LABEL
               ret = True
          elif fileType == '3DLM' or fileType == 'LM3D':
               tileInfo.layerType = LAYER_ID.LM3D
               tileInfo.labelType = LAYER_ID.DVA_LABEL
               ret = True
          elif fileType == '3DUT':
               tileInfo.layerType = LAYER_ID.B3D
               tileInfo.labelType = LAYER_ID.DVA_LABEL
               ret = True
          elif fileType == 'DMAT' or fileType == 'NMAT':
               ret = True
          elif fileType == 'LABE':
               ret = True
          elif fileType == 'BR11':
               tileInfo.layerType = LAYER_ID.BR11
               tileInfo.x = int(title[:5])
               tileInfo.y = int(title[5:10])
               tileInfo.z = 11
               return True
          elif fileType == 'BR12':
               tileInfo.layerType = LAYER_ID.BR12
               tileInfo.x = int(title[:5])
               tileInfo.y = int(title[5:10])
               tileInfo.z = 12
               return True
          elif fileType == 'BR13':
               tileInfo.layerType = LAYER_ID.BR13
               tileInfo.x = int(title[:5])
               tileInfo.y = int(title[5:10])
               tileInfo.z = 13
               return True
          elif fileType == 'BR14':
               tileInfo.layerType = LAYER_ID.BR14
               tileInfo.x = int(title[:5])
               tileInfo.y = int(title[5:10])
               tileInfo.z = 14
               return True
          elif fileType == 'BR15':
               tileInfo.layerType = LAYER_ID.BR15
               tileInfo.x = int(title[:5])
               tileInfo.y = int(title[5:10])
               tileInfo.z = 15
               return True
          elif fileType == 'TRAF':
               tileInfo.layerType = LAYER_ID.TRAFFIC
               tileInfo.x = int(title[:5])
               tileInfo.y = int(title[5:10])
               tileInfo.z = 11
               return True
          elif fileType == 'RLLP':
               tileInfo.layerType = LAYER_ID.POINT_LABEL
               tileInfo.x = int(title[:5])
               tileInfo.y = int(title[5:10])
               tileInfo.z = 8
               return True
          elif fileType == 'RAST':
               tileInfo.x = int(title[:5])
               tileInfo.y = int(title[5:10])
               tileInfo.z = 15
               tileInfo.layerType = LAYER_ID.RASTER
               return True
          elif fileType == 'LBLT':
               tileInfo.x = int(title[:5])
               tileInfo.y = int(title[5:10])
               tileInfo.z = 15
               tileInfo.layerType = LAYER_ID.RASTER
               return True
          elif fileType == 'RUTE':
               ret = True
          else:
               ret = False
          if ret:
               tileInfo.x = int(title[:5])
               tileInfo.y = int(title[5:10])
               tileInfo.z = 15

     #2. try to parse T_DVA_1_NBM_23_5899_12853
     #3. try to parse T_BR2_1_PNG_23_0_0
     if not ret:
          if ext == '' and title[0] == 'T':
               data = title.split('_')
               if len(data) == 7:
                    tileInfo.x = int(data[5])
                    tileInfo.y = int(data[6])
                    fileType = data[1]

                    if fileType == 'DVR':
                         tileInfo.layerType = LAYER_ID.DVR
                         tileInfo.labelType = LAYER_ID.DVR_LABEL
                         tileInfo.z = 15
                         return True
                    elif fileType == 'DVA':
                         tileInfo.layerType = LAYER_ID.DVA
                         tileInfo.labelType = LAYER_ID.DVA_LABEL
                         tileInfo.z = 15
                         return True
                    elif fileType == 'GVR':
                         tileInfo.layerType = LAYER_ID.GVA
                         tileInfo.labelType = LAYER_ID.DVR_LABEL
                         tileInfo.z = 12
                         return True
                    elif fileType == 'GVA':
                         tileInfo.layerType = LAYER_ID.GVA
                         tileInfo.labelType = LAYER_ID.DVA_LABEL
                         tileInfo.z = 12
                         return True
                    elif fileType == 'B3D':
                         tileInfo.layerType = LAYER_ID.B3D
                         tileInfo.labelType = LAYER_ID.DVA_LABEL
                         tileInfo.z = 15
                         return True
                    elif fileType == '3DLM' or fileType == 'LM3D':
                         tileInfo.layerType = LAYER_ID.LM3D
                         tileInfo.labelType = LAYER_ID.DVA_LABEL
                         #ileInfo.z = 15
                         return True
                    elif fileType == 'BR2':
                         tileInfo.layerType = LAYER_ID.BR2
                         tileInfo.z = 2
                         return True
                    elif fileType == 'BR3':
                         tileInfo.layerType = LAYER_ID.BR3
                         tileInfo.z = 3
                         return True
                    elif fileType == 'BR4':
                         tileInfo.layerType = LAYER_ID.BR4
                         tileInfo.z = 4
                         ret = True
                    elif fileType == 'BR5':
                         tileInfo.layerType = LAYER_ID.BR5
                         tileInfo.z = 5
                         return True
                    elif fileType == 'BR6':
                         tileInfo.layerType = LAYER_ID.BR6
                         tileInfo.z = 6
                         return True
                    elif fileType == 'BR7':
                         tileInfo.layerType = LAYER_ID.BR7
                         tileInfo.z = 7
                         return True
                    elif fileType == 'BR8':
                         tileInfo.layerType = LAYER_ID.BR8
                         tileInfo.z = 8
                         return True
                    elif fileType == 'BR9':
                         tileInfo.layerType = LAYER_ID.BR9
                         tileInfo.z = 9
                         return True
                    elif fileType == 'BR10':
                         tileInfo.layerType = LAYER_ID.BR10
                         tileInfo.z = 10
                         ret = True
                    elif fileType == 'BR11':
                         tileInfo.layerType = LAYER_ID.BR11
                         tileInfo.z = 11
                         return True
                    elif fileType == 'DVRT':
                         tileInfo.layerType = LAYER_ID.DVR
                         tileInfo.z = 15
                         return True
                    elif fileType == 'B2DT':
                         tileInfo.x = int(title[:5])
                         tileInfo.y = int(title[5:10])
                         tileInfo.z = 15
                         tileInfo.layerType = LAYER_ID.BUILDING_2D
                         return True
                    else:
                         ret = False
     # try to parse 000010000103RAST
     if not ret:
          fileType = title[12:16]
          if fileType == 'RAST':
               tileInfo.layerType = LAYER_ID.BR2
               ret = True
          elif fileType == 'LBLT':
               tileInfo.labelType = LAYER_ID.POINT_LABEL
               ret = True
          else:
               ret = False
          if ret:
               tileInfo.x = int(title[:5])
               tileInfo.y = int(title[5:10])
               tileInfo.z = int(title[10:12])
     return ret

RELATIVE_ZOOM_LEVEL = 17
MAP_CELL_SIZE_ON_RELATIVE_ZOOM_LEVEL = 1.194329
def CalcHeightByZoomLevel(zoomLevel, fov = 45.0):
     viewHeight = int(os.environ['nbgm_height'])
     referenceDistance = MAP_CELL_SIZE_ON_RELATIVE_ZOOM_LEVEL*viewHeight /(2*math.tan(math.radians(fov*0.5)))
     height =  referenceDistance*(2**(RELATIVE_ZOOM_LEVEL - zoomLevel))
     return height