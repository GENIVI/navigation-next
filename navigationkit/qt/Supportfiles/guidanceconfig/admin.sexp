(admin (version [uint]1)
    ; nav-config: not overridden by device config
    (nav-config (default-poi-count [uint]10 gps-warn [uint]25 nav-recalc-limit [uint]3 nav-recalc-time-limit [uint]120
                 gps-filter-a [float]0.96 msvh [float]2.75
                 nav-avg-speed-a [float]0.98 nav-avg-speed-threshold [float]5.5
                 nav-avg-speed-min-update-below [uint]60 nav-avg-speed-min-update-above [uint]1
                 heading-interp [uint]1
                 snap-map-threshold [float]50.0
                 nav-max-traffic-incident-announce [float]10000.0
                 nav-min-traffic-incident-announce [float]500.0
                 nav-max-traffic-congestion-announce [float]10000.0
                 nav-min-traffic-congestion-announce [float]500.0
                 nav-min-traffic-congestion-length-announce [float]1609.0
                 traffic-meter-time-window [uint]0
                 traffic-notify-initial-delay [uint]15000
                 traffic-notify-restart-delay [uint]900
                 traffic-notify-restart-threshold [uint]10000
                 max-traffic-meter-light-traffic [uint]0
                 min-traffic-meter-light-traffic [uint]0
                 max-traffic-meter-moderate-traffic [uint]50
                 min-traffic-meter-moderate-traffic [uint]25
                 max-traffic-meter-severe-traffic [uint]25
                 min-traffic-meter-severe-traffic [uint]0
                 max-traffic-meter-delay-announce [uint]50
                 min-traffic-meter-delay-announce [uint]0
                 min-traffic-meter-change-to-announce [uint]50
                 nav-min-tmc-speed [float]0.44704
                 max-traffic-notify-retries [uint]3
                 nav-soft-recalc-dist [uint]5000
                 min-remain-traffic-update-announce [uint]600
                 min-no-data-disable-traffic-bar [uint]100
                 max-recalcs-in-time [uint]8
                 max-recalcs-time-limit [uint]120
                 sar-height-landscape [uint]190 
                 sar-height-portrait [uint]190 
                 sar-scaling-enabled [uint]1
                 sar-scaling-percentage-landscape [uint]40
                 sar-scaling-percentage-portrait [uint]25
                 sar-padding-left-right-landscape [uint]10 
                 sar-padding-left-right-portrait [uint]10
                 min-brightness-value [uint]3
                 max-brightness-value [uint]100
				 default-brightness-value [uint]50
                 prefetch-window [uint]10000
                 prefetch-min-window [uint]2000)
                 
               (nav-vecmap-config (
                 nav-query-num-tiles [uint]16
                 nav-route-map-tz [uint]16
                 nav-route-map-passes [uint]8
                 nav-sliding-mode-horizontal-fov [uint]45
                 nav-3d-camera-height [uint]250
                 nav-2d-camera-height [uint]400
                 nav-sliding-max-view-dist [uint]1500
                 nav-prefetch-tolerance [float]1.25
                 nav-initial-prefetch-delay [uint]10
                 nav-sliding-prefetch-delay [uint]180
                 nav-sliding-turn-map-prefetch [uint]1
                 nav-min-prefetch-distance [uint]2000
                 nav-sliding-max-prefetch-tiles [uint]120
                 nav-lc-max-prefetch-tiles [uint]45
                 nav-lc-max-prefetch-turns [uint]9
                 nav-sliding-max-prefetch-turns [uint]1
                 nav-max-tile-cache [uint]180
                 nav-max-z-clip [uint]1000
                 nav-min-z-clip [uint]5))
               (nav-instruct-distance (
                 name normal
                 bdm [float]7.999
                 bda [float]37.144
                 adm [float]6.9
                 cdm [float]11.5
                 pdm [float]4.0
                 pmm [float]3.0
                 idm [float]0.5
                 xdm [float]4.0
                 vdm [float]4.0
                 tdi [float]1000.0
                 tdm [float]300.0
                 stdi [float]1500.0
                 stdm [float]450.0))
               (nav-instruct-distance (
                 name highway
                 bdm [float]8.0
                 bda [float]40.0
                 adm [float]8.05
                 cdm [float]17.25
                 pdm [float]6.0
                 pmm [float]4.0
                 idm [float]1.0
                 xdm [float]6.0
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
                 vdm [float]1.25
                 tdi [float]0.0
                 tdm [float]0.0
                 stdi [float]0.0
                 stdm [float]0.0))
              (nav-turn-cost (command "" time [uint]0))
              (nav-turn-cost (command "TR.L" time [uint]15))
              (nav-turn-cost (command "TR.R" time [uint]10))
              (nav-turn-cost (command "EN.L" time [uint]10))
              (nav-turn-cost (command "EN.R" time [uint]5))
              (nav-turn-cost (command "UT." time [uint]45))
              (nav-turn-cost (command "SC." time [uint]0))
              (nav-turn-cost (command "RE." time [uint]30))
              
              
              ; Speed cameras enabled
  			  (nav-speed-cameras (
	 			 min-camera-distance [float]250.0)))			 
    ; ped-nav-config: not overridden by device config
    (ped-nav-config (gps-warn [uint]25 nav-recalc-limit [uint]3 nav-recalc-time-limit [uint]120
                 gps-filter-a [float]0.96 msvh [float]0.49
                 nav-avg-speed-a [float]0.98 nav-avg-speed-threshold [float]1.0
                 nav-avg-speed-min-update-below [uint]60 nav-avg-speed-min-update-above [uint]1
				 heading-interp [uint]1
                 snap-map-threshold [float]50.0
                 num-prefetch-tiles [uint]16
                 min-prefetch-distance [float]100.0
                 nav-max-traffic-incident-announce [float]10000.0
                 nav-min-traffic-incident-announce [float]500.0
                 nav-max-traffic-congestion-announce [float]10000.0
                 nav-min-traffic-congestion-announce [float]500.0
                 nav-min-traffic-congestion-length-announce [float]1609.0
                 traffic-meter-time-window [uint]0
                 traffic-notify-initial-delay [uint]15000
                 traffic-notify-restart-delay [uint]900
                 traffic-notify-restart-threshold [uint]10000
                 max-traffic-meter-light-traffic [uint]0
                 min-traffic-meter-light-traffic [uint]0
                 max-traffic-meter-moderate-traffic [uint]50
                 min-traffic-meter-moderate-traffic [uint]25
                 max-traffic-meter-severe-traffic [uint]25
                 min-traffic-meter-severe-traffic [uint]0
                 max-traffic-meter-delay-announce [uint]50
                 min-traffic-meter-delay-announce [uint]0
                 min-traffic-meter-change-to-announce [uint]50
                 nav-min-tmc-speed [float]0.44704
                 max-traffic-notify-retries [uint]3
                 nav-route-map-passes [uint]8
                 nav-route-map-tz [uint]16
                 nav-soft-recalc-dist [uint]5000
                 min-remain-traffic-update-announce [uint]600
                 min-no-data-disable-traffic-bar [uint]100
                 nav-turnmap-tiles-initial-num [uint]15
                 nav-turnmap-tiles-prefetch-maneuvers [uint]3
                 nav-turnmap-tiles-prefetch-num [uint]15)
						
						
               (nav-instruct-distance (
                 name normal
                 bdm [float]7.999
                 bda [float]37.144
                 adm [float]6.0
                 cdm [float]10.0
                 pdm [float]4.0
                 pmm [float]5.0
                 idm [float]1.0
                 xdm [float]4.0
                 vdm [float]4.0
                 tdi [float]1000.0
                 tdm [float]300.0
                 stdi [float]1500.0
                 stdm [float]450.0))
               (nav-instruct-distance (
                 name highway
                 bdm [float]8.0
                 bda [float]40.0
                 adm [float]7.0
                 cdm [float]15.0
                 pdm [float]5.0
                 pmm [float]3.5
                 idm [float]2.0
                 xdm [float]5.0
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
                 vdm [float]4.0
                 tdi [float]1000.0
                 tdm [float]300.0
                 stdi [float]1500.0
                 stdm [float]450.0))
              (nav-instruct-distance (
                 name destination
                 bdm [float]3.0
                 bda [float]50.0
                 adm [float]75.0
                 cdm [float]100.0
                 pdm [float]20.0
                 pmm [float]5.0
                 idm [float]1.0
                 xdm [float]4.0
                 vdm [float]4.0
                 tdi [float]1000.0
                 tdm [float]300.0
                 stdi [float]1500.0
                 stdm [float]450.0))
              (nav-turn-cost (command "" time [uint]0))
              (nav-turn-cost (command "TR.L" time [uint]15))
              (nav-turn-cost (command "TR.R" time [uint]10))
              (nav-turn-cost (command "EN.L" time [uint]10))
              (nav-turn-cost (command "EN.R" time [uint]5))
              (nav-turn-cost (command "UT." time [uint]45))
              (nav-turn-cost (command "RE." time [uint]30)))
)

