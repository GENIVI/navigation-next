pragma Singleton
import QtQuick 2.3
//import QtApplicationManager 1.0
import QtQuick.Window 2.2

QtObject {
    id: root

    property color colorTransparent: "#00ffffff";
    property color colorWhite: "#ffffff";
    property color colorBlack1: "#000000";
    property color colorBlack2: "#141B25";
    property color colorBlack3: "#08202C";
    property color colorBlack4: "#111D29";
    property color colorBlack5: "#273133";
    property color colorBlack6: "#13151A";
    property color colorLightGray: "#D3D3D3";
    property color colorDarkGray: "#7D7F7F";
    property color colorGrey1: "#999999";
    property color colorCyan1: "#43fffe";
    property color colorCyan2: "#0ED9FD";
    property color colorCyan3: "#21dcfd";
    property color colorCyan4: "#00DEFF"
    property color colorLightBlue: "#55b1ea";
    property color colorLightBlue1: "#0FD9FD";
    property color colorGreen: "#008000";
    property color colorLightGreen: "#8FED31";
    property color colorLightGreen1: "#00F938";
    property color colorDeepBlue1: "#22566F";
    property color colorDeepBlue2: "#194255";
    property color colorDeepBlue3: "#142831";
    property color colorOrange: "#FF7C00";

    property double maxOpacity: 0.95;
    property double midOpacity: 0.8;
    property double minOpacity: 0.5;

    property int borderWidth1: 1;
    property int borderWidth2: 2;
    property int borderWidth3: 3;

    property int paddingXXS: 2;
    property int paddingXS: 5;
    property int paddingS: 10;
    property int padding: 15
    property int paddingL: 20;
    property int paddingXL: 25;
    property int paddingXXL: 30;

    property int fontSize50: 50;
    property int fontSize45: 45;
    property int fontSize38: 38;
    property int fontSize36: 36;
    property int fontSize32: 32;
    property int fontSize31: 31;
    property int fontSize30: 30;
    property int fontSize28: 28;
    property int fontSize24: 24;
    property int fontSize21: 21;
    property int fontSize20: 20;
    property int fontSize14: 14;

    property int rectRadiusXS: 3;
    property int rectRadiusS: 5;
    property int rectRadiusM: 16;
    property int rectRadiusXL: 50;
    property int rectRadiusL: 40;

//    property FontLoader fontRegular: FontLoader {
//        id:sfRegular
//        source: applicationDirPath + "/resource/font/SF-UI-Display-Regular.otf";
//    }
//    property FontLoader fontMedium: FontLoader {
//        id:sfMedium
//        source: applicationDirPath + "/resource/font/SF-UI-Display-Medium.otf";
//    }
//    property FontLoader fontIcon: FontLoader {
//        id:nextGen
//        source:  applicationDirPath+ "/resource/font/LG_NexGen.ttf";
//    }
    property FontLoader fontGuidance: FontLoader { id: maneuverFont; source: applicationDirPath1 + "/resource/fonts/guidancefont.ttf" }

    property FontLoader fontSans: FontLoader { id: sansFont; source: applicationDirPath1 + "/resource/fonts/DROIDSANS.TTF" }
}
