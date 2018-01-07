(nav-instruct (command-set-version [uint]3 media-type text/x-nim-formatted media-sub-type "")

 ;

 ; Define translations for unnamed segments, currently numbered 0 through 7

 ;

    (reserved-phrases ()

        (reserved (unnamed-road-type [uint]0 unnamed-road-phrase "Ramp"))
        (reserved (unnamed-road-type [uint]1 unnamed-road-phrase "A local road"))
        (reserved (unnamed-road-type [uint]2 unnamed-road-phrase "A tunnel"))
        (reserved (unnamed-road-type [uint]3 unnamed-road-phrase "A ferry"))
        (reserved (unnamed-road-type [uint]4 unnamed-road-phrase "A bridge"))
        (reserved (unnamed-road-type [uint]5 unnamed-road-phrase "A walkway"))
        (reserved (unnamed-road-type [uint]6 unnamed-road-phrase "Stairs"))
        (reserved (unnamed-road-type [uint]7 unnamed-road-phrase "An escalator"))
        (reserved (unnamed-road-type [uint]8 unnamed-road-phrase "Walkway"))
        (reserved (unnamed-road-type [uint]9 unnamed-road-phrase "Covered walkway"))
        (reserved (unnamed-road-type [uint]10 unnamed-road-phrase "Escalator"))
        (reserved (unnamed-road-type [uint]11 unnamed-road-phrase "Park"))
        (reserved (unnamed-road-type [uint]12 unnamed-road-phrase "Square"))
        (reserved (unnamed-road-type [uint]13 unnamed-road-phrase "Stairs"))
    )


;

; Define the basic instruction for each turn.

; This allows many subsequent rules to be combined since they 

; only differ in the basic turn command

;

  (turn-sounds ()

; TR.R - Right Turn

    (define-turn-sound (turn TR.R pos *)

      (text (data "TURN RIGHT")))

; TR.L - Left Turn

    (define-turn-sound (turn TR.L pos *)

      (text (data "TURN LEFT")))

; UT. - U Turn

    (define-turn-sound (turn UT. pos *)

      (text (data "MAKE A U-TURN")))

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

      (text (data "EXIT")))

; EX.R - Exit Highway on the Right

    (define-turn-sound (turn EX.R pos *)

      (text (data "EXIT RIGHT")))

; EX.L - Exit Highway on the Left

    (define-turn-sound (turn EX.L pos *)

      (text (data "EXIT LEFT")))

; MR.R - Merge into traffic on the right

    (define-turn-sound (turn MR.R pos *)

      (text (data "MERGE RIGHT")))

; MR.L - Exit Highway on the Left

    (define-turn-sound (turn MR.L pos *)

      (text (data "MERGE LEFT")))

; KP.R - Keep to the right (Split)

    (define-turn-sound (turn KP.R pos *)

      (text (data "KEEP RIGHT")))

; KP.L - Keep to the left (Split)

    (define-turn-sound (turn KP.L pos *)

      (text (data "KEEP LEFT")))

; KP. - Keep to the center (Split)

    (define-turn-sound (turn KP. pos *)

      (text (data "KEEP STRAIGHT")))

; NC. - Name Change (Continue)

    (define-turn-sound (turn NC. pos *)

      (text (data "CONTINUE")))

; RE. - Enter Traffic Circle

    (define-turn-sound (turn RE. pos *)

      (text (data "ENTER TRAFFIC CIRCLE")))
; RX.1 - Exit Traffic Cirle 1st Exit

    (define-turn-sound (turn RX.1 pos *)

      (text (data "Take the FIRST EXIT at the TRAFFIC CIRCLE")))
; RX.2 - Exit Traffic Cirle 2nd Exit

    (define-turn-sound (turn RX.2 pos *)

      (text (data "Take the SECOND EXIT at the TRAFFIC CIRCLE")))
; RX.3 - Exit Traffic Cirle 3rd Exit

    (define-turn-sound (turn RX.3 pos *)

      (text (data "Take the THIRD EXIT at the TRAFFIC CIRCLE")))
; RX.4 - Exit Traffic Cirle 4th Exit

    (define-turn-sound (turn RX.4 pos *)

      (text (data "Take the FOURTH EXIT at the TRAFFIC CIRCLE")))
; RX.5 - Exit Traffic Cirle 5th Exit

    (define-turn-sound (turn RX.5 pos *)

      (text (data "Take the FIFTH EXIT at the TRAFFIC CIRCLE")))
; RX.6 - Exit Traffic Cirle 6th Exit

    (define-turn-sound (turn RX.6 pos *)

      (text (data "Take the SIXTH EXIT at the TRAFFIC CIRCLE")))
; RX.7 - Exit Traffic Cirle 7th Exit

    (define-turn-sound (turn RX.7 pos *)

      (text (data "Take the SEVENTH EXIT at the TRAFFIC CIRCLE")))
; RX.8 - Exit Traffic Cirle 8th Exit

    (define-turn-sound (turn RX.8 pos *)

      (text (data "Take the EIGHTH EXIT at the TRAFFIC CIRCLE")))
; RX.9 - Exit Traffic Cirle 9th Exit

    (define-turn-sound (turn RX.9 pos *)

      (text (data "Take the NINTH EXIT at the TRAFFIC CIRCLE")))
; RX.10 - Exit Traffic Cirle 10th Exit

    (define-turn-sound (turn RX.10 pos *)

      (text (data "Take the TENTH EXIT at the TRAFFIC CIRCLE")))
; RT. - Drive straight through rotary TODO CHANGE LATER

    (define-turn-sound (turn RT. pos *)

      (text (data "Go STRAIGHT through the TRAFFIC CIRCLE")))
; ES.L - Enter Stairs

    (define-turn-sound (turn ES.L pos *)

      (text (data "Take STAIRS on your LEFT")))

; ES.R - Enter Stairs

    (define-turn-sound (turn ES.R pos *)

      (text (data "Take STAIRS on your RIGHT")))

; ES. - Enter Stairs

    (define-turn-sound (turn ES. pos *)

      (text (data "Take STAIRS STRAIGHT AHEAD")))
; EE.L - Enter Escalator

    (define-turn-sound (turn EE.L pos *)

      (text (data "Take ESCALATOR on your LEFT")))

; EE.R - Enter Escalator

    (define-turn-sound (turn EE.R pos *)

      (text (data "Take ESCALATOR on your RIGHT")))

; EE. - Enter Escalator

    (define-turn-sound (turn EE. pos *)

      (text (data "Take ESCALATOR STRAIGHT AHEAD")))
; NR.R - Enter Private Roadway on the Right

    (define-turn-sound (turn NR.R pos *)

      (text (data "Enter Private Roadway on the RIGHT")))

; NR.L - Enter Private Roadway on the Left

    (define-turn-sound (turn NR.L pos *)

      (text (data "Enter Private Roadway on the LEFT")))

; NR.  - Enter Private Roadway Straight Ahead

    (define-turn-sound (turn NR. pos *)

      (text (data "Enter Private Roadway STRAIGHT AHEAD")))
; BE.R - Cross Bridge  on the Right

    (define-turn-sound (turn BE.R pos *)

      (text (data "Cross BRIDGE on the RIGHT")))

; BE.L - Cross Bridge on the Left

    (define-turn-sound (turn BE.L pos *)

      (text (data "Cross BRIDGE on the LEFT")))

; BE.  - Cross Bridge Straight Ahead

    (define-turn-sound (turn BE. pos *)

      (text (data "Cross BRIDGE STRAIGHT AHEAD")))
; TE.R - Enter Tunnel  on the Right

    (define-turn-sound (turn TE.R pos *)

      (text (data "Enter TUNNEL on the RIGHT")))

; TE.L - Enter Tunnel on the Left

    (define-turn-sound (turn TE.L pos *)

      (text (data "Enter TUNNEL on the LEFT")))

; TE.  - Enter Tunnel Straight Ahead

    (define-turn-sound (turn TE. pos *)

      (text (data "Enter TUNNEL STRAIGHT AHEAD")))
; ER.R - Take the Ramp on the Right

    (define-turn-sound (turn ER.R pos *)

      (text (data "Take the RAMP on the RIGHT")))

; ER.L - Take the Ramp on the Left

    (define-turn-sound (turn ER.L pos *)

      (text (data "Take the RAMP on the LEFT")))

; ER.  - Take the Ramp Straight Ahead

    (define-turn-sound (turn ER. pos *)

      (text (data "Take the RAMP STRAIGHT AHEAD")))
; PE.  - Continue on Foot

    (define-turn-sound (turn PE. pos *)

      (text (data "CONTINUE on FOOT")))

; EC.  - Enter Country Straight Ahead

    (define-turn-sound (turn EC. pos *)

      (text (data "Enter COUNTRY STRAIGHT AHEAD")))
; EC.R  - Enter Country on the Right

    (define-turn-sound (turn EC.R pos *)

      (text (data "Enter COUNTRY on the RIGHT")))

; EC.L  - Enter Country on the Left

    (define-turn-sound (turn EC.L pos *)

      (text (data "Enter COUNTRY on the LEFT")))      

; FE.  - Take a Ferry

    (define-turn-sound (turn FE. pos *)

      (text (data "ENTER FERRY")))
; FX.  - Leave a Ferry

    (define-turn-sound (turn FX. pos *)

      (text (data "EXIT FERRY"))))       
;

; Define the transition sounds associated with each type of turn

;

  (transition-sounds ()

    (define-transition-sound (turn TR.R|TR.L|H.TR.R|H.TR.L|MR.R|MR.L|KP.R|KP.L|KP.|FX. pos *)

      (text (data " on ")))

    (define-transition-sound (turn EN.R|EN.L|EN. pos *)

      (text (data " for ")))

    (define-transition-sound (turn EX.|EX.R|EX.L pos *)

      (text (data " at ")))

    (define-transition-sound (turn NC. pos *)

      (text (data " on ")))

    (define-transition-sound (turn UT. pos *)

      (text (data " at ")))

    (define-transition-sound (turn RE.|RX.1|RX.2|RX.3|RX.4|RX.5|RX.6|RX.7|RX.8|RX.9|RX.10|RT. pos *)

      (text (data " on "))))
      
      
;

; Define the additional stack text.

;

  (section (name stack-text)

    (instruction (turn * stacknext "RX.1")

      (stack-dist ())

      (text (data " / 1st")))

    (instruction (turn * stacknext "RX.2")

      (stack-dist ())

      (text (data " / 2nd")))

    (instruction (turn * stacknext "RX.3")

      (stack-dist ())

      (text (data " / 3rd")))

    (instruction (turn * stacknext "RX.4")

      (stack-dist ())

      (text (data " / 4th")))

    (instruction (turn * stacknext "RX.5")

      (stack-dist ())

      (text (data " / 5th")))

    (instruction (turn * stacknext "RX.6")

      (stack-dist ())

      (text (data " / 6th")))

    (instruction (turn * stacknext "RX.7")

      (stack-dist ())

      (text (data " / 7th")))

    (instruction (turn * stacknext "RX.8")

      (stack-dist ())

      (text (data " / 8th")))

    (instruction (turn * stacknext "RX.9")

      (stack-dist ())

      (text (data " / 9th")))

    (instruction (turn * stacknext "RX.10")

      (stack-dist ())

      (text (data " / 10th")))

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

        (text (data "You have ARRIVED at your destination:")))

      (if (turn DT.L stacknext * next *)

        (text (data "You have ARRIVED at your destination on the LEFT:")))	

      (if (turn DT.R stacknext * next *)

        (text (data "You have ARRIVED at your destination on the RIGHT:")))	

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

      (text (data "Arrived"))))
      
      
;

; Define the secondary screen startup case text

;


  (section (name sec-text)

    (instruction (turn OR.|OR.L|OR.R)

      (font (name custom-4))

      (text (data "Go to"))

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

        (text (data "Distance / Time Remaining:")))

      (if-not-navigating ()

        (text (data "Distance / Time:")))

      (newline ())

      (font (name large))

      (color (red [uint]0 green [uint]0 blue [uint]0))

      (tripremaindist ())

      (text (data " / "))

      (tripremaintime ())

      (newline ())

      (if-traffic-delay ()

        (color (red [uint]173 green [uint]0 blue [uint]0))

        (text (data "Traffic Delay: "))

        (traffic-delay ())

        (newline ()))

      (if-show-speed-camera ()

        (font (name large-bold))

        (color (red [uint]7 green [uint]66 blue [uint]128))

		(text (data "Next speed camera:"))

        (newline ())

        (font (name large))

        (color (red [uint]0 green [uint]0 blue [uint]0))

        (speedcameraremaindist ())

        (newline ()))

      (font (name large-bold))

      (newline ())

      (color (red [uint]173 green [uint]0 blue [uint]0))

      (if-navigating ()

        (text (data "Destination:")))

      (if-not-navigating ()

        (text (data "End Trip At:")))

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

        (text (data "Start Trip At:"))

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

      (font (name large-bold))        

	    (if-ferry-on-route ()

			(color (red [uint]173 green [uint]0 blue [uint]0))

			(font (name large-bold))

			(newline ())

			(text (data "Trip includes "))

			(ferriesremaining ())

			(text (data " FERRY(s)")))

      (newline ())

      (newline ())

	  (color (red [uint]7 green [uint]66 blue [uint]128))

      (text (data "Trip Settings:"))

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

        (text (data "Go "))

        (dist ())

        (text (data " before "))

        (color (red [uint]173 green [uint]0 blue [uint]0))

        (text (data "ARRIVING")))

      (if-lookahead ()

        (color (red [uint]173 green [uint]0 blue [uint]0))

        (text (data "ARRIVE")))

      (color (red [uint]0 green [uint]0 blue [uint]0))

      (if (turn DT.R stacknext * next *)

        (text (data " on the RIGHT")))

      (if (turn DT.L stacknext * next *)

        (text (data " on the LEFT")))

      (text (data " at "))

      (if-destination-name ()

        (destination-name ())

        (text (data " - ")))

      (destination-address ())

      (newline ())

      (newline ())

      (text (data "Step "))

      (step-number ())

      (text (data " of "))

      (total-steps ())

      (if-navigating ()

        (text (data " ("))

        (ahead-dist ())

        (text (data " ahead"))))


;

; Enter Highway Maneuver

;

    (instruction (turn EN.|EN.R|EN.L stacknext *)

      (font (name large))

      (if-not-lookahead ()

        (color (red [uint]0 green [uint]0 blue [uint]0))

        (text (data "Go "))

        (dist ())

        (text (data " and ENTER ")))

      (if-lookahead ()

        (text (data "ENTER ")))

      (dname ())

      (if-aname ()

        (text (data " ("))

        (aname ())

        (text (data ")")))

      (if (turn EN. stacknext * next *)

        (text (data " STRAIGHT AHEAD")))

      (if (turn EN.R stacknext * next *)

        (text (data " on the RIGHT")))

      (if (turn EN.L stacknext * next *)

        (text (data " on the LEFT")))

      (newline ())

      (newline ())

      (text (data "Step "))

      (step-number ())

      (text (data " of "))

      (total-steps ())

      (if-navigating ()

        (text (data " ("))

        (ahead-dist ())

        (text (data " ahead"))))
        
        
;

; New Manuevers turn case

;

    (instruction (turn NR.R|NR.L|NR.|BE.R|BE.L|BE.|TE.R|TE.L|TE.|ES.R|ES.L|ES.|EE.R|EE.L|EE.|ER.R|ER.L|ER.|FE.|FX. stacknext *)

      (font (name large))

      (if-not-lookahead ()

        (color (red [uint]0 green [uint]0 blue [uint]0))

        (text (data "Go "))

        (dist ())

        (text (data " and ")))

      (turn-sound ())

      (if-unnamed ()

        (text (data " towards "))

        (if-next-named () 

          (next-named ()

            (dname-base ())

            (if-aname-base ()

              (text (data " ("))

              (aname-base ())

              (text (data ")")))))

          (if-not-next-named ()

              (text (data " your destination"))))

      (newline ())

      (newline ())

      (text (data "Step "))

      (step-number ())

      (text (data " of "))

      (total-steps ())

      (if-navigating ()

        (text (data " ("))

        (ahead-dist ())

        (text (data " ahead"))))




;

; Generic Turn Case

;

    (instruction (turn * stacknext *)

      (font (name large))

      (if-not-lookahead ()

        (color (red [uint]0 green [uint]0 blue [uint]0))

        (text (data "Go "))

        (dist ())

        (text (data " and ")))

      (turn-sound ())

      (if-unnamed ()

        (text (data " towards "))

        (if-next-named () 

          (next-named ()

            (dname-base ())

            (if-aname-base ()

              (text (data " ("))

              (aname-base ())

              (text (data ")")))))

          (if-not-next-named ()

              (text (data " your destination"))))

      (if-not-unnamed ()

        (if-toward-name ()

          (text (data " towards ")))

        (if-not-toward-name ()

          (text (data " on ")))

        (dname ())

        (if-aname ()

          (text (data " ("))

          (aname ())

          (text (data ")"))))

      (newline ())

      (newline ())

      (text (data "Step "))

      (step-number ())

      (text (data " of "))

      (total-steps ())

      (if-navigating ()

        (text (data " ("))

        (ahead-dist ())

        (text (data " ahead"))))))

