import ConfigParser,logging

class Region:
    def __init__(self, mapPath = '', inputPath = ''):
        self.mapPath = mapPath.strip()
        self.inputPath = inputPath.strip()

    def isValid(self):
        return self.mapPath and self.inputPath

class LevelDspProp:
    def __init__(self, text):
        self.item1 = ''
        self.item2 = ''
        self.load(text)

    def load(self, text):
        content = text.strip()
        index = content.find(' ')
        if index != -1:
            self.item1 = content[:index].strip()
            self.item2 = content[index+1:].strip()
        else:
            self.item1 = content
            self.item2 = ''

    def hasBR(self):
        return self.item1.find('BR') != -1 or self.item2.find('BR') != -1

    def hasLBL(self):
        return self.item1.find('LBL') !=  -1 or self.item2.find('LBL') != -1

    def isValid(self):
        if (not self.item1 or self.item1 == '-') and (not self.item2 or self.item2 == '-'):
            return False
        return True

class NBGMINIReader:
    COMMON = 'Common'
    REGIONS = 'Regions'
    FEATURE_TEST_LEVES = 'Feature Test Levels'

    def __init__(self):
        self.iniFile = ConfigParser.ConfigParser()
        self.regionList = []
        self.testLevels = {}
        self.waitTime = 5.0

    def read(self, filePath):
        try:
            self.iniFile.read(filePath)
            if not self.iniFile.has_section(NBGMINIReader.COMMON):
                logging.CRITICAL('Cannot find the section %s!' % COMMON)
                return False
            if not self.iniFile.has_section(NBGMINIReader.REGIONS):
                logging.CRITICAL('Cannot find the section %s!' % REGIONS)
                return False
            if not self.iniFile.has_section(NBGMINIReader.FEATURE_TEST_LEVES):
                logging.CRITICAL('Cannot find the section %s!' % FEATURE_TEST_LEVES)
                return False
            waitTime = self.iniFile.getint(NBGMINIReader.COMMON, 'WaitTime')
            self.waitTime = waitTime/1000.0
            ops = self.iniFile.options(NBGMINIReader.REGIONS)
            if not ops:
                logging.CRITICAL('Section [Regions] is empty!')
                return False
            regions = {}
            for item in ops:
                if item.find('map') != -1:
                    regions[item] = ''
            for item in ops:
                if item.find('input') != -1:
                    map_key = item.replace('input', 'map')
                    if regions.has_key(map_key):
                        regions[map_key] = item
            for k,v in regions.iteritems():
                mapPath = self.iniFile.get(NBGMINIReader.REGIONS, k).strip()
                inputPath = self.iniFile.get(NBGMINIReader.REGIONS, v).strip()
                if mapPath and inputPath:
                    self.regionList.append(Region(mapPath, inputPath))

            testLevels = self.iniFile.options(NBGMINIReader.FEATURE_TEST_LEVES)
            if not testLevels:
                logging.CRITICAL('Section [Feature Test Levels] is empty!')
                return False
            for level in testLevels:
                items = self.iniFile.get(NBGMINIReader.FEATURE_TEST_LEVES, level).split(',')
                data = []
                for i in items:
                    data.append(LevelDspProp(i))
                if data:
                    self.testLevels[level] = data
            return True
        except Exception,e:
            print (e)
            logging.CRITICAL('The ini file %s is invalid!' % filePath)
            return False

    def GetLevles(self, fileName):
        name = fileName.lower()
        for level in self.testLevels:
            if name.find(level) != -1:
                return self.testLevels[level]
        return None