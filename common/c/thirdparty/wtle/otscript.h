/*****************************************************************************
 *
 *  otscript.h - OpenType scripts.
 *
 *  Copyright (C) 2002 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef OTSCRIPT_H
#define OTSCRIPT_H

#include "tstag.h"

TS_BEGIN_HEADER

/******************************************************************************
 *
 *  Constant TsTag declarations and initializations of known OT script
 *  designations.
 *
 *  <GROUP opentype>
 */
#define TsTag_arab TsMakeTag('a','r','a','b') /* Arabic */
#define TsTag_armn TsMakeTag('a','r','m','n') /* Armenian */
#define TsTag_bali TsMakeTag('b','a','l','i') /* Balinese */
#define TsTag_beng TsMakeTag('b','e','n','g') /* Bengali */
#define TsTag_bng2 TsMakeTag('b','n','g','2') /* Bengali v.2 */
#define TsTag_bopo TsMakeTag('b','o','p','o') /* Bopomofo */
#define TsTag_brai TsMakeTag('b','r','a','i') /* Braille */
#define TsTag_bugi TsMakeTag('b','u','g','i') /* Buginese */
#define TsTag_buhd TsMakeTag('b','u','h','d') /* Buhid */
#define TsTag_byzm TsMakeTag('b','y','z','m') /* Byzantine Music */
#define TsTag_cans TsMakeTag('c','a','n','s') /* Canadian Syllabics */
#define TsTag_cari TsMakeTag('c','a','r','i') /* Carian */
#define TsTag_cham TsMakeTag('c','h','a','m') /* Cham */
#define TsTag_cher TsMakeTag('c','h','e','r') /* Cherokee */
#define TsTag_copt TsMakeTag('c','o','p','t') /* Coptic */
#define TsTag_cprt TsMakeTag('c','p','r','t') /* Cypriot Syllabary */
#define TsTag_cyrl TsMakeTag('c','y','r','l') /* Cyrillic */
#define TsTag_dev2 TsMakeTag('d','e','v','2') /* Devanagari v.2 */
#define TsTag_deva TsMakeTag('d','e','v','a') /* Devanagari */
#define TsTag_DFLT TsMakeTag('D','F','L','T') /* Default */
#define TsTag_dsrt TsMakeTag('d','s','r','t') /* Deseret */
#define TsTag_ethi TsMakeTag('e','t','h','i') /* Ethiopic */
#define TsTag_geor TsMakeTag('g','e','o','r') /* Georgian */
#define TsTag_gjr2 TsMakeTag('g','j','r','2') /* Gujarati v.2 */
#define TsTag_glag TsMakeTag('g','l','a','g') /* Glagolitic */
#define TsTag_goth TsMakeTag('g','o','t','h') /* Gothic */
#define TsTag_grek TsMakeTag('g','r','e','k') /* Greek */
#define TsTag_gujr TsMakeTag('g','u','j','r') /* Gujarati */
#define TsTag_gur2 TsMakeTag('g','u','r','2') /* Gurmukhi v.2 */
#define TsTag_guru TsMakeTag('g','u','r','u') /* Gurmukhi */
#define TsTag_hang TsMakeTag('h','a','n','g') /* Hangul */
#define TsTag_hani TsMakeTag('h','a','n','i') /* CJK Ideographic */
#define TsTag_hano TsMakeTag('h','a','n','o') /* Hanunoo */
#define TsTag_hebr TsMakeTag('h','e','b','r') /* Hebrew */
#define TsTag_ital TsMakeTag('i','t','a','l') /* Old Italic */
#define TsTag_jamo TsMakeTag('j','a','m','o') /* Hangul Jamo */
#define TsTag_java TsMakeTag('j','a','v','a') /* Javanese */
#define TsTag_kali TsMakeTag('k','a','l','i') /* Kayah Li */
#define TsTag_kana TsMakeTag('k','a','n','a') /* Hiragana and Katakana */
#define TsTag_khar TsMakeTag('k','h','a','r') /* Kharosthi */
#define TsTag_khmr TsMakeTag('k','h','m','r') /* Khmer */
#define TsTag_knd2 TsMakeTag('k','n','d','2') /* Kannada v.2 */
#define TsTag_knda TsMakeTag('k','n','d','a') /* Kannada */
#define TsTag_lao  TsMakeTag('l','a','o',' ') /* Lao */
#define TsTag_latn TsMakeTag('l','a','t','n') /* Latin */
#define TsTag_lepc TsMakeTag('l','e','p','c') /* Lepcha */
#define TsTag_limb TsMakeTag('l','i','m','b') /* Limbu */
#define TsTag_linb TsMakeTag('l','i','n','b') /* Linear B */
#define TsTag_lyci TsMakeTag('l','y','c','i') /* Lycian */
#define TsTag_lydi TsMakeTag('l','y','d','i') /* Lydian */
#define TsTag_math TsMakeTag('m','a','t','h') /* Mathematical Alphanumeric Symbols */
#define TsTag_mlm2 TsMakeTag('m','l','m','2') /* Malayalam v.2 */
#define TsTag_mlym TsMakeTag('m','l','y','m') /* Malayalam */
#define TsTag_mong TsMakeTag('m','o','n','g') /* Mongolian */
#define TsTag_musc TsMakeTag('m','u','s','c') /* Musical Symbols */
#define TsTag_mymr TsMakeTag('m','y','m','r') /* Myanmar */
#define TsTag_nko  TsMakeTag('n','k','o',' ') /* N'Ko */
#define TsTag_ogam TsMakeTag('o','g','a','m') /* Ogham */
#define TsTag_olck TsMakeTag('o','l','c','k') /* Ol Chiki */
#define TsTag_ory2 TsMakeTag('o','r','y','2') /* Oriya v.2 */
#define TsTag_orya TsMakeTag('o','r','y','a') /* Oriya */
#define TsTag_osma TsMakeTag('o','s','m','a') /* Osmanya */
#define TsTag_phag TsMakeTag('p','h','a','g') /* Phags-pa */
#define TsTag_phnx TsMakeTag('p','h','n','x') /* Phoenician */
#define TsTag_rjng TsMakeTag('r','j','n','g') /* Rejang */
#define TsTag_runr TsMakeTag('r','u','n','r') /* Runic */
#define TsTag_saur TsMakeTag('s','a','u','r') /* Saurashtra */
#define TsTag_shaw TsMakeTag('s','h','a','w') /* Shavian */
#define TsTag_sinh TsMakeTag('s','i','n','h') /* Sinhala */
#define TsTag_sund TsMakeTag('s','u','n','d') /* Sundanese */
#define TsTag_sylo TsMakeTag('s','y','l','o') /* Syloti Nagri */
#define TsTag_syrc TsMakeTag('s','y','r','c') /* Syriac */
#define TsTag_tagb TsMakeTag('t','a','g','b') /* Tagbanwa */
#define TsTag_tale TsMakeTag('t','a','l','e') /* Tai Le */
#define TsTag_talu TsMakeTag('t','a','l','u') /* New Tai Lue */
#define TsTag_taml TsMakeTag('t','a','m','l') /* Tamil */
#define TsTag_tel2 TsMakeTag('t','e','l','2') /* Telugu v.2 */
#define TsTag_telu TsMakeTag('t','e','l','u') /* Telugu */
#define TsTag_tfng TsMakeTag('t','f','n','g') /* Tifinagh */
#define TsTag_tglg TsMakeTag('t','g','l','g') /* Tagalog */
#define TsTag_thaa TsMakeTag('t','h','a','a') /* Thaana */
#define TsTag_thai TsMakeTag('t','h','a','i') /* Thai */
#define TsTag_tibt TsMakeTag('t','i','b','t') /* Tibetan */
#define TsTag_tml2 TsMakeTag('t','m','l','2') /* Tamil v.2 */
#define TsTag_ugar TsMakeTag('u','g','a','r') /* Ugaritic Cuneiform */
#define TsTag_vai  TsMakeTag('v','a','i',' ') /* Vai */
#define TsTag_xpeo TsMakeTag('x','p','e','o') /* Old Persian Cuneiform */
#define TsTag_xsux TsMakeTag('x','s','u','x') /* Sumero-Akkadian Cuneiform */
#define TsTag_yi   TsMakeTag('y','i',' ',' ') /* Yi */

TS_END_HEADER

#endif /* OTSCRIPT_H */
