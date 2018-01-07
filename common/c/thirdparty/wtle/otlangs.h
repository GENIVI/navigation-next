/*****************************************************************************
 *
 *  otlangs.h - OpenType languages.
 *
 *  Copyright (C) 2002 Monotype Imaging Inc. All rights reserved.
 *
 * Monotype Imaging Confidential.
 *
 ****************************************************************************/

#ifndef OTLANGS_H
#define OTLANGS_H

#include "tstag.h"

TS_BEGIN_HEADER

/******************************************************************************
 *
 *  Constant TsTag declarations and initializations of known OT language
 *  system designations.
 *
 *  <GROUP opentype>
 */
#define TsTag_ABA  TsMakeTag('A','B','A',' ') /* Abaza */
#define TsTag_ABK  TsMakeTag('A','B','K',' ') /* Abkhazian */
#define TsTag_ADY  TsMakeTag('A','D','Y',' ') /* Adyghe */
#define TsTag_AFK  TsMakeTag('A','F','K',' ') /* Afrikaans */
#define TsTag_AFR  TsMakeTag('A','F','R',' ') /* Afar */
#define TsTag_AGW  TsMakeTag('A','G','W',' ') /* Agaw */
#define TsTag_ALT  TsMakeTag('A','L','T',' ') /* Altai */
#define TsTag_AMH  TsMakeTag('A','M','H',' ') /* Amharic */
#define TsTag_ARA  TsMakeTag('A','R','A',' ') /* Arabic */
#define TsTag_ARI  TsMakeTag('A','R','I',' ') /* Aari */
#define TsTag_ARK  TsMakeTag('A','R','K',' ') /* Arakanese */
#define TsTag_ASM  TsMakeTag('A','S','M',' ') /* Assamese */
#define TsTag_ATH  TsMakeTag('A','T','H',' ') /* Athapaskan */
#define TsTag_AVR  TsMakeTag('A','V','R',' ') /* Avar */
#define TsTag_AWA  TsMakeTag('A','W','A',' ') /* Awadhi */
#define TsTag_AYM  TsMakeTag('A','Y','M',' ') /* Aymara */
#define TsTag_AZE  TsMakeTag('A','Z','E',' ') /* Azeri */
#define TsTag_BAD  TsMakeTag('B','A','D',' ') /* Badaga */
#define TsTag_BAG  TsMakeTag('B','A','G',' ') /* Baghelkhandi */
#define TsTag_BAL  TsMakeTag('B','A','L',' ') /* Balkar */
#define TsTag_BAU  TsMakeTag('B','A','U',' ') /* Baule */
#define TsTag_BBR  TsMakeTag('B','B','R',' ') /* Berber */
#define TsTag_BCH  TsMakeTag('B','C','H',' ') /* Bench */
#define TsTag_BCR  TsMakeTag('B','C','R',' ') /* Bible Cree */
#define TsTag_BEL  TsMakeTag('B','E','L',' ') /* Belarussian */
#define TsTag_BEM  TsMakeTag('B','E','M',' ') /* Bemba */
#define TsTag_BEN  TsMakeTag('B','E','N',' ') /* Bengali */
#define TsTag_BGR  TsMakeTag('B','G','R',' ') /* Bulgarian */
#define TsTag_BHI  TsMakeTag('B','H','I',' ') /* Bhili */
#define TsTag_BHO  TsMakeTag('B','H','O',' ') /* Bhojpuri */
#define TsTag_BIK  TsMakeTag('B','I','K',' ') /* Bikol */
#define TsTag_BIL  TsMakeTag('B','I','L',' ') /* Bilen */
#define TsTag_BKF  TsMakeTag('B','K','F',' ') /* Blackfoot */
#define TsTag_BLI  TsMakeTag('B','L','I',' ') /* Balochi */
#define TsTag_BLN  TsMakeTag('B','L','N',' ') /* Balante */
#define TsTag_BLT  TsMakeTag('B','L','T',' ') /* Balti */
#define TsTag_BMB  TsMakeTag('B','M','B',' ') /* Bambara */
#define TsTag_BML  TsMakeTag('B','M','L',' ') /* Bamileke */
#define TsTag_BRE  TsMakeTag('B','R','E',' ') /* Breton */
#define TsTag_BRH  TsMakeTag('B','R','H',' ') /* Brahui */
#define TsTag_BRI  TsMakeTag('B','R','I',' ') /* Braj Bhasha */
#define TsTag_BRM  TsMakeTag('B','R','M',' ') /* Burmese */
#define TsTag_BSH  TsMakeTag('B','S','H',' ') /* Bashkir */
#define TsTag_BTI  TsMakeTag('B','T','I',' ') /* Beti */
#define TsTag_CAT  TsMakeTag('C','A','T',' ') /* Catalan */
#define TsTag_CEB  TsMakeTag('C','E','B',' ') /* Cebuano */
#define TsTag_CHE  TsMakeTag('C','H','E',' ') /* Chechen */
#define TsTag_CHG  TsMakeTag('C','H','G',' ') /* Chaha Gurage */
#define TsTag_CHH  TsMakeTag('C','H','H',' ') /* Chattisgarhi */
#define TsTag_CHI  TsMakeTag('C','H','I',' ') /* Chichewa */
#define TsTag_CHK  TsMakeTag('C','H','K',' ') /* Chukchi */
#define TsTag_CHP  TsMakeTag('C','H','P',' ') /* Chipewyan */
#define TsTag_CHR  TsMakeTag('C','H','R',' ') /* Cherokee */
#define TsTag_CHU  TsMakeTag('C','H','U',' ') /* Chuvash */
#define TsTag_CMR  TsMakeTag('C','M','R',' ') /* Comorian */
#define TsTag_COP  TsMakeTag('C','O','P',' ') /* Coptic */
#define TsTag_CRE  TsMakeTag('C','R','E',' ') /* Cree */
#define TsTag_CRR  TsMakeTag('C','R','R',' ') /* Carrier */
#define TsTag_CRT  TsMakeTag('C','R','T',' ') /* Crimean Tatar */
#define TsTag_CSL  TsMakeTag('C','S','L',' ') /* Church Slavonic */
#define TsTag_CSY  TsMakeTag('C','S','Y',' ') /* Czech */
#define TsTag_DAN  TsMakeTag('D','A','N',' ') /* Danish */
#define TsTag_DAR  TsMakeTag('D','A','R',' ') /* Dargwa */
#define TsTag_DCR  TsMakeTag('D','C','R',' ') /* Woods Cree */
#define TsTag_DEU  TsMakeTag('D','E','U',' ') /* German (Standard) */
#define TsTag_DGR  TsMakeTag('D','G','R',' ') /* Dogri */
#define TsTag_DHV  TsMakeTag('D','H','V',' ') /* Dhivehi */
#define TsTag_DJR  TsMakeTag('D','J','R',' ') /* Djerma */
#define TsTag_DNG  TsMakeTag('D','N','G',' ') /* Dangme */
#define TsTag_DNK  TsMakeTag('D','N','K',' ') /* Dinka */
#define TsTag_DUN  TsMakeTag('D','U','N',' ') /* Dungan */
#define TsTag_DZN  TsMakeTag('D','Z','N',' ') /* Dzongkha */
#define TsTag_EBI  TsMakeTag('E','B','I',' ') /* Ebira */
#define TsTag_ECR  TsMakeTag('E','C','R',' ') /* Eastern Cree */
#define TsTag_EDO  TsMakeTag('E','D','O',' ') /* Edo */
#define TsTag_EFI  TsMakeTag('E','F','I',' ') /* Efik */
#define TsTag_ELL  TsMakeTag('E','L','L',' ') /* Greek */
#define TsTag_ENG  TsMakeTag('E','N','G',' ') /* English */
#define TsTag_ERZ  TsMakeTag('E','R','Z',' ') /* Erzya */
#define TsTag_ESP  TsMakeTag('E','S','P',' ') /* Spanish */
#define TsTag_ETI  TsMakeTag('E','T','I',' ') /* Estonian */
#define TsTag_EUQ  TsMakeTag('E','U','Q',' ') /* Basque */
#define TsTag_EVK  TsMakeTag('E','V','K',' ') /* Evenki */
#define TsTag_EVN  TsMakeTag('E','V','N',' ') /* Even */
#define TsTag_EWE  TsMakeTag('E','W','E',' ') /* Ewe */
#define TsTag_FAN  TsMakeTag('F','A','N',' ') /* French Antillean */
#define TsTag_FAR  TsMakeTag('F','A','R',' ') /* Farsi */
#define TsTag_FIN  TsMakeTag('F','I','N',' ') /* Finnish */
#define TsTag_FJI  TsMakeTag('F','J','I',' ') /* Fijian */
#define TsTag_FLE  TsMakeTag('F','L','E',' ') /* Flemish */
#define TsTag_FNE  TsMakeTag('F','N','E',' ') /* Forest Nenets */
#define TsTag_FON  TsMakeTag('F','O','N',' ') /* Fon */
#define TsTag_FOS  TsMakeTag('F','O','S',' ') /* Faroese */
#define TsTag_FRA  TsMakeTag('F','R','A',' ') /* French (Standard) */
#define TsTag_FRI  TsMakeTag('F','R','I',' ') /* Frisian */
#define TsTag_FRL  TsMakeTag('F','R','L',' ') /* Friulian */
#define TsTag_FTA  TsMakeTag('F','T','A',' ') /* Futa */
#define TsTag_FUL  TsMakeTag('F','U','L',' ') /* Fulani */
#define TsTag_GAD  TsMakeTag('G','A','D',' ') /* Ga */
#define TsTag_GAE  TsMakeTag('G','A','E',' ') /* Gaelic */
#define TsTag_GAG  TsMakeTag('G','A','G',' ') /* Gagauz */
#define TsTag_GAL  TsMakeTag('G','A','L',' ') /* Galician */
#define TsTag_GAR  TsMakeTag('G','A','R',' ') /* Garshuni */
#define TsTag_GAW  TsMakeTag('G','A','W',' ') /* Garhwali */
#define TsTag_GEZ  TsMakeTag('G','E','Z',' ') /* Ge'ez */
#define TsTag_GIL  TsMakeTag('G','I','L',' ') /* Gilyak */
#define TsTag_GMZ  TsMakeTag('G','M','Z',' ') /* Gumuz */
#define TsTag_GON  TsMakeTag('G','O','N',' ') /* Gondi */
#define TsTag_GRN  TsMakeTag('G','R','N',' ') /* Greenlandic */
#define TsTag_GRO  TsMakeTag('G','R','O',' ') /* Garo */
#define TsTag_GUA  TsMakeTag('G','U','A',' ') /* Guarani */
#define TsTag_GUJ  TsMakeTag('G','U','J',' ') /* Gujarati */
#define TsTag_HAI  TsMakeTag('H','A','I',' ') /* Haitian */
#define TsTag_HAL  TsMakeTag('H','A','L',' ') /* Halam */
#define TsTag_HAR  TsMakeTag('H','A','R',' ') /* Harauti */
#define TsTag_HAU  TsMakeTag('H','A','U',' ') /* Hausa */
#define TsTag_HAW  TsMakeTag('H','A','W',' ') /* Hawaiin */
#define TsTag_HBN  TsMakeTag('H','B','N',' ') /* Hammer-Banna */
#define TsTag_HIL  TsMakeTag('H','I','L',' ') /* Hiligaynon */
#define TsTag_HIN  TsMakeTag('H','I','N',' ') /* Hindi */
#define TsTag_HMA  TsMakeTag('H','M','A',' ') /* High Mari */
#define TsTag_HND  TsMakeTag('H','N','D',' ') /* Hindko */
#define TsTag_HO   TsMakeTag('H','O',' ',' ') /* Ho */
#define TsTag_HRI  TsMakeTag('H','R','I',' ') /* Harari */
#define TsTag_HRV  TsMakeTag('H','R','V',' ') /* Croatian */
#define TsTag_HUN  TsMakeTag('H','U','N',' ') /* Hungarian */
#define TsTag_HYE  TsMakeTag('H','Y','E',' ') /* Armenian */
#define TsTag_IBO  TsMakeTag('I','B','O',' ') /* Igbo */
#define TsTag_IJO  TsMakeTag('I','J','O',' ') /* Ijo */
#define TsTag_ILO  TsMakeTag('I','L','O',' ') /* Ilokano */
#define TsTag_IND  TsMakeTag('I','N','D',' ') /* Indonesian */
#define TsTag_ING  TsMakeTag('I','N','G',' ') /* Ingush */
#define TsTag_INU  TsMakeTag('I','N','U',' ') /* Inuktitut */
#define TsTag_IRI  TsMakeTag('I','R','I',' ') /* Irish */
#define TsTag_IRT  TsMakeTag('I','R','T',' ') /* Irish Traditional */
#define TsTag_ISL  TsMakeTag('I','S','L',' ') /* Icelandic */
#define TsTag_ISM  TsMakeTag('I','S','M',' ') /* Inari Sami */
#define TsTag_ITA  TsMakeTag('I','T','A',' ') /* Italian */
#define TsTag_IWR  TsMakeTag('I','W','R',' ') /* Hebrew */
#define TsTag_JAV  TsMakeTag('J','A','V',' ') /* Javanese */
#define TsTag_JII  TsMakeTag('J','I','I',' ') /* Yiddish */
#define TsTag_JAN  TsMakeTag('J','A','N',' ') /* Japanese */
#define TsTag_JUD  TsMakeTag('J','U','D',' ') /* Judezmo */
#define TsTag_JUL  TsMakeTag('J','U','L',' ') /* Jula */
#define TsTag_KAB  TsMakeTag('K','A','B',' ') /* Kabardian */
#define TsTag_KAC  TsMakeTag('K','A','C',' ') /* Kachchi */
#define TsTag_KAL  TsMakeTag('K','A','L',' ') /* Kalenjin */
#define TsTag_KAN  TsMakeTag('K','A','N',' ') /* Kannada */
#define TsTag_KAR  TsMakeTag('K','A','R',' ') /* Karachay */
#define TsTag_KAT  TsMakeTag('K','A','T',' ') /* Georgian */
#define TsTag_KAZ  TsMakeTag('K','A','Z',' ') /* Kazakh */
#define TsTag_KEB  TsMakeTag('K','E','B',' ') /* Kebena */
#define TsTag_KGE  TsMakeTag('K','G','E',' ') /* Khutsuri Georgian */
#define TsTag_KHA  TsMakeTag('K','H','A',' ') /* Khakass */
#define TsTag_KHK  TsMakeTag('K','H','K',' ') /* Khanty-Kazim */
#define TsTag_KHM  TsMakeTag('K','H','M',' ') /* Khmer */
#define TsTag_KHS  TsMakeTag('K','H','S',' ') /* Khanty-Shurishkar */
#define TsTag_KHV  TsMakeTag('K','H','V',' ') /* Khanty-Vakhi */
#define TsTag_KHW  TsMakeTag('K','H','W',' ') /* Khowar */
#define TsTag_KIK  TsMakeTag('K','I','K',' ') /* Kikuyu */
#define TsTag_KIR  TsMakeTag('K','I','R',' ') /* Kirghiz */
#define TsTag_KIS  TsMakeTag('K','I','S',' ') /* Kisii */
#define TsTag_KKN  TsMakeTag('K','K','N',' ') /* Kokni */
#define TsTag_KLM  TsMakeTag('K','L','M',' ') /* Kalmyk */
#define TsTag_KMB  TsMakeTag('K','M','B',' ') /* Kamba */
#define TsTag_KMN  TsMakeTag('K','M','N',' ') /* Kumaoni */
#define TsTag_KMO  TsMakeTag('K','M','O',' ') /* Komo */
#define TsTag_KMS  TsMakeTag('K','M','S',' ') /* Komso */
#define TsTag_KNR  TsMakeTag('K','N','R',' ') /* Kanuri */
#define TsTag_KOD  TsMakeTag('K','O','D',' ') /* Kodagu */
#define TsTag_KOK  TsMakeTag('K','O','K',' ') /* Konkani */
#define TsTag_KON  TsMakeTag('K','O','N',' ') /* Kikongo */
#define TsTag_KOP  TsMakeTag('K','O','P',' ') /* Komi-Permyak */
#define TsTag_KOR  TsMakeTag('K','O','R',' ') /* Korean */
#define TsTag_KOZ  TsMakeTag('K','O','Z',' ') /* Komi-Zyrian */
#define TsTag_KPL  TsMakeTag('K','P','L',' ') /* Kpelle */
#define TsTag_KRI  TsMakeTag('K','R','I',' ') /* Krio */
#define TsTag_KRK  TsMakeTag('K','R','K',' ') /* Karakalpak */
#define TsTag_KRL  TsMakeTag('K','R','L',' ') /* Karelian */
#define TsTag_KRM  TsMakeTag('K','R','M',' ') /* Karaim */
#define TsTag_KRN  TsMakeTag('K','R','N',' ') /* Karen */
#define TsTag_KRT  TsMakeTag('K','R','T',' ') /* Koorete */
#define TsTag_KSH  TsMakeTag('K','S','H',' ') /* Kashmiri */
#define TsTag_KSI  TsMakeTag('K','S','I',' ') /* Khasi */
#define TsTag_KSM  TsMakeTag('K','S','M',' ') /* Kildin Sami */
#define TsTag_KUI  TsMakeTag('K','U','I',' ') /* Kui */
#define TsTag_KUL  TsMakeTag('K','U','L',' ') /* Kulvi */
#define TsTag_KUM  TsMakeTag('K','U','M',' ') /* Kumyk */
#define TsTag_KUR  TsMakeTag('K','U','R',' ') /* Kurdish */
#define TsTag_KUU  TsMakeTag('K','U','U',' ') /* Kurukh */
#define TsTag_KUY  TsMakeTag('K','U','Y',' ') /* Kuy */
#define TsTag_KYK  TsMakeTag('K','Y','K',' ') /* Koryak */
#define TsTag_LAD  TsMakeTag('L','A','D',' ') /* Ladin */
#define TsTag_LAH  TsMakeTag('L','A','H',' ') /* Lahuli */
#define TsTag_LAK  TsMakeTag('L','A','K',' ') /* Lak */
#define TsTag_LAM  TsMakeTag('L','A','M',' ') /* Lambani */
#define TsTag_LAO  TsMakeTag('L','A','O',' ') /* Lao */
#define TsTag_LAT  TsMakeTag('L','A','T',' ') /* Latin */
#define TsTag_LAZ  TsMakeTag('L','A','Z',' ') /* Laz */
#define TsTag_LCR  TsMakeTag('L','C','R',' ') /* L-Cree */
#define TsTag_LDK  TsMakeTag('L','D','K',' ') /* Ladakhi */
#define TsTag_LEZ  TsMakeTag('L','E','Z',' ') /* Lezgi */
#define TsTag_LIN  TsMakeTag('L','I','N',' ') /* Lingala */
#define TsTag_LMA  TsMakeTag('L','M','A',' ') /* Low Mari */
#define TsTag_LMB  TsMakeTag('L','M','B',' ') /* Limbu */
#define TsTag_LMW  TsMakeTag('L','M','W',' ') /* Lomwe */
#define TsTag_LSB  TsMakeTag('L','S','B',' ') /* Lower Sorbian */
#define TsTag_LSM  TsMakeTag('L','S','M',' ') /* Lule Sami */
#define TsTag_LTH  TsMakeTag('L','T','H',' ') /* Lithuanian */
#define TsTag_LUB  TsMakeTag('L','U','B',' ') /* Luba */
#define TsTag_LUG  TsMakeTag('L','U','G',' ') /* Luganda */
#define TsTag_LUH  TsMakeTag('L','U','H',' ') /* Luhya */
#define TsTag_LUO  TsMakeTag('L','U','O',' ') /* Luo */
#define TsTag_LVI  TsMakeTag('L','V','I',' ') /* Latvian */
#define TsTag_MAJ  TsMakeTag('M','A','J',' ') /* Majang */
#define TsTag_MAK  TsMakeTag('M','A','K',' ') /* Makua */
#define TsTag_MAL  TsMakeTag('M','A','L',' ') /* Malayalam Traditional */
#define TsTag_MAN  TsMakeTag('M','A','N',' ') /* Mansi */
#define TsTag_MAR  TsMakeTag('M','A','R',' ') /* Marathi */
#define TsTag_MAW  TsMakeTag('M','A','W',' ') /* Marwari */
#define TsTag_MBN  TsMakeTag('M','B','N',' ') /* Mbundu */
#define TsTag_MCH  TsMakeTag('M','C','H',' ') /* Manchu */
#define TsTag_MCR  TsMakeTag('M','C','R',' ') /* Moose Cree */
#define TsTag_MDE  TsMakeTag('M','D','E',' ') /* Mende */
#define TsTag_MEN  TsMakeTag('M','E','N',' ') /* Me'en */
#define TsTag_MIZ  TsMakeTag('M','I','Z',' ') /* Mizo */
#define TsTag_MKD  TsMakeTag('M','K','D',' ') /* Macedonian */
#define TsTag_MLE  TsMakeTag('M','L','E',' ') /* Male */
#define TsTag_MLG  TsMakeTag('M','L','G',' ') /* Malagasy */
#define TsTag_MLN  TsMakeTag('M','L','N',' ') /* Malinke */
#define TsTag_MLR  TsMakeTag('M','L','R',' ') /* Malayalam Reformed */
#define TsTag_MLY  TsMakeTag('M','L','Y',' ') /* Malay */
#define TsTag_MND  TsMakeTag('M','N','D',' ') /* Mandinka */
#define TsTag_MNG  TsMakeTag('M','N','G',' ') /* Mongolian */
#define TsTag_MNI  TsMakeTag('M','N','I',' ') /* Manipuri */
#define TsTag_MNK  TsMakeTag('M','N','K',' ') /* Maninka */
#define TsTag_MNX  TsMakeTag('M','N','X',' ') /* Manx Gaelic */
#define TsTag_MOK  TsMakeTag('M','O','K',' ') /* Moksha */
#define TsTag_MOL  TsMakeTag('M','O','L',' ') /* Moldavian */
#define TsTag_MON  TsMakeTag('M','O','N',' ') /* Mon */
#define TsTag_MOR  TsMakeTag('M','O','R',' ') /* Moroccan */
#define TsTag_MRI  TsMakeTag('M','R','I',' ') /* Maori */
#define TsTag_MTH  TsMakeTag('M','T','H',' ') /* Maithili */
#define TsTag_MTS  TsMakeTag('M','T','S',' ') /* Maltese */
#define TsTag_MUN  TsMakeTag('M','U','N',' ') /* Mundari */
#define TsTag_NAG  TsMakeTag('N','A','G',' ') /* Naga-Assamese */
#define TsTag_NAN  TsMakeTag('N','A','N',' ') /* Nanai */
#define TsTag_NAS  TsMakeTag('N','A','S',' ') /* Naskapi */
#define TsTag_NCR  TsMakeTag('N','C','R',' ') /* N-Cree */
#define TsTag_NDB  TsMakeTag('N','D','B',' ') /* Ndebele */
#define TsTag_NDG  TsMakeTag('N','D','G',' ') /* Ndonga */
#define TsTag_NEP  TsMakeTag('N','E','P',' ') /* Nepali */
#define TsTag_NEW  TsMakeTag('N','E','W',' ') /* Newari */
#define TsTag_NHC  TsMakeTag('N','H','C',' ') /* Norway House Cree */
#define TsTag_NIS  TsMakeTag('N','I','S',' ') /* Nisi */
#define TsTag_NIU  TsMakeTag('N','I','U',' ') /* Niuean */
#define TsTag_NKL  TsMakeTag('N','K','L',' ') /* Nkole */
#define TsTag_NLD  TsMakeTag('N','L','D',' ') /* Dutch */
#define TsTag_NOG  TsMakeTag('N','O','G',' ') /* Nogai */
#define TsTag_NOR  TsMakeTag('N','O','R',' ') /* Norwegian */
#define TsTag_NSM  TsMakeTag('N','S','M',' ') /* Northern Sami */
#define TsTag_NTA  TsMakeTag('N','T','A',' ') /* Northern Tai */
#define TsTag_NTO  TsMakeTag('N','T','O',' ') /* Esperanto */
#define TsTag_NYN  TsMakeTag('N','Y','N',' ') /* Nynorsk */
#define TsTag_OCR  TsMakeTag('O','C','R',' ') /* Oji-Cree */
#define TsTag_OJB  TsMakeTag('O','J','B',' ') /* Ojibway */
#define TsTag_ORI  TsMakeTag('O','R','I',' ') /* Oriya */
#define TsTag_ORO  TsMakeTag('O','R','O',' ') /* Oromo */
#define TsTag_OSS  TsMakeTag('O','S','S',' ') /* Ossetian */
#define TsTag_PAA  TsMakeTag('P','A','A',' ') /* Palestinian Aramaic */
#define TsTag_PAL  TsMakeTag('P','A','L',' ') /* Pali */
#define TsTag_PAN  TsMakeTag('P','A','N',' ') /* Punjabi */
#define TsTag_PAP  TsMakeTag('P','A','P',' ') /* Palpa */
#define TsTag_PAS  TsMakeTag('P','A','S',' ') /* Pashto */
#define TsTag_PGR  TsMakeTag('P','G','R',' ') /* Polytonic Greek */
#define TsTag_PIL  TsMakeTag('P','I','L',' ') /* Pilipino */
#define TsTag_PLG  TsMakeTag('P','L','G',' ') /* Palaung */
#define TsTag_PLK  TsMakeTag('P','L','K',' ') /* Polish */
#define TsTag_PRO  TsMakeTag('P','R','O',' ') /* Provencal */
#define TsTag_PTG  TsMakeTag('P','T','G',' ') /* Portuguese */
#define TsTag_QIN  TsMakeTag('Q','I','N',' ') /* Chin */
#define TsTag_RAJ  TsMakeTag('R','A','J',' ') /* Rajasthani */
#define TsTag_RCR  TsMakeTag('R','C','R',' ') /* R-Cree */
#define TsTag_RBU  TsMakeTag('R','B','U',' ') /* Russian Buriat */
#define TsTag_RIA  TsMakeTag('R','I','A',' ') /* Riang */
#define TsTag_RMS  TsMakeTag('R','M','S',' ') /* Rhaeto-Romanic */
#define TsTag_ROM  TsMakeTag('R','O','M',' ') /* Romanian */
#define TsTag_ROY  TsMakeTag('R','O','Y',' ') /* Romany */
#define TsTag_RSY  TsMakeTag('R','S','Y',' ') /* Rusyn */
#define TsTag_RUA  TsMakeTag('R','U','A',' ') /* Ruanda */
#define TsTag_RUS  TsMakeTag('R','U','S',' ') /* Russian */
#define TsTag_SAD  TsMakeTag('S','A','D',' ') /* Sadri */
#define TsTag_SAN  TsMakeTag('S','A','N',' ') /* Sanskrit */
#define TsTag_SAT  TsMakeTag('S','A','T',' ') /* Santali */
#define TsTag_SAY  TsMakeTag('S','A','Y',' ') /* Sayisi */
#define TsTag_SEK  TsMakeTag('S','E','K',' ') /* Sekota */
#define TsTag_SEL  TsMakeTag('S','E','L',' ') /* Selkup */
#define TsTag_SGO  TsMakeTag('S','G','O',' ') /* Sango */
#define TsTag_SHN  TsMakeTag('S','H','N',' ') /* Shan */
#define TsTag_SIB  TsMakeTag('S','I','B',' ') /* Sibe */
#define TsTag_SID  TsMakeTag('S','I','D',' ') /* Sidamo */
#define TsTag_SIG  TsMakeTag('S','I','G',' ') /* Silte Gurage */
#define TsTag_SKS  TsMakeTag('S','K','S',' ') /* Skolt Sami */
#define TsTag_SKY  TsMakeTag('S','K','Y',' ') /* Slovak */
#define TsTag_SLA  TsMakeTag('S','L','A',' ') /* Slavey */
#define TsTag_SLV  TsMakeTag('S','L','V',' ') /* Slovenian */
#define TsTag_SML  TsMakeTag('S','M','L',' ') /* Somali */
#define TsTag_SMO  TsMakeTag('S','M','O',' ') /* Samoan */
#define TsTag_SNA  TsMakeTag('S','N','A',' ') /* Sena */
#define TsTag_SND  TsMakeTag('S','N','D',' ') /* Sindhi */
#define TsTag_SNH  TsMakeTag('S','N','H',' ') /* Sinhalese */
#define TsTag_SNK  TsMakeTag('S','N','K',' ') /* Soninke */
#define TsTag_SOG  TsMakeTag('S','O','G',' ') /* Sodo Gurage */
#define TsTag_SOT  TsMakeTag('S','O','T',' ') /* Sotho */
#define TsTag_SQI  TsMakeTag('S','Q','I',' ') /* Albanian */
#define TsTag_SRB  TsMakeTag('S','R','B',' ') /* Serbian */
#define TsTag_SRK  TsMakeTag('S','R','K',' ') /* Saraiki */
#define TsTag_SRR  TsMakeTag('S','R','R',' ') /* Serer */
#define TsTag_SSL  TsMakeTag('S','S','L',' ') /* South Slavey */
#define TsTag_SSM  TsMakeTag('S','S','M',' ') /* Southern Sami */
#define TsTag_SUR  TsMakeTag('S','U','R',' ') /* Suri */
#define TsTag_SVA  TsMakeTag('S','V','A',' ') /* Svan */
#define TsTag_SVE  TsMakeTag('S','V','E',' ') /* Swedish */
#define TsTag_SWA  TsMakeTag('S','W','A',' ') /* Swadaya Aramaic */
#define TsTag_SWK  TsMakeTag('S','W','K',' ') /* Swahili */
#define TsTag_SWZ  TsMakeTag('S','W','Z',' ') /* Swazi */
#define TsTag_SXT  TsMakeTag('S','X','T',' ') /* Sutu */
#define TsTag_SYR  TsMakeTag('S','Y','R',' ') /* Syriac */
#define TsTag_TAB  TsMakeTag('T','A','B',' ') /* Tabasaran */
#define TsTag_TAJ  TsMakeTag('T','A','J',' ') /* Tajiki */
#define TsTag_TAM  TsMakeTag('T','A','M',' ') /* Tamil */
#define TsTag_TAT  TsMakeTag('T','A','T',' ') /* Tatar */
#define TsTag_TCR  TsMakeTag('T','C','R',' ') /* TH-Cree */
#define TsTag_TEL  TsMakeTag('T','E','L',' ') /* Telugu */
#define TsTag_TGN  TsMakeTag('T','G','N',' ') /* Tongan */
#define TsTag_TGR  TsMakeTag('T','G','R',' ') /* Tigre */
#define TsTag_TGY  TsMakeTag('T','G','Y',' ') /* Tigrinya */
#define TsTag_THA  TsMakeTag('T','H','A',' ') /* Thai */
#define TsTag_THT  TsMakeTag('T','H','T',' ') /* Tahitian */
#define TsTag_TIB  TsMakeTag('T','I','B',' ') /* Tibetan */
#define TsTag_TKM  TsMakeTag('T','K','M',' ') /* Turkmen */
#define TsTag_TMN  TsMakeTag('T','M','N',' ') /* Temne */
#define TsTag_TNA  TsMakeTag('T','N','A',' ') /* Tswana */
#define TsTag_TNE  TsMakeTag('T','N','E',' ') /* Tundra Nenets */
#define TsTag_TNG  TsMakeTag('T','N','G',' ') /* Tonga */
#define TsTag_TOD  TsMakeTag('T','O','D',' ') /* Todo */
#define TsTag_TRK  TsMakeTag('T','R','K',' ') /* Turkish */
#define TsTag_TSG  TsMakeTag('T','S','G',' ') /* Tsonga */
#define TsTag_TUA  TsMakeTag('T','U','A',' ') /* Turoyo Aramaic */
#define TsTag_TUL  TsMakeTag('T','U','L',' ') /* Tulu */
#define TsTag_TUV  TsMakeTag('T','U','V',' ') /* Tuvin */
#define TsTag_TWI  TsMakeTag('T','W','I',' ') /* Twi */
#define TsTag_UDM  TsMakeTag('U','D','M',' ') /* Udmurt */
#define TsTag_UKR  TsMakeTag('U','K','R',' ') /* Ukrainian */
#define TsTag_URD  TsMakeTag('U','R','D',' ') /* Urdu */
#define TsTag_USB  TsMakeTag('U','S','B',' ') /* Upper Sorbian */
#define TsTag_UYG  TsMakeTag('U','Y','G',' ') /* Uyghur */
#define TsTag_UZB  TsMakeTag('U','Z','B',' ') /* Uzbek */
#define TsTag_VEN  TsMakeTag('V','E','N',' ') /* Venda */
#define TsTag_VIT  TsMakeTag('V','I','T',' ') /* Vietnamese */
#define TsTag_WA   TsMakeTag('W','A',' ',' ') /* Wa */
#define TsTag_WAG  TsMakeTag('W','A','G',' ') /* Wagdi */
#define TsTag_WCR  TsMakeTag('W','C','R',' ') /* West-Cree */
#define TsTag_WEL  TsMakeTag('W','E','L',' ') /* Welsh */
#define TsTag_WLF  TsMakeTag('W','L','F',' ') /* Wolof */
#define TsTag_XHS  TsMakeTag('X','H','S',' ') /* Xhosa */
#define TsTag_YAK  TsMakeTag('Y','A','K',' ') /* Yakut */
#define TsTag_YBA  TsMakeTag('Y','B','A',' ') /* Yoruba */
#define TsTag_YCR  TsMakeTag('Y','C','R',' ') /* Y-Cree */
#define TsTag_YIC  TsMakeTag('Y','I','C',' ') /* Yi Classic */
#define TsTag_YIM  TsMakeTag('Y','I','M',' ') /* Yi Modern */
#define TsTag_ZHP  TsMakeTag('Z','H','P',' ') /* Chinese Phonetic */
#define TsTag_ZHS  TsMakeTag('Z','H','S',' ') /* Chinese Simplified */
#define TsTag_ZHT  TsMakeTag('Z','H','T',' ') /* Chinese Traditional */
#define TsTag_ZND  TsMakeTag('Z','N','D',' ') /* Zande */
#define TsTag_ZUL  TsMakeTag('Z','U','L',' ') /* Zulu */

TS_END_HEADER

#endif /* OTLANGS_H */
