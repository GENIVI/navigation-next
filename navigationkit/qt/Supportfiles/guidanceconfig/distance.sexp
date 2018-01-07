(guidanceconfig()
	(distance() 
		(nav-instruct-distance (
                 name normal
                 bdm [float]12.0
                 bda [float]37.144
                 adm [float]6.9
                 cdm [float]11.5
                 pdm [float]2.0
                 pmm [float]1.0
                 idm [float]0.45
                 xdm [float]4.0
		         vlm [float]0.0
                 vdm [float]4.0
                 tdi [float]1000.0
                 tdm [float]300.0
                 stdi [float]1500.0
                 stdm [float]450.0))
	    (nav-instruct-distance (
                 name keep
                 bdm [float]13.0
                 bda [float]37.144
                 adm [float]6.9
                 cdm [float]11.5
                 pdm [float]2.0
                 pmm [float]1.0
                 idm [float]0.5
                 xdm [float]4.0
		         vlm [float]0.0
                 vdm [float]4.0
                 tdi [float]1150.0
                 tdm [float]350.0
                 stdi [float]1650.0
                 stdm [float]500.0))
        (nav-instruct-distance (
                 name highway
                 bdm [float]8.0
                 bda [float]40.0
                 adm [float]8.05
                 cdm [float]17.25
                 pdm [float]6.0
                 pmm [float]3.0
                 idm [float]1.25
                 xdm [float]6.0
 		         vlm [float]0.0
                 vdm [float]4.0
                 tdi [float]1500.0
                 tdm [float]450.0
                 stdi [float]2000.0
                 stdm [float]600.0))
        (nav-instruct-distance (
                 name merge
                 bdm [float]8.0
                 bda [float]40.0
                 adm [float]7.0
                 cdm [float]15.0
                 pdm [float]5.0
                 pmm [float]3.5
                 idm [float]2.0
                 xdm [float]5.0
		         vlm [float]0.0
                 vdm [float]4.0
                 tdi [float]1000.0
                 tdm [float]300.0
                 stdi [float]1500.0
                 stdm [float]450.0))
        (nav-instruct-distance (
                 name origin
                 bdm [float]0
                 bda [float]100000.0
                 adm [float]1.4
                 cdm [float]1.5
                 pdm [float]1.2
                 pmm [float]1.1
                 idm [float]1.0
                 xdm [float]1.2
		         vlm [float]0.0
                 vdm [float]4.0
                 tdi [float]1000.0
                 tdm [float]300.0
                 stdi [float]1500.0
                 stdm [float]450.0))
        (nav-instruct-distance (
                 name destination
                 bdm [float]3.0
                 bda [float]50.0
                 adm [float]86.25
                 cdm [float]115.0
                 pdm [float]8.0
                 pmm [float]5.0
                 idm [float]0.5
                 xdm [float]8.0
		         vlm [float]0.0
                 vdm [float]4.0
                 tdi [float]1000.0
                 tdm [float]300.0
                 stdi [float]1500.0
                 stdm [float]450.0))
        (nav-instruct-distance (
                 name camera
                 bdm [float]16.0
                 bda [float]100.0
                 adm [float]5.0
                 cdm [float]0.0
                 pdm [float]0.0 
                 pmm [float]0.0 
                 idm [float]1.0
                 xdm [float]0.0
		         vlm [float]0.0
                 vdm [float]1.25
                 tdi [float]0.0
                 tdm [float]0.0
                 stdi [float]0.0
                 stdm [float]0.0))
	)

	(commandindex()
            (command (name DT. index destination))
            (command (name DT.L index destination))
            (command (name DT.R index destination))
            
            (command (name ER. index highway))
            (command (name ER.L index highway))
            (command (name ER.R index highway))
            (command (name EX. index highway))
            (command (name EX.L index highway))
            (command (name EX.R index highway))
            (command (name KH. index highway))
            (command (name KH.L index highway))
            (command (name KH.R index highway))
            (command (name MR. index highway))
            (command (name MR.L index highway))
            (command (name MR.R index highway))
            (command (name SH. index highway))
            (command (name SH.L index highway))
            (command (name SH.R index highway))
            (command (name KP. index highway))
            (command (name KP.L index highway))
            (command (name KP.R index highway))
            
            (command (name KR. index normal))
            (command (name KR.L index normal))
            (command (name KR.R index normal))
            (command (name BE. index normal))
            (command (name BE.L index normal))
            (command (name BE.R index normal))
            (command (name EC. index normal))
            (command (name EC.L index normal))
            (command (name EC.R index normal))
            (command (name EN index normal))
            (command (name EN.L index normal))
            (command (name EN.R index normal))
            (command (name FE. index normal))
            (command (name FX. index normal))
            (command (name NC. index normal))
            (command (name NR. index normal))
            (command (name NR.L index normal))
            (command (name NR.R index normal))
            (command (name OR. index normal))
            (command (name PE. index normal))
            (command (name RE. index normal))
            (command (name RT. index normal))
            (command (name ST. index normal))
            (command (name ST.L index normal))
            (command (name ST.R index normal))
            (command (name TE. index normal))
            (command (name TE.L index normal))
            (command (name TE.R index normal))
            (command (name TR. index normal))
            (command (name TR.L index normal))
            (command (name TR.R index normal))
            (command (name TR.SL index normal))
            (command (name TR.SR index normal))
            (command (name TR.HL index normal))
            (command (name TR.HR index normal))
            (command (name UT. index normal))
			
			(command (name KS. index keep))
			(command (name KS.L index keep))
			(command (name KS.R index keep))
	)
	(minimummaneuverspeed (value 4.47))


)
              
