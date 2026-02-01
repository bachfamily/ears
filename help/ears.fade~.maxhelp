{
    "patcher": {
        "fileversion": 1,
        "appversion": {
            "major": 9,
            "minor": 1,
            "revision": 1,
            "architecture": "x64",
            "modernui": 1
        },
        "classnamespace": "box",
        "rect": [ 100.0, 100.0, 658.0, 496.0 ],
        "gridsize": [ 5.0, 5.0 ],
        "showrootpatcherontab": 0,
        "showontab": 0,
        "boxes": [
            {
                "box": {
                    "fontname": "Arial",
                    "fontsize": 12.0,
                    "id": "obj-3",
                    "maxclass": "newobj",
                    "numinlets": 0,
                    "numoutlets": 0,
                    "patcher": {
                        "fileversion": 1,
                        "appversion": {
                            "major": 9,
                            "minor": 1,
                            "revision": 1,
                            "architecture": "x64",
                            "modernui": 1
                        },
                        "classnamespace": "box",
                        "rect": [ 100.0, 126.0, 658.0, 470.0 ],
                        "default_fontsize": 13.0,
                        "gridsize": [ 5.0, 5.0 ],
                        "showontab": 1,
                        "boxes": [
                            {
                                "box": {
                                    "fontname": "Arial",
                                    "fontsize": 13.0,
                                    "id": "obj-42",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 10.0, 63.0, 69.0, 21.0 ],
                                    "presentation_linecount": 2,
                                    "text": "Fade typs:"
                                }
                            },
                            {
                                "box": {
                                    "bubble": 1,
                                    "bubbleside": 2,
                                    "fontname": "Arial",
                                    "fontsize": 13.0,
                                    "id": "obj-13",
                                    "linecount": 2,
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 464.0, 200.0, 176.0, 55.0 ],
                                    "text": "For fades of type \"curve\" you can specify the slope"
                                }
                            },
                            {
                                "box": {
                                    "attr": "fadeoutcurve",
                                    "id": "obj-33",
                                    "maxclass": "attrui",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 464.0, 297.0, 156.0, 23.0 ]
                                }
                            },
                            {
                                "box": {
                                    "attr": "fadeincurve",
                                    "id": "obj-34",
                                    "maxclass": "attrui",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 464.0, 265.0, 156.0, 23.0 ]
                                }
                            },
                            {
                                "box": {
                                    "attr": "fadeouttype",
                                    "id": "obj-35",
                                    "maxclass": "attrui",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 263.0, 301.0, 189.0, 23.0 ]
                                }
                            },
                            {
                                "box": {
                                    "attr": "fadeintype",
                                    "id": "obj-36",
                                    "maxclass": "attrui",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 247.0, 269.0, 189.0, 23.0 ]
                                }
                            },
                            {
                                "box": {
                                    "buffername": "u944007947",
                                    "id": "obj-37",
                                    "maxclass": "waveform~",
                                    "numinlets": 5,
                                    "numoutlets": 6,
                                    "outlettype": [ "float", "float", "float", "float", "list", "" ],
                                    "patching_rect": [ 194.0, 407.0, 152.0, 58.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-38",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 194.0, 382.0, 46.0, 23.0 ],
                                    "text": "set $1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-39",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 194.0, 238.0, 77.0, 23.0 ],
                                    "text": "earsTestBuf"
                                }
                            },
                            {
                                "box": {
                                    "fontname": "Arial",
                                    "fontsize": 13.0,
                                    "id": "obj-41",
                                    "linecount": 2,
                                    "maxclass": "newobj",
                                    "numinlets": 3,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 194.0, 338.0, 324.0, 38.0 ],
                                    "text": "ears.fade~ 500 500 @fadeintype 3 @fadeincurve 0.5 @fadeouttype 3 @fadeoutcurve -0.5"
                                }
                            },
                            {
                                "box": {
                                    "attr": "fadeouttype",
                                    "id": "obj-30",
                                    "maxclass": "attrui",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 83.25, 152.0, 274.0, 23.0 ]
                                }
                            },
                            {
                                "box": {
                                    "attr": "fadeintype",
                                    "id": "obj-28",
                                    "maxclass": "attrui",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 68.0, 120.0, 274.0, 23.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-8",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 326.0, 27.2999267578125, 91.0, 23.0 ],
                                    "text": "loadmess fill 1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-1",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "float", "bang" ],
                                    "patching_rect": [ 326.0, 61.0, 154.0, 23.0 ],
                                    "text": "buffer~ earsTestBuf 1000"
                                }
                            },
                            {
                                "box": {
                                    "buffername": "u678005876",
                                    "id": "obj-22",
                                    "maxclass": "waveform~",
                                    "numinlets": 5,
                                    "numoutlets": 6,
                                    "outlettype": [ "float", "float", "float", "float", "list", "" ],
                                    "patching_rect": [ 15.0, 269.0, 152.0, 58.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-21",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 15.0, 244.0, 46.0, 23.0 ],
                                    "text": "set $1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-4",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 15.0, 89.0, 77.0, 23.0 ],
                                    "text": "earsTestBuf"
                                }
                            },
                            {
                                "box": {
                                    "fontname": "Arial",
                                    "fontsize": 13.0,
                                    "id": "obj-14",
                                    "maxclass": "newobj",
                                    "numinlets": 3,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 15.0, 206.0, 304.0, 23.0 ],
                                    "text": "ears.fade~ 500 500 @fadeintype 2 @fadeouttype 2"
                                }
                            },
                            {
                                "box": {
                                    "border": 0,
                                    "filename": "helpname.js",
                                    "id": "obj-2",
                                    "ignoreclick": 1,
                                    "jsarguments": [ "ears.fade~" ],
                                    "maxclass": "jsui",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 10.0, 10.0, 224.52798461914062, 57.599853515625 ]
                                }
                            }
                        ],
                        "lines": [
                            {
                                "patchline": {
                                    "destination": [ "obj-21", 0 ],
                                    "source": [ "obj-14", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-22", 0 ],
                                    "source": [ "obj-21", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-14", 0 ],
                                    "source": [ "obj-28", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-14", 0 ],
                                    "source": [ "obj-30", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-41", 0 ],
                                    "source": [ "obj-33", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-41", 0 ],
                                    "source": [ "obj-34", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-41", 0 ],
                                    "source": [ "obj-35", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-41", 0 ],
                                    "source": [ "obj-36", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-37", 0 ],
                                    "source": [ "obj-38", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-41", 0 ],
                                    "source": [ "obj-39", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-14", 0 ],
                                    "source": [ "obj-4", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-38", 0 ],
                                    "source": [ "obj-41", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-1", 0 ],
                                    "source": [ "obj-8", 0 ]
                                }
                            }
                        ]
                    },
                    "patching_rect": [ 50.0, 144.0, 76.0, 22.0 ],
                    "presentation_linecount": 2,
                    "saved_object_attributes": {
                        "fontsize": 13.0
                    },
                    "text": "p \"fade type\"",
                    "varname": "basic_tab[1]"
                }
            },
            {
                "box": {
                    "fontname": "Arial",
                    "fontsize": 12.0,
                    "id": "obj-1",
                    "maxclass": "newobj",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "patching_rect": [ 299.0, 205.0, 149.0, 22.0 ],
                    "saved_object_attributes": {
                        "filename": "helpstarter.js",
                        "parameter_enable": 0
                    },
                    "text": "js helpstarter.js ears.fade~"
                }
            },
            {
                "box": {
                    "fontname": "Arial",
                    "fontsize": 12.0,
                    "id": "obj-2",
                    "maxclass": "newobj",
                    "numinlets": 0,
                    "numoutlets": 0,
                    "patcher": {
                        "fileversion": 1,
                        "appversion": {
                            "major": 9,
                            "minor": 1,
                            "revision": 1,
                            "architecture": "x64",
                            "modernui": 1
                        },
                        "classnamespace": "box",
                        "rect": [ 0.0, 26.0, 658.0, 470.0 ],
                        "default_fontsize": 13.0,
                        "gridsize": [ 5.0, 5.0 ],
                        "showontab": 1,
                        "boxes": [
                            {
                                "box": {
                                    "buffername": "u792005138",
                                    "id": "obj-9",
                                    "maxclass": "waveform~",
                                    "numinlets": 5,
                                    "numoutlets": 6,
                                    "outlettype": [ "float", "float", "float", "float", "list", "" ],
                                    "patching_rect": [ 457.0, 388.0, 120.5, 39.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-18",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 457.0, 363.0, 46.0, 23.0 ],
                                    "text": "set $1"
                                }
                            },
                            {
                                "box": {
                                    "fontface": 0,
                                    "fontname": "Arial",
                                    "fontsize": 13.0,
                                    "hyperlinkcolor": [ 0.741176, 0.356863, 0.047059, 1.0 ],
                                    "id": "obj-29",
                                    "linecount": 2,
                                    "linkstart": [ "h" ],
                                    "maxclass": "bach.hypercomment",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 10.0, 413.0, 393.0, 36.0 ],
                                    "text": "Click here to learn about the common features of ears modules, including in-place operations and dynamic allocation."
                                }
                            },
                            {
                                "box": {
                                    "hidden": 1,
                                    "id": "obj-27",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 21.0, 447.0, 152.0, 23.0 ],
                                    "text": "load ears.help.commons"
                                }
                            },
                            {
                                "box": {
                                    "hidden": 1,
                                    "id": "obj-17",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 21.0, 482.0, 57.0, 23.0 ],
                                    "text": "pcontrol"
                                }
                            },
                            {
                                "box": {
                                    "buffername": "u735001678",
                                    "id": "obj-25",
                                    "maxclass": "waveform~",
                                    "numinlets": 5,
                                    "numoutlets": 6,
                                    "outlettype": [ "float", "float", "float", "float", "list", "" ],
                                    "patching_rect": [ 160.0, 355.0, 120.5, 39.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-26",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 160.0, 330.0, 46.0, 23.0 ],
                                    "text": "set $2"
                                }
                            },
                            {
                                "box": {
                                    "buffername": "earsBufAnton",
                                    "id": "obj-24",
                                    "maxclass": "waveform~",
                                    "numinlets": 5,
                                    "numoutlets": 6,
                                    "outlettype": [ "float", "float", "float", "float", "list", "" ],
                                    "patching_rect": [ 431.0, 92.0, 182.5, 39.0 ]
                                }
                            },
                            {
                                "box": {
                                    "buffername": "earsBufDrumLoop",
                                    "id": "obj-23",
                                    "maxclass": "waveform~",
                                    "numinlets": 5,
                                    "numoutlets": 6,
                                    "outlettype": [ "float", "float", "float", "float", "list", "" ],
                                    "patching_rect": [ 431.0, 10.0, 182.5, 39.0 ]
                                }
                            },
                            {
                                "box": {
                                    "buffername": "u854001653",
                                    "id": "obj-22",
                                    "maxclass": "waveform~",
                                    "numinlets": 5,
                                    "numoutlets": 6,
                                    "outlettype": [ "float", "float", "float", "float", "list", "" ],
                                    "patching_rect": [ 26.0, 355.0, 120.5, 39.0 ]
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-21",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 26.0, 330.0, 46.0, 23.0 ],
                                    "text": "set $1"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-16",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 457.0, 279.0, 89.0, 23.0 ],
                                    "text": "earsBufAnton"
                                }
                            },
                            {
                                "box": {
                                    "bubble": 1,
                                    "bubbleside": 2,
                                    "fontname": "Arial",
                                    "fontsize": 13.0,
                                    "id": "obj-13",
                                    "linecount": 3,
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 426.5, 204.0, 147.0, 69.0 ],
                                    "text": "Fade in for half of sound length, fade out for the other half"
                                }
                            },
                            {
                                "box": {
                                    "fontname": "Arial",
                                    "fontsize": 13.0,
                                    "id": "obj-12",
                                    "linecount": 2,
                                    "maxclass": "newobj",
                                    "numinlets": 3,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 457.0, 315.0, 180.0, 38.0 ],
                                    "text": "ears.fade~ 0.5 0.5 @timeunit relative"
                                }
                            },
                            {
                                "box": {
                                    "hidden": 1,
                                    "id": "obj-11",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 266.5, 159.5, 99.0, 23.0 ],
                                    "text": "loadmess 3000"
                                }
                            },
                            {
                                "box": {
                                    "hidden": 1,
                                    "id": "obj-10",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 70.5, 141.5, 91.0, 23.0 ],
                                    "text": "loadmess 400"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-3",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 15.0, 285.0, 57.0, 23.0 ],
                                    "text": "print out"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-8",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "float", "bang" ],
                                    "patching_rect": [ 431.0, 138.5, 170.0, 23.0 ],
                                    "text": "buffer~ earsBufAnton anton"
                                }
                            },
                            {
                                "box": {
                                    "bubble": 1,
                                    "fontname": "Arial",
                                    "fontsize": 13.0,
                                    "id": "obj-6",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 222.5, 208.5, 147.0, 25.0 ],
                                    "text": "Fade out length (ms)"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-7",
                                    "maxclass": "number",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 170.5, 208.5, 50.0, 23.0 ]
                                }
                            },
                            {
                                "box": {
                                    "bubble": 1,
                                    "fontname": "Arial",
                                    "fontsize": 13.0,
                                    "id": "obj-19",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 122.5, 170.5, 139.0, 25.0 ],
                                    "text": "Fade in length (ms)"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-20",
                                    "maxclass": "number",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "", "bang" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 70.5, 170.5, 50.0, 23.0 ]
                                }
                            },
                            {
                                "box": {
                                    "bubble": 1,
                                    "fontname": "Arial",
                                    "fontsize": 13.0,
                                    "id": "obj-5",
                                    "maxclass": "comment",
                                    "numinlets": 1,
                                    "numoutlets": 0,
                                    "patching_rect": [ 238.606537, 127.5, 146.0, 25.0 ],
                                    "text": "Send buffer name(s)"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-4",
                                    "maxclass": "message",
                                    "numinlets": 2,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 15.0, 127.5, 198.0, 23.0 ],
                                    "text": "earsBufDrumLoop earsBufAnton"
                                }
                            },
                            {
                                "box": {
                                    "id": "obj-1",
                                    "maxclass": "newobj",
                                    "numinlets": 1,
                                    "numoutlets": 2,
                                    "outlettype": [ "float", "bang" ],
                                    "patching_rect": [ 431.0, 55.5, 222.0, 23.0 ],
                                    "text": "buffer~ earsBufDrumLoop drumLoop"
                                }
                            },
                            {
                                "box": {
                                    "border": 0,
                                    "filename": "helpargs.js",
                                    "id": "obj-15",
                                    "ignoreclick": 1,
                                    "jsarguments": [ "ears.fade~" ],
                                    "maxclass": "jsui",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 195.0, 244.0, 189.59249877929688, 69.0 ]
                                }
                            },
                            {
                                "box": {
                                    "fontname": "Arial",
                                    "fontsize": 13.0,
                                    "id": "obj-14",
                                    "maxclass": "newobj",
                                    "numinlets": 3,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "patching_rect": [ 15.0, 244.0, 174.5, 23.0 ],
                                    "text": "ears.fade~ 400 3000"
                                }
                            },
                            {
                                "box": {
                                    "border": 0,
                                    "filename": "helpdetails.js",
                                    "id": "obj-2",
                                    "ignoreclick": 1,
                                    "jsarguments": [ "ears.fade~", 90 ],
                                    "maxclass": "jsui",
                                    "numinlets": 1,
                                    "numoutlets": 1,
                                    "outlettype": [ "" ],
                                    "parameter_enable": 0,
                                    "patching_rect": [ 10.0, 10.0, 485.25, 100.0 ]
                                }
                            }
                        ],
                        "lines": [
                            {
                                "patchline": {
                                    "destination": [ "obj-20", 0 ],
                                    "hidden": 1,
                                    "source": [ "obj-10", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-7", 0 ],
                                    "hidden": 1,
                                    "source": [ "obj-11", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-18", 0 ],
                                    "source": [ "obj-12", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-21", 0 ],
                                    "order": 1,
                                    "source": [ "obj-14", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-26", 0 ],
                                    "order": 0,
                                    "source": [ "obj-14", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-3", 0 ],
                                    "order": 2,
                                    "source": [ "obj-14", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-12", 0 ],
                                    "source": [ "obj-16", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-9", 0 ],
                                    "source": [ "obj-18", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-14", 1 ],
                                    "source": [ "obj-20", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-22", 0 ],
                                    "source": [ "obj-21", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-25", 0 ],
                                    "source": [ "obj-26", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-17", 0 ],
                                    "hidden": 1,
                                    "source": [ "obj-27", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-27", 0 ],
                                    "hidden": 1,
                                    "source": [ "obj-29", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-14", 0 ],
                                    "source": [ "obj-4", 0 ]
                                }
                            },
                            {
                                "patchline": {
                                    "destination": [ "obj-14", 2 ],
                                    "source": [ "obj-7", 0 ]
                                }
                            }
                        ]
                    },
                    "patching_rect": [ 10.0, 85.0, 49.0, 22.0 ],
                    "saved_object_attributes": {
                        "fontsize": 13.0
                    },
                    "text": "p basic",
                    "varname": "basic_tab"
                }
            },
            {
                "box": {
                    "border": 0,
                    "filename": "helpname.js",
                    "id": "obj-4",
                    "ignoreclick": 1,
                    "jsarguments": [ "ears.fade~" ],
                    "maxclass": "jsui",
                    "numinlets": 1,
                    "numoutlets": 1,
                    "outlettype": [ "" ],
                    "parameter_enable": 0,
                    "patching_rect": [ 10.0, 10.0, 224.52798461914062, 57.599853515625 ]
                }
            },
            {
                "box": {
                    "fontname": "Arial",
                    "fontsize": 12.0,
                    "id": "obj-8",
                    "maxclass": "newobj",
                    "numinlets": 0,
                    "numoutlets": 0,
                    "patcher": {
                        "fileversion": 1,
                        "appversion": {
                            "major": 9,
                            "minor": 1,
                            "revision": 1,
                            "architecture": "x64",
                            "modernui": 1
                        },
                        "classnamespace": "box",
                        "rect": [ 0.0, 26.0, 658.0, 470.0 ],
                        "default_fontsize": 13.0,
                        "gridsize": [ 5.0, 5.0 ],
                        "showontab": 1,
                        "boxes": [],
                        "lines": []
                    },
                    "patching_rect": [ 205.0, 205.0, 50.0, 22.0 ],
                    "saved_object_attributes": {
                        "fontsize": 13.0
                    },
                    "text": "p ?",
                    "varname": "q_tab"
                }
            }
        ],
        "lines": [],
        "autosave": 0
    }
}