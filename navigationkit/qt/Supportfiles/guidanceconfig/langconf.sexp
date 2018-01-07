; Language Configuration File
;
; Refer to following Wiki Pages:
; http://en.wikipedia.org/wiki/ISO_3166-1
; http://en.wikipedia.org/wiki/List_of_ISO_639-1_codes
; http://en.wikipedia.org/wiki/List_of_languages_by_writing_system#Latin_script
; http://www.ivona.com/en/voices-list/
;
; Sorted by NAVTEQ language code.
;
; Note: "trans" value here has two meanings according to NAVTEQ source data.
;       "..X(or T)" means latin transliterated name, such as "RUS"->"RUX", or
;       "GRE"->"GRT".
;       "..E" means to use English phonetic to simulate local pronunciation,
;       such as "THA"->"THE".
;
; Ver = 1.0.0
;
(langconf()
; IVONA currently supports:
; ENG
    (lang (code "en-US" name "English (United States)" latin 1 trans 0))
; UKE, IRE(Irish English)
    (lang (code "en-GB" name "English (British)" latin 1 trans 0))
; CFR(Canadian French)
    (lang (code "fr-CA" name "French (Canadian)" latin 1 trans 0))
; DAN
    (lang (code "da-DK" name "Danish" latin 1 trans 0))
; DUT
    (lang (code "nl-NL" name "Dutch" latin 1 trans 0))
; FRE
    (lang (code "fr-FR" name "French" latin 1 trans 0))
; GER
    (lang (code "de-DE" name "German" latin 1 trans 0))
; ICE
    (lang (code "is-IS" name "Icelandic" latin 1 trans 0))
; ITA
    (lang (code "it-IT" name "Italian" latin 1 trans 0))
; POL[POX]
    (lang (code "pl-PL" name "Polish" latin 1 trans 0))
; POR (Brazilian Portuguese)
    (lang (code "pt-BR" name "Portuguese (Brazilian)" latin 1 trans 0))
; RUM, MOL[MOX]
    (lang (code "ro-RO" name "Romanian" latin 1 trans 0))
; SPA, NAS(North American Spanish)
    (lang (code "es-ES" name "Spanish (Castilian)" latin 1 trans 0))
; WEL
    (lang (code "cy-GB" name "Welsh" latin 1 trans 0))
; IVONA currently doesn't support:
; ALB
    (lang (code "sq-AL" name "Albanian" latin 1 trans 0))
; ARA[trans:ARE]
    (lang (code "ar-WW" name "Arabic" latin 0 trans 1))
; BAQ
    (lang (code "eu-ES" name "Basque" latin 1 trans 0))
; BEL[trans:BEX]
    (lang (code "be-BY" name "Belarusian" latin 0 trans 1))
; BOS
    (lang (code "bs-BA" name "Bosnian" latin 1 trans 0))
; CAT
    (lang (code "ca-ES" name "Catalan" latin 1 trans 0))
; CHI[trans:PYN], CHT
    (lang (code "zh-CN" name "Chinese" latin 0 trans 1))
; CZE[CZX]
    (lang (code "cs-CZ" name "Czech" latin 1 trans 0))
; ENG (Australian English)
    (lang (code "en-AU" name "English (Australian)" latin 1 trans 0))
; EST
    (lang (code "et-EE" name "Estonian" latin 1 trans 0))
; FIN
    (lang (code "fi-FI" name "Finnish" latin 1 trans 0))
; GLE
    (lang (code "ga-IE" name "Irish" latin 1 trans 0))
; GLG
    (lang (code "gl-ES" name "Galician" latin 1 trans 0))
; GRE[trans:GRT,GRX]
    (lang (code "el-GR" name "Greek" latin 0 trans 1))
; HEB[trans:HEX]
    (lang (code "he-IL" name "Hebrew" latin 0 trans 1))
; HUN[HUX]
    (lang (code "hu-HU" name "Hungarian" latin 1 trans 0))
; LAV
    (lang (code "lv-LV" name "Latvian" latin 1 trans 0))
; LIT
    (lang (code "lt-LT" name "Lithuanian" latin 1 trans 0))
; MAC[trans:MAX]
    (lang (code "mk-LT" name "Macedonian" latin 0 trans 1))
; MAY
    (lang (code "ms-MY" name "Malay" latin 1 trans 0))
; MLT
    (lang (code "mt-MT" name "Maltese" latin 1 trans 0))
; NOR
    (lang (code "no-NO" name "Norwegian" latin 1 trans 0))
; POR
    (lang (code "pt-PT" name "Portuguese" latin 1 trans 0))
; RUS[trans:RUX,RST]
    (lang (code "ru-RU" name "Russian" latin 0 trans 1))
; SCR[CRX]
    (lang (code "hr-HR" name "Croatian" latin 1 trans 0))
; SLO[SLX]
    (lang (code "sk-SK" name "Slovak" latin 1 trans 0))
; SLV[SIX]
    (lang (code "sl-SI" name "Slovene" latin 1 trans 0))
; SRB[SCT, SCX], MNE[MNX]
    (lang (code "sr-RS" name "Serbian" latin 1 trans 0))
; SWE
    (lang (code "sv-SE" name "Swedish" latin 1 trans 0))
; THA[trans:THE]
    (lang (code "th-TH" name "Thai" latin 0 trans 1))
; TUR[TUX]
    (lang (code "tr-TR" name "Turkish" latin 1 trans 0))
; UKR[trans:UKT, UKX]
    (lang (code "uk-UA" name "Ukrainian" latin 0 trans 1))
; VIE[VIX]
    (lang (code "vi-VN" name "Vietnamese" latin 1 trans 0))
)
