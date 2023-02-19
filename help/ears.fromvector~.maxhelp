{
	"patcher" : 	{
		"fileversion" : 1,
		"appversion" : 		{
			"major" : 8,
			"minor" : 5,
			"revision" : 3,
			"architecture" : "x64",
			"modernui" : 1
		}
,
		"classnamespace" : "box",
		"rect" : [ 100.0, 100.0, 985.0, 599.0 ],
		"bglocked" : 0,
		"openinpresentation" : 0,
		"default_fontsize" : 12.0,
		"default_fontface" : 0,
		"default_fontname" : "Arial",
		"gridonopen" : 1,
		"gridsize" : [ 5.0, 5.0 ],
		"gridsnaponopen" : 1,
		"objectsnaponopen" : 1,
		"statusbarvisible" : 2,
		"toolbarvisible" : 1,
		"lefttoolbarpinned" : 0,
		"toptoolbarpinned" : 0,
		"righttoolbarpinned" : 0,
		"bottomtoolbarpinned" : 0,
		"toolbars_unpinned_last_save" : 0,
		"tallnewobj" : 0,
		"boxanimatetime" : 200,
		"enablehscroll" : 1,
		"enablevscroll" : 1,
		"devicewidth" : 0.0,
		"description" : "",
		"digest" : "",
		"tags" : "",
		"style" : "",
		"subpatcher_template" : "",
		"showrootpatcherontab" : 0,
		"showontab" : 0,
		"assistshowspatchername" : 0,
		"boxes" : [ 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-1",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 311.0, 205.0, 162.0, 22.0 ],
					"saved_object_attributes" : 					{
						"filename" : "helpstarter.js",
						"parameter_enable" : 0
					}
,
					"text" : "js helpstarter.js ears.biquad~"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-2",
					"maxclass" : "newobj",
					"numinlets" : 0,
					"numoutlets" : 0,
					"patcher" : 					{
						"fileversion" : 1,
						"appversion" : 						{
							"major" : 8,
							"minor" : 5,
							"revision" : 3,
							"architecture" : "x64",
							"modernui" : 1
						}
,
						"classnamespace" : "box",
						"rect" : [ 100.0, 126.0, 985.0, 573.0 ],
						"bglocked" : 0,
						"openinpresentation" : 0,
						"default_fontsize" : 13.0,
						"default_fontface" : 0,
						"default_fontname" : "Arial",
						"gridonopen" : 1,
						"gridsize" : [ 5.0, 5.0 ],
						"gridsnaponopen" : 1,
						"objectsnaponopen" : 1,
						"statusbarvisible" : 2,
						"toolbarvisible" : 1,
						"lefttoolbarpinned" : 0,
						"toptoolbarpinned" : 0,
						"righttoolbarpinned" : 0,
						"bottomtoolbarpinned" : 0,
						"toolbars_unpinned_last_save" : 0,
						"tallnewobj" : 0,
						"boxanimatetime" : 200,
						"enablehscroll" : 1,
						"enablevscroll" : 1,
						"devicewidth" : 0.0,
						"description" : "",
						"digest" : "",
						"tags" : "",
						"style" : "",
						"subpatcher_template" : "",
						"showontab" : 1,
						"assistshowspatchername" : 0,
						"boxes" : [ 							{
								"box" : 								{
									"id" : "obj-58",
									"linecount" : 2,
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 18.0, 343.0, 276.0, 36.0 ],
									"text" : "N.B.: tempting as it may sound, you can't use ears.fromvector~ outside ears.process~"
								}

							}
, 							{
								"box" : 								{
									"buffername" : "u613011245",
									"id" : "obj-56",
									"maxclass" : "waveform~",
									"numinlets" : 5,
									"numoutlets" : 6,
									"outlettype" : [ "float", "float", "float", "float", "list", "" ],
									"patching_rect" : [ 10.0, 253.0, 256.0, 64.0 ]
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-55",
									"maxclass" : "message",
									"numinlets" : 2,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 10.0, 213.0, 44.0, 23.0 ],
									"text" : "set $1"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-53",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 10.0, 177.0, 205.0, 23.0 ],
									"text" : "ears.process~ ears.listGain-inside"
								}

							}
, 							{
								"box" : 								{
									"buffername" : "earsBufCherokee",
									"id" : "obj-52",
									"maxclass" : "waveform~",
									"numinlets" : 5,
									"numoutlets" : 6,
									"outlettype" : [ "float", "float", "float", "float", "list", "" ],
									"patching_rect" : [ 597.0, 14.0, 256.0, 64.0 ]
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-51",
									"maxclass" : "message",
									"numinlets" : 2,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 10.0, 144.5, 110.0, 23.0 ],
									"text" : "earsBufCherokee"
								}

							}
, 							{
								"box" : 								{
									"bubble" : 1,
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-50",
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 221.606537000000003, 177.0, 179.0, 25.0 ],
									"text" : "Double-click to look inside!"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-49",
									"maxclass" : "newobj",
									"numinlets" : 2,
									"numoutlets" : 5,
									"outlettype" : [ "bang", "", "", "", "" ],
									"patcher" : 									{
										"fileversion" : 1,
										"appversion" : 										{
											"major" : 8,
											"minor" : 5,
											"revision" : 3,
											"architecture" : "x64",
											"modernui" : 1
										}
,
										"classnamespace" : "box",
										"rect" : [ 59.0, 106.0, 420.0, 415.0 ],
										"bglocked" : 0,
										"openinpresentation" : 0,
										"default_fontsize" : 12.0,
										"default_fontface" : 0,
										"default_fontname" : "Arial",
										"gridonopen" : 1,
										"gridsize" : [ 15.0, 15.0 ],
										"gridsnaponopen" : 1,
										"objectsnaponopen" : 1,
										"statusbarvisible" : 2,
										"toolbarvisible" : 1,
										"lefttoolbarpinned" : 0,
										"toptoolbarpinned" : 0,
										"righttoolbarpinned" : 0,
										"bottomtoolbarpinned" : 0,
										"toolbars_unpinned_last_save" : 0,
										"tallnewobj" : 0,
										"boxanimatetime" : 200,
										"enablehscroll" : 1,
										"enablevscroll" : 1,
										"devicewidth" : 0.0,
										"description" : "",
										"digest" : "",
										"tags" : "",
										"style" : "",
										"subpatcher_template" : "",
										"assistshowspatchername" : 0,
										"boxes" : [ 											{
												"box" : 												{
													"id" : "obj-40",
													"maxclass" : "button",
													"numinlets" : 1,
													"numoutlets" : 1,
													"outlettype" : [ "bang" ],
													"parameter_enable" : 0,
													"patching_rect" : [ 30.0, 311.0, 24.0, 24.0 ]
												}

											}
, 											{
												"box" : 												{
													"id" : "obj-38",
													"maxclass" : "newobj",
													"numinlets" : 1,
													"numoutlets" : 1,
													"outlettype" : [ "" ],
													"patching_rect" : [ 320.199999999999989, 121.0, 79.0, 22.0 ],
													"saved_object_attributes" : 													{
														"versionnumber" : 80300
													}
,
													"text" : "bach.wrap 2"
												}

											}
, 											{
												"box" : 												{
													"id" : "obj-37",
													"maxclass" : "newobj",
													"numinlets" : 1,
													"numoutlets" : 1,
													"outlettype" : [ "" ],
													"patching_rect" : [ 224.0, 311.0, 68.0, 22.0 ],
													"saved_object_attributes" : 													{
														"versionnumber" : 80300
													}
,
													"text" : "bach.wrap"
												}

											}
, 											{
												"box" : 												{
													"id" : "obj-36",
													"maxclass" : "newobj",
													"numinlets" : 1,
													"numoutlets" : 1,
													"outlettype" : [ "" ],
													"patching_rect" : [ 146.0, 311.0, 68.0, 22.0 ],
													"saved_object_attributes" : 													{
														"versionnumber" : 80300
													}
,
													"text" : "bach.wrap"
												}

											}
, 											{
												"box" : 												{
													"id" : "obj-35",
													"maxclass" : "newobj",
													"numinlets" : 1,
													"numoutlets" : 1,
													"outlettype" : [ "" ],
													"patching_rect" : [ 65.0, 311.0, 68.0, 22.0 ],
													"saved_object_attributes" : 													{
														"versionnumber" : 80300
													}
,
													"text" : "bach.wrap"
												}

											}
, 											{
												"box" : 												{
													"id" : "obj-34",
													"maxclass" : "newobj",
													"numinlets" : 2,
													"numoutlets" : 1,
													"outlettype" : [ "" ],
													"patching_rect" : [ 93.0, 264.0, 68.0, 22.0 ],
													"text" : "bach.f2mc"
												}

											}
, 											{
												"box" : 												{
													"id" : "obj-31",
													"maxclass" : "newobj",
													"numinlets" : 3,
													"numoutlets" : 1,
													"outlettype" : [ "" ],
													"patching_rect" : [ 169.0, 264.0, 147.0, 22.0 ],
													"text" : "bach.autoscale 10. 127."
												}

											}
, 											{
												"box" : 												{
													"id" : "obj-30",
													"maxclass" : "newobj",
													"numinlets" : 1,
													"numoutlets" : 2,
													"outlettype" : [ "", "" ],
													"patching_rect" : [ 93.0, 229.0, 95.0, 22.0 ],
													"saved_object_attributes" : 													{
														"versionnumber" : 80300
													}
,
													"text" : "bach.pick 1 2"
												}

											}
, 											{
												"box" : 												{
													"id" : "obj-27",
													"maxclass" : "newobj",
													"numinlets" : 1,
													"numoutlets" : 1,
													"outlettype" : [ "" ],
													"patching_rect" : [ 93.0, 196.0, 91.0, 22.0 ],
													"saved_object_attributes" : 													{
														"versionnumber" : 80300
													}
,
													"text" : "bach.trans 2 1"
												}

											}
, 											{
												"box" : 												{
													"id" : "obj-25",
													"maxclass" : "newobj",
													"numinlets" : 1,
													"numoutlets" : 2,
													"outlettype" : [ "", "" ],
													"patching_rect" : [ 27.0, 160.0, 85.0, 22.0 ],
													"saved_object_attributes" : 													{
														"versionnumber" : 80300
													}
,
													"text" : "bach.pick 1 2"
												}

											}
, 											{
												"box" : 												{
													"id" : "obj-24",
													"maxclass" : "newobj",
													"numinlets" : 1,
													"numoutlets" : 1,
													"outlettype" : [ "" ],
													"patching_rect" : [ 27.0, 121.0, 69.0, 22.0 ],
													"saved_object_attributes" : 													{
														"versionnumber" : 80300
													}
,
													"text" : "bach.trans"
												}

											}
, 											{
												"box" : 												{
													"id" : "obj-5",
													"maxclass" : "newobj",
													"numinlets" : 1,
													"numoutlets" : 1,
													"outlettype" : [ "" ],
													"patching_rect" : [ 27.0, 81.0, 74.0, 22.0 ],
													"saved_object_attributes" : 													{
														"versionnumber" : 80300
													}
,
													"text" : "bach.pick 1"
												}

											}
, 											{
												"box" : 												{
													"comment" : "",
													"id" : "obj-42",
													"index" : 1,
													"maxclass" : "inlet",
													"numinlets" : 0,
													"numoutlets" : 1,
													"outlettype" : [ "" ],
													"patching_rect" : [ 27.0, 21.0, 30.0, 30.0 ]
												}

											}
, 											{
												"box" : 												{
													"comment" : "",
													"id" : "obj-43",
													"index" : 2,
													"maxclass" : "inlet",
													"numinlets" : 0,
													"numoutlets" : 1,
													"outlettype" : [ "" ],
													"patching_rect" : [ 320.200012000000015, 21.0, 30.0, 30.0 ]
												}

											}
, 											{
												"box" : 												{
													"comment" : "",
													"id" : "obj-44",
													"index" : 1,
													"maxclass" : "outlet",
													"numinlets" : 1,
													"numoutlets" : 0,
													"patching_rect" : [ 30.0, 361.0, 30.0, 30.0 ]
												}

											}
, 											{
												"box" : 												{
													"comment" : "",
													"id" : "obj-45",
													"index" : 2,
													"maxclass" : "outlet",
													"numinlets" : 1,
													"numoutlets" : 0,
													"patching_rect" : [ 65.0, 361.0, 30.0, 30.0 ]
												}

											}
, 											{
												"box" : 												{
													"comment" : "",
													"id" : "obj-46",
													"index" : 3,
													"maxclass" : "outlet",
													"numinlets" : 1,
													"numoutlets" : 0,
													"patching_rect" : [ 146.0, 361.0, 30.0, 30.0 ]
												}

											}
, 											{
												"box" : 												{
													"comment" : "",
													"id" : "obj-47",
													"index" : 5,
													"maxclass" : "outlet",
													"numinlets" : 1,
													"numoutlets" : 0,
													"patching_rect" : [ 300.200000000000045, 361.0, 30.0, 30.0 ]
												}

											}
, 											{
												"box" : 												{
													"comment" : "",
													"id" : "obj-48",
													"index" : 4,
													"maxclass" : "outlet",
													"numinlets" : 1,
													"numoutlets" : 0,
													"patching_rect" : [ 227.5, 361.0, 30.0, 30.0 ]
												}

											}
 ],
										"lines" : [ 											{
												"patchline" : 												{
													"destination" : [ "obj-25", 0 ],
													"source" : [ "obj-24", 0 ]
												}

											}
, 											{
												"patchline" : 												{
													"destination" : [ "obj-27", 0 ],
													"source" : [ "obj-25", 1 ]
												}

											}
, 											{
												"patchline" : 												{
													"destination" : [ "obj-35", 0 ],
													"order" : 0,
													"source" : [ "obj-25", 0 ]
												}

											}
, 											{
												"patchline" : 												{
													"destination" : [ "obj-40", 0 ],
													"order" : 1,
													"source" : [ "obj-25", 0 ]
												}

											}
, 											{
												"patchline" : 												{
													"destination" : [ "obj-30", 0 ],
													"source" : [ "obj-27", 0 ]
												}

											}
, 											{
												"patchline" : 												{
													"destination" : [ "obj-31", 0 ],
													"source" : [ "obj-30", 1 ]
												}

											}
, 											{
												"patchline" : 												{
													"destination" : [ "obj-34", 0 ],
													"source" : [ "obj-30", 0 ]
												}

											}
, 											{
												"patchline" : 												{
													"destination" : [ "obj-37", 0 ],
													"source" : [ "obj-31", 0 ]
												}

											}
, 											{
												"patchline" : 												{
													"destination" : [ "obj-36", 0 ],
													"source" : [ "obj-34", 0 ]
												}

											}
, 											{
												"patchline" : 												{
													"destination" : [ "obj-45", 0 ],
													"source" : [ "obj-35", 0 ]
												}

											}
, 											{
												"patchline" : 												{
													"destination" : [ "obj-46", 0 ],
													"source" : [ "obj-36", 0 ]
												}

											}
, 											{
												"patchline" : 												{
													"destination" : [ "obj-47", 0 ],
													"source" : [ "obj-37", 0 ]
												}

											}
, 											{
												"patchline" : 												{
													"destination" : [ "obj-48", 0 ],
													"source" : [ "obj-38", 0 ]
												}

											}
, 											{
												"patchline" : 												{
													"destination" : [ "obj-44", 0 ],
													"source" : [ "obj-40", 0 ]
												}

											}
, 											{
												"patchline" : 												{
													"destination" : [ "obj-5", 0 ],
													"source" : [ "obj-42", 0 ]
												}

											}
, 											{
												"patchline" : 												{
													"destination" : [ "obj-38", 0 ],
													"source" : [ "obj-43", 0 ]
												}

											}
, 											{
												"patchline" : 												{
													"destination" : [ "obj-24", 0 ],
													"source" : [ "obj-5", 0 ]
												}

											}
 ]
									}
,
									"patching_rect" : [ 431.0, 217.0, 312.0, 23.0 ],
									"saved_object_attributes" : 									{
										"description" : "",
										"digest" : "",
										"globalpatchername" : "",
										"tags" : ""
									}
,
									"text" : "p format"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-41",
									"maxclass" : "newobj",
									"numinlets" : 3,
									"numoutlets" : 2,
									"outlettype" : [ "", "int" ],
									"patching_rect" : [ 865.571428571428442, 541.0, 113.0, 23.0 ],
									"text" : "bach.ezmidiplay 8"
								}

							}
, 							{
								"box" : 								{
									"bwcompatibility" : 80300,
									"clefs" : [ "FFGG" ],
									"defaultnoteslots" : [ "null" ],
									"embed" : 0,
									"fontface" : 0,
									"fontname" : "Arial",
									"fontsize" : 12.0,
									"id" : "obj-12",
									"loop" : [ 0.0, 1000.0 ],
									"maxclass" : "bach.roll",
									"numinlets" : 6,
									"numoutlets" : 8,
									"numvoices" : 1,
									"out" : "nnnnnnn",
									"outlettype" : [ "", "", "", "", "", "", "", "bang" ],
									"patching_rect" : [ 431.0, 261.0, 526.0, 264.833333333333371 ],
									"pitcheditrange" : [ "null" ],
									"showvelocity" : 1,
									"stafflines" : [ 5 ],
									"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ],
									"tonedivision" : 8,
									"versionnumber" : 80300,
									"voicenames" : [ "[", "]" ],
									"voicespacing" : [ 0.0, 17.0 ],
									"zoom" : 175.636819005012512
								}

							}
, 							{
								"box" : 								{
									"bubble" : 1,
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-9",
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 752.606537000000003, 178.0, 152.0, 25.0 ],
									"text" : "A more advanced one"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-4",
									"maxclass" : "message",
									"numinlets" : 2,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 431.0, 140.5, 110.0, 23.0 ],
									"text" : "earsBufCherokee"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-1",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 2,
									"outlettype" : [ "float", "bang" ],
									"patching_rect" : [ 597.0, 80.0, 211.0, 23.0 ],
									"text" : "buffer~ earsBufCherokee cherokee"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-14",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 2,
									"outlettype" : [ "", "" ],
									"patching_rect" : [ 431.0, 178.0, 312.0, 23.0 ],
									"text" : "ears.process~ ears.naiveAnalysis-insideearsprocess"
								}

							}
, 							{
								"box" : 								{
									"border" : 0,
									"filename" : "helpdetails.js",
									"id" : "obj-2",
									"ignoreclick" : 1,
									"jsarguments" : [ "ears.fromvector~", 80 ],
									"maxclass" : "jsui",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"parameter_enable" : 0,
									"patching_rect" : [ 10.0, 10.0, 547.25, 122.0 ]
								}

							}
 ],
						"lines" : [ 							{
								"patchline" : 								{
									"destination" : [ "obj-41", 0 ],
									"source" : [ "obj-12", 6 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-49", 1 ],
									"source" : [ "obj-14", 1 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-49", 0 ],
									"source" : [ "obj-14", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-14", 0 ],
									"source" : [ "obj-4", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-12", 4 ],
									"source" : [ "obj-49", 4 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-12", 3 ],
									"source" : [ "obj-49", 3 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-12", 2 ],
									"source" : [ "obj-49", 2 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-12", 1 ],
									"source" : [ "obj-49", 1 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-12", 0 ],
									"source" : [ "obj-49", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-53", 0 ],
									"source" : [ "obj-51", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-55", 0 ],
									"source" : [ "obj-53", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-56", 0 ],
									"source" : [ "obj-55", 0 ]
								}

							}
 ]
					}
,
					"patching_rect" : [ 10.0, 85.0, 49.0, 22.0 ],
					"saved_object_attributes" : 					{
						"description" : "",
						"digest" : "",
						"fontsize" : 13.0,
						"globalpatchername" : "",
						"tags" : ""
					}
,
					"text" : "p basic",
					"varname" : "basic_tab"
				}

			}
, 			{
				"box" : 				{
					"border" : 0,
					"filename" : "helpname.js",
					"id" : "obj-4",
					"ignoreclick" : 1,
					"jsarguments" : [ "ears.biquad~" ],
					"maxclass" : "jsui",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 10.0, 10.0, 274.399993896484375, 57.599853515625 ]
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-8",
					"maxclass" : "newobj",
					"numinlets" : 0,
					"numoutlets" : 0,
					"patcher" : 					{
						"fileversion" : 1,
						"appversion" : 						{
							"major" : 8,
							"minor" : 5,
							"revision" : 3,
							"architecture" : "x64",
							"modernui" : 1
						}
,
						"classnamespace" : "box",
						"rect" : [ 0.0, 26.0, 985.0, 573.0 ],
						"bglocked" : 0,
						"openinpresentation" : 0,
						"default_fontsize" : 13.0,
						"default_fontface" : 0,
						"default_fontname" : "Arial",
						"gridonopen" : 1,
						"gridsize" : [ 5.0, 5.0 ],
						"gridsnaponopen" : 1,
						"objectsnaponopen" : 1,
						"statusbarvisible" : 2,
						"toolbarvisible" : 1,
						"lefttoolbarpinned" : 0,
						"toptoolbarpinned" : 0,
						"righttoolbarpinned" : 0,
						"bottomtoolbarpinned" : 0,
						"toolbars_unpinned_last_save" : 0,
						"tallnewobj" : 0,
						"boxanimatetime" : 200,
						"enablehscroll" : 1,
						"enablevscroll" : 1,
						"devicewidth" : 0.0,
						"description" : "",
						"digest" : "",
						"tags" : "",
						"style" : "",
						"subpatcher_template" : "",
						"showontab" : 1,
						"assistshowspatchername" : 0,
						"boxes" : [  ],
						"lines" : [  ]
					}
,
					"patching_rect" : [ 205.0, 205.0, 50.0, 22.0 ],
					"saved_object_attributes" : 					{
						"description" : "",
						"digest" : "",
						"fontsize" : 13.0,
						"globalpatchername" : "",
						"tags" : ""
					}
,
					"text" : "p ?",
					"varname" : "q_tab"
				}

			}
 ],
		"lines" : [  ],
		"dependency_cache" : [ 			{
				"name" : "bach.approx.maxpat",
				"bootpath" : "~/Documents/Max 8/Packages/bach/patchers",
				"patcherrelativepath" : "../../bach/patchers",
				"type" : "JSON",
				"implicit" : 1
			}
, 			{
				"name" : "bach.args.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.autoscale.maxpat",
				"bootpath" : "~/Documents/Max 8/Packages/bach/patchers",
				"patcherrelativepath" : "../../bach/patchers",
				"type" : "JSON",
				"implicit" : 1
			}
, 			{
				"name" : "bach.collect.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.eq.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.eval.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.expr.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.ezmidiplay.maxpat",
				"bootpath" : "~/Documents/Max 8/Packages/bach/patchers",
				"patcherrelativepath" : "../../bach/patchers",
				"type" : "JSON",
				"implicit" : 1
			}
, 			{
				"name" : "bach.f2mc.maxpat",
				"bootpath" : "~/Documents/Max 8/Packages/bach/patchers",
				"patcherrelativepath" : "../../bach/patchers",
				"type" : "JSON",
				"implicit" : 1
			}
, 			{
				"name" : "bach.filter.maxpat",
				"bootpath" : "~/Documents/Max 8/Packages/bach/patchers",
				"patcherrelativepath" : "../../bach/patchers",
				"type" : "JSON",
				"implicit" : 1
			}
, 			{
				"name" : "bach.flat.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.gcd.maxpat",
				"bootpath" : "~/Documents/Max 8/Packages/bach/patchers",
				"patcherrelativepath" : "../../bach/patchers",
				"type" : "JSON",
				"implicit" : 1
			}
, 			{
				"name" : "bach.gt.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.is.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.iter.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.keys.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.length.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.mapelem.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.minmax.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.neq.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.nth.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.pick.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.playkeys.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.portal.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.print.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.reg.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.roll.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.slice.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.trans.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.wrap.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.fromvector~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.in~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.listGain-inside.maxpat",
				"bootpath" : "~/Documents/Max 8/Packages/ears/help",
				"patcherrelativepath" : ".",
				"type" : "JSON",
				"implicit" : 1
			}
, 			{
				"name" : "ears.naiveAnalysis-insideearsprocess.maxpat",
				"bootpath" : "~/Documents/Max 8/Packages/ears/help",
				"patcherrelativepath" : ".",
				"type" : "JSON",
				"implicit" : 1
			}
, 			{
				"name" : "ears.naiveAnalysis-insidepfft.maxpat",
				"bootpath" : "~/Documents/Max 8/Packages/ears/help",
				"patcherrelativepath" : ".",
				"type" : "JSON",
				"implicit" : 1
			}
, 			{
				"name" : "ears.out.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.out~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.processinfo~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.process~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.tovector~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "helpdetails.js",
				"bootpath" : "C74:/help/resources",
				"type" : "TEXT",
				"implicit" : 1
			}
, 			{
				"name" : "helpname.js",
				"bootpath" : "C74:/help/resources",
				"type" : "TEXT",
				"implicit" : 1
			}
, 			{
				"name" : "helpstarter.js",
				"bootpath" : "C74:/help/resources",
				"type" : "TEXT",
				"implicit" : 1
			}
 ],
		"autosave" : 0
	}

}
