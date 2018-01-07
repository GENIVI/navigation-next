(nav-instruct (command-set-version [uint]3 media-type text/x-nim-formatted media-sub-type "")
;
; Define the basic instruction for each turn.
; This allows many subsequent rules to be combined since they 
; only differ in the basic turn command
;
  (turn-sounds ()
; TR.R - Right Turn
    (define-turn-sound (turn TR.R pos *)
      (text (data "VUELTA A LA DERECHA")))
; TR.L - Left Turn
    (define-turn-sound (turn TR.L pos *)
      (text (data "VUELTA A LA IZQUIERDA")))
; UT. - U Turn
    (define-turn-sound (turn UT. pos *)
      (text (data "DÉ VUELTA EN U")))
; EN.R - Enter Highway on the Right
    (define-turn-sound (turn EN.R pos *)
      (text (data "")))
; EN.L - Enter Highway on the Left
    (define-turn-sound (turn EN.L pos *)
      (text (data "")))
; EN.  - Enter Highway straight ahead
    (define-turn-sound (turn EN. pos *)
      (text (data "")))
; EX. - Exit Highway
    (define-turn-sound (turn EX. pos *)
      (text (data "SALGA")))
; EX.R - Exit Highway on the Right
    (define-turn-sound (turn EX.R pos *)
      (text (data "SALGA A LA DERECHA")))
; EX.L - Exit Highway on the Left
    (define-turn-sound (turn EX.L pos *)
      (text (data "SALGA A LA IZQUIERDA")))
; MR.R - Merge into traffic on the right
    (define-turn-sound (turn MR.R pos *)
      (text (data "INCORPÓRESE A LA DERECHA")))
; MR.L - Exit Highway on the Left
    (define-turn-sound (turn MR.L pos *)
      (text (data "INCORPÓRESE A LA IZQUIERDA")))
; KP.R - Keep to the right (Split)
    (define-turn-sound (turn KP.R pos *)
      (text (data "MANTÉNGASE A LA DERECHA")))
; KP.L - Keep to the left (Split)
    (define-turn-sound (turn KP.L pos *)
      (text (data "MANTÉNGASE A LA IZQUIERDA")))
; KP. - Keep to the center (Split)
    (define-turn-sound (turn KP. pos *)
      (text (data "SIGA DERECHO")))
; NC. - Name Change (Continue)
    (define-turn-sound (turn NC. pos *)
      (text (data "CONTINÚE")))
; RE. - Enter Traffic Circle
    (define-turn-sound (turn RE. pos *)
      (text (data "INCORPÓRESE A LA GLORIETA")))
; RX.1 - Exit Traffic Cirle 1st Exit
    (define-turn-sound (turn RX.1 pos *)
      (text (data "Tome la PRIMERA SALIDA de la GLORIETA")))
; RX.2 - Exit Traffic Cirle 2nd Exit
    (define-turn-sound (turn RX.2 pos *)
      (text (data "Tome la SEGUNDA SALIDA de la GLORIETA")))
; RX.3 - Exit Traffic Cirle 3rd Exit
    (define-turn-sound (turn RX.3 pos *)
      (text (data "Tome la TERCERA SALIDA de la GLORIETA")))
; RX.4 - Exit Traffic Cirle 4th Exit
    (define-turn-sound (turn RX.4 pos *)
      (text (data "Tome la CUARTA SALIDA de la GLORIETA")))
; RX.5 - Exit Traffic Cirle 5th Exit
    (define-turn-sound (turn RX.5 pos *)
      (text (data "Tome la QUINTA SALIDA de la GLORIETA")))
; RX.6 - Exit Traffic Cirle 6th Exit
    (define-turn-sound (turn RX.6 pos *)
      (text (data "Tome la SEXTA SALIDA de la GLORIETA")))
; RX.7 - Exit Traffic Cirle 7th Exit
    (define-turn-sound (turn RX.7 pos *)
      (text (data "Tome la SÉPTIMA SALIDA de la GLORIETA")))
; RX.8 - Exit Traffic Cirle 8th Exit
    (define-turn-sound (turn RX.8 pos *)
      (text (data "Tome la OCTAVA SALIDA de la GLORIETA")))
; RX.9 - Exit Traffic Cirle 9th Exit
    (define-turn-sound (turn RX.9 pos *)
      (text (data "Tome la NOVENA SALIDA de la GLORIETA")))
; RX.10 - Exit Traffic Cirle 10th Exit
    (define-turn-sound (turn RX.10 pos *)
      (text (data "Tome la DÉCIMA SALIDA de la GLORIETA")))
; RT. - Drive straight through rotary
    (define-turn-sound (turn RT. pos *)
      (text (data "En la GLORIETA, siga DERECHO")))
; ES.L - Enter Stairs
    (define-turn-sound (turn ES.L pos *)
      (text (data "Toma las ESCALERAS a la IZQUIERDA")))
; ES.R - Enter Stairs
    (define-turn-sound (turn ES.R pos *)
      (text (data "Toma las ESCALERAS a la DERECHA")))
; ES. - Enter Stairs
    (define-turn-sound (turn ES. pos *)
      (text (data "Toma las ESCALERAS ENFRENTE")))
; EE.L - Enter Escalator
    (define-turn-sound (turn EE.L pos *)
      (text (data "Toma las ESCALERAS ELÉCTRICAS a la IZQUIERDA")))
; EE.R - Enter Escalator
    (define-turn-sound (turn EE.R pos *)
      (text (data "Toma las ESCALERAS ELÉCTRICAS A LA DERECHA")))
; EE. - Enter Escalator
    (define-turn-sound (turn EE. pos *)
      (text (data "Toma las ESCALERAS ELÉCTRICAS ENFRENTE")))
; NR.R - Enter Private Roadway on the Right
    (define-turn-sound (turn NR.R pos *)
      (text (data "Tome el camino privado a la DERECHA")))
; NR.L - Enter Private Roadway on the Left
    (define-turn-sound (turn NR.L pos *)
      (text (data "Tome el camino privado a la IZQUIERDA")))
; NR.  - Enter Private Roadway Straight Ahead
    (define-turn-sound (turn NR. pos *)
      (text (data "Tome el camino privado ENFRENTE")))
; BE.R - Cross Bridge  on the Right
    (define-turn-sound (turn BE.R pos *)
      (text (data "Cruce el PUENTE a su DERECHA")))
; BE.L - Cross Bridge on the Left
    (define-turn-sound (turn BE.L pos *)
      (text (data "Cruce el PUENTE a su IZQUIERDA")))
; BE.  - Cross Bridge Straight Ahead
    (define-turn-sound (turn BE. pos *)
      (text (data "Cruce el PUENTE ENFRENTE")))
; TE.R - Enter Tunnel  on the Right
    (define-turn-sound (turn TE.R pos *)
      (text (data "Entre en el TÚNEL A LA DERECHA")))
; TE.L - Enter Tunnel on the Left
    (define-turn-sound (turn TE.L pos *)
      (text (data "Entre en el TÚNEL A LA IZQUIERDA")))
; TE.  - Enter Tunnel Straight Ahead
    (define-turn-sound (turn TE. pos *)
      (text (data "Entre en EL TÚNEL ENFRENTE")))
; ER.R - Take the Ramp on the Right
    (define-turn-sound (turn ER.R pos *)
      (text (data "Tome la RAMPA DE ACCESO A LA DERECHA")))
; ER.L - Take the Ramp on the Left
    (define-turn-sound (turn ER.L pos *)
      (text (data "Tome la RAMPA DE ACCESO A LA IZQUIERDA")))
; ER.  - Take the Ramp Straight Ahead
    (define-turn-sound (turn ER. pos *)
      (text (data "Tome RAMPA DE ACCESO ENFRENTE")))
; PE.  - Continue on Foot
    (define-turn-sound (turn PE. pos *)
      (text (data "SIGA a PIE")))
; FE. - Enter a ferry 
    (define-turn-sound (turn FE. pos *)
      (text (data "Tome un FERRY")))
; FX. - Exit ferry
    (define-turn-sound (turn FX. pos *)
      (text (data "DESEMBARQUE DEL FERRY")))
; EC. - Enter Country straight ahead
      (define-turn-sound (turn EC. pos *)
        (text (data "Entre al PAÍS ENFRENTE")))
; EC.R - Enter Country on the Right
      (define-turn-sound (turn EC.R pos *)
        (text (data "Entre al PAÍS a la DERECHA")))
; EC.L - Enter Country on the Left
      (define-turn-sound (turn EC.L pos *)
      (text (data "Entre al PAÍS a la IZQUIERDA"))))
;
; Define the transition sounds associated with each type of turn
; This is where the template differs from the en-gb/us
;

  (transition-sounds ()
    (define-transition-sound (turn EN.R|EN.L|EN.|MR.R|MR.L pos *)
      (text (data " en ")))
    (define-transition-sound (turn FE. pos *)
      (text (data " ")))
    (define-transition-sound (turn * pos *)
      (text (data " en "))))

 
 
; Define translations for unnamed segments, currently numbered 0 through 7
    (reserved-phrases ()
	(reserved (unnamed-road-type [uint]0 unnamed-road-phrase "Acceso"))
	(reserved (unnamed-road-type [uint]1 unnamed-road-phrase "Una carretera local"))
	(reserved (unnamed-road-type [uint]2 unnamed-road-phrase "Un túnel"))
	(reserved (unnamed-road-type [uint]3 unnamed-road-phrase "Un ferry"))
	(reserved (unnamed-road-type [uint]4 unnamed-road-phrase "Un puente"))
	(reserved (unnamed-road-type [uint]5 unnamed-road-phrase "Un paso peatonal"))
	(reserved (unnamed-road-type [uint]6 unnamed-road-phrase "Escaleras"))
	(reserved (unnamed-road-type [uint]7 unnamed-road-phrase "Escaleras eléctricas"))
	(reserved (unnamed-road-type [uint]8 unnamed-road-phrase "Paso peatonal"))
	(reserved (unnamed-road-type [uint]9 unnamed-road-phrase "Paso peatonal cubierto"))
	(reserved (unnamed-road-type [uint]10 unnamed-road-phrase "Escaleras eléctricas"))
	(reserved (unnamed-road-type [uint]11 unnamed-road-phrase "Parque"))
	(reserved (unnamed-road-type [uint]12 unnamed-road-phrase "Plaza"))
	(reserved (unnamed-road-type [uint]13 unnamed-road-phrase "Escaleras"))
   )
;
; Define the additional stack text.
;
  (section (name stack-text)
    (instruction (turn * stacknext "RX.1")
      (stack-dist ())
      (text (data " / 1ª")))
    (instruction (turn * stacknext "RX.2")
      (stack-dist ())
      (text (data " / 2ª")))
    (instruction (turn * stacknext "RX.3")
      (stack-dist ())
      (text (data " / 3ª")))
    (instruction (turn * stacknext "RX.4")
      (stack-dist ())
      (text (data " / 4ª")))
    (instruction (turn * stacknext "RX.5")
      (stack-dist ())
      (text (data " / 5ª")))
    (instruction (turn * stacknext "RX.6")
      (stack-dist ())
      (text (data " / 6ª")))
    (instruction (turn * stacknext "RX.7")
      (stack-dist ())
      (text (data " / 7ª")))
    (instruction (turn * stacknext "RX.8")
      (stack-dist ())
      (text (data " / 8ª")))
    (instruction (turn * stacknext "RX.9")
      (stack-dist ())
      (text (data " / 9ª")))
    (instruction (turn * stacknext "RX.10")
      (stack-dist ())
      (text (data " / 10ª")))
     (instruction (turn * stacknext *)
      (stack-dist ()))
   )

;
; Define the text for the arrival screen
;
  (section (name arrival-text)
    (instruction (turn DT.|DT.L|DT.R)
      (font (name large))
      (color (red [uint]0 green [uint]0 blue [uint]0))
      (if (turn DT. stacknext * next *)
        (text (data "Ha llegado a su destino:")))
      (if (turn DT.L stacknext * next *)
        (text (data "Ha LLEGADO a su destino; está a su IZQUIERDA:")))	
      (if (turn DT.R stacknext * next *)
        (text (data "Ha LLEGADO a su destino; está a su DERECHA:")))	
      (text (data " "))
      (newline ())
      (if-destination-name ()
        (destination-name ())
        (newline ()))
      (destination-address ()
        (multiline ()))
      (if-destination-phonenum ()
	    (destination-phonenum ()))))
	   
;
; Define the arrival text for the secondary screen
;

  (section (name sec-arrival-text)
    (instruction (turn DT.|DT.L|DT.R)
      (font (name custom-4))	; the app stores the small secondary display font in custom-4
      (text (data "Llegó"))))
      
;
; Define the secondary screen startup case text
;

  (section (name sec-text)
    (instruction (turn OR.|OR.L|OR.R)
      (font (name custom-4))
      (text (data "Ir a"))
      (newline ())
      (dname ())))
    
;
; Define the text for the steps screen
;
  (section (name static-text)
;
; Trip Summary Screen
;
    (instruction (turn TS.)
      (font (name large-bold))
      (color (red [uint]7 green [uint]66 blue [uint]128))
      (if-navigating ()
        (text (data "Distancia / tiempo restante:")))
      (if-not-navigating ()
        (text (data "Distancia / tiempo:")))
      (newline ())
      (font (name large))
      (color (red [uint]0 green [uint]0 blue [uint]0))
      (tripremaindist ())
      (text (data " / "))
      (tripremaintime ())
      (newline ())
      (font (name large-bold))
      (color (red [uint]7 green [uint]66 blue [uint]128))
      (if-show-speed-camera ()
         (if-navigating ()
            (text (data "Próxima cám. veloc.:")))
         (if-not-navigating ()
            (text (data "Primera cám. veloc.:")))
         (newline ())
         (font (name large))
         (color (red [uint]0 green [uint]0 blue [uint]0))
         (speedcameraremaindist ())
         (newline ()))            
      (if-traffic-delay ()
        (color (red [uint]173 green [uint]0 blue [uint]0))
        (text (data "Retraso tráfico: "))
        (traffic-delay ())
        (newline ()))
      (font (name large-bold))
      (newline ())
      (color (red [uint]173 green [uint]0 blue [uint]0))
      (if-navigating ()
        (text (data "Destino:")))
      (if-not-navigating ()
        (text (data "Terminar en:")))
      (newline ())
      (font (name large))
      (color (red [uint]0 green [uint]0 blue [uint]0))
      (font (name large-bold))
      (if-destination-name ()
        (destination-name ())
        (newline ()))
      (font (name large))
      (destination-address ()
        (multiline ()))
      (if-destination-phonenum ()
		(destination-phonenum ())
		(newline ()))
	  (if-not-navigating ()      
 	    (newline ())
        (font (name large-bold))
        (color (red [uint]11 green [uint]58 blue [uint]35))
        (text (data "Empezar en:"))
        (newline ())
        (font (name large))
        (color (red [uint]0 green [uint]0 blue [uint]0))
        (font (name large-bold))
        (if-start-name ()
          (start-name ())
          (newline ()))
        (font (name large))
        (start-address ()
          (multiline ()))
        (if-start-phonenum ()
          (start-phonenum ())
          (newline ())))
      (if-ferry-on-route ()
        (color (red [uint]173 green [uint]0 blue [uint]0))
        (font (name large-bold))
        (newline ())
        (text (data "La ruta incluye: "))
        (ferriesremaining ())
        (text (data " FERRY(S)"))
        (newline ()))
      (font (name large-bold))        
      (newline ())
	  (color (red [uint]7 green [uint]66 blue [uint]128))
      (text (data "Parám. ruta:"))
      (newline ())
      (font (name large))
      (color (red [uint]0 green [uint]0 blue [uint]0))
      (tripsettings ()))

;
; Destination Maneuver
;
    (instruction (turn DT.|DT.R|DT.L)
      (font (name large))
      (if-not-lookahead ()
        (color (red [uint]0 green [uint]0 blue [uint]0))
        (text (data "Avance "))
        (dist ())
        (text (data " antes de "))
        (color (red [uint]173 green [uint]0 blue [uint]0))
        (text (data "LLEGAR")))
      (if-lookahead ()
        (color (red [uint]173 green [uint]0 blue [uint]0))
        (text (data "Llegó")))
      (color (red [uint]0 green [uint]0 blue [uint]0))
      (if (turn DT.R stacknext * next *)
        (text (data " a DERECHA")))
      (if (turn DT.L stacknext * next *)
        (text (data " a IZQUIERDA")))
      (text (data " "))
      (if-destination-name ()
        (destination-name ())
        (text (data " - ")))
      (destination-address ())
      (newline ())
      (newline ())
      (if-navigating ()
        (ahead-dist ())
        (text (data " adelante"))))

;
; Enter Highway Maneuver
;
    (instruction (turn EN.|EN.R|EN.L stacknext *)
      (font (name large))
      (if-not-lookahead ()
        (color (red [uint]0 green [uint]0 blue [uint]0))
        (text (data "Avance "))
        (dist ())
        (text (data " y ENTRE")))
      (if-lookahead ()
        (text (data " ENTRE ")))
      (if-unnamed ()
        (text (data " hacia "))
        (if-next-named () 
           (next-named ()
           	 (dname-base ())
        	 (if-aname-base ()
               (text (data " ("))
               (aname-base ())
               (text (data ")")))))
        (if-not-next-named ()
            (text (data " su destino"))))
      (if-not-unnamed ()
        (if-toward-name ()
           (text (data " hacia ")))
        (if-not-toward-name ()
           (transition-sound ()))
        (dname ())
        (if-aname ()
          (text (data " ("))
          (aname ())
          (text (data ")"))))
      (if (turn EN. stacknext * next *)
        (text (data " TODO DERECHO")))
      (if (turn EN.R stacknext * next *)
        (text (data " a DERECHA")))
      (if (turn EN.L stacknext * next *)
        (text (data " a IZQUIERDA")))
      (newline ())
      (newline ())
      (if-navigating ()
        (ahead-dist ())
        (text (data " adelante"))))

;
; Generic turn case
; (copy of New Manuevers turn case, changed to catch all turn-sounds)
;
    (instruction (turn * stacknext *)
      (font (name large))
      (if-not-lookahead ()
        (color (red [uint]0 green [uint]0 blue [uint]0))
        (text (data "Avance "))
        (dist ())
        (text (data " y ")))
      (turn-sound ())
      (if-unnamed ()
        (text (data " hacia "))
        (if-next-named () 
          (next-named ()
            (dname-base ())
            (if-aname-base ()
              (text (data " ("))
              (aname-base ())
              (text (data ")")))))
        (if-not-next-named ()
              (text (data " su destino"))))              
      (if-not-unnamed ()
        (if-toward-name ()
           (text (data " hacia ")))
        (if-not-toward-name ()
           (transition-sound ()))
        (dname ())
        (if-aname ()
           (text (data " ("))
        (aname ())
        (text (data ")"))))
      (newline ())
      (newline ())
      (if-navigating ()
        (ahead-dist ())
        (text (data " adelante"))))

))
