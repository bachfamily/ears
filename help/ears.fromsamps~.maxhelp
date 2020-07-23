{
	"patcher" : 	{
		"fileversion" : 1,
		"appversion" : 		{
			"major" : 8,
			"minor" : 0,
			"revision" : 5,
			"architecture" : "x64",
			"modernui" : 1
		}
,
		"classnamespace" : "box",
		"rect" : [ 106.0, 98.0, 693.0, 486.0 ],
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
		"boxes" : [ 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-1",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 311.0, 205.0, 185.0, 22.0 ],
					"saved_object_attributes" : 					{
						"filename" : "helpstarter.js",
						"parameter_enable" : 0
					}
,
					"text" : "js helpstarter.js ears.fromsamps~"
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
							"minor" : 0,
							"revision" : 5,
							"architecture" : "x64",
							"modernui" : 1
						}
,
						"classnamespace" : "box",
						"rect" : [ 106.0, 124.0, 693.0, 460.0 ],
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
						"boxes" : [ 							{
								"box" : 								{
									"bubble" : 1,
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-50",
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 400.0, 246.0, 253.0, 25.0 ],
									"text" : "(this automatically creates new buffers)"
								}

							}
, 							{
								"box" : 								{
									"bubble" : 1,
									"bubblepoint" : 0.0,
									"bubbleside" : 0,
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-48",
									"linecount" : 2,
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 65.5, 326.0, 175.0, 55.0 ],
									"text" : "This fills the buffer \"earsFromSampsHelp\""
								}

							}
, 							{
								"box" : 								{
									"bubble" : 1,
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-47",
									"linecount" : 2,
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 146.5, 131.0, 107.0, 40.0 ],
									"text" : "Send a list of numbers..."
								}

							}
, 							{
								"box" : 								{
									"bubble" : 1,
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-46",
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 77.5, 217.5, 128.0, 25.0 ],
									"text" : "...or any native llll"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-45",
									"maxclass" : "button",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "bang" ],
									"parameter_enable" : 0,
									"patching_rect" : [ 47.5, 218.0, 24.0, 24.0 ]
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-6",
									"maxclass" : "newobj",
									"numinlets" : 2,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 47.5, 252.0, 128.0, 23.0 ],
									"reg_data_0000000000" : [ "[", "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066164224, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065930752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065820160, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1056964608, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1061224448, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1060896768, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1059061760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1060372480, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1062207488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1062207488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1061879808, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1062535168, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1062633472, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1063485440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1064812544, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065545728, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065590784, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065197568, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1064910848, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065205760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065295872, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065451520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065754624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065787392, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065861120, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065992192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065979904, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066233856, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066610688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066930176, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067042816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067284480, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067688960, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067919360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067952128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067916288, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067771904, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067585536, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067480064, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067474944, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067604992, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067837440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068048384, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068189696, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068130304, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067852800, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067804672, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067560960, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065873408, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3213361152, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214940160, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215292416, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215101952, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214919680, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214923776, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214530560, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214252032, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214772224, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215269888, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215416320, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215707136, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216064512, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216045056, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216058880, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216266240, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216419328, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216547840, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216754176, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217011712, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217061376, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217021440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217175296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217333504, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217310208, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217346560, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217453824, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217493504, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217504768, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217599488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217665792, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217691648, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217669632, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217684736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217890816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218113920, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218161408, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218164736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218194688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218171392, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218123392, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218140032, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218167424, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218133120, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218014720, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217981696, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218069760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218085760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218076416, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218127872, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218178560, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218166656, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218186368, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218286464, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218358272, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218333184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218236288, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218221568, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218348800, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218383872, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218220800, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217996032, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218082816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218330496, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218373760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218142464, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217916160, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218066944, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218231808, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218304000, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217875456, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216381952, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216281088, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217661440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218215680, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218376192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218003200, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3213877248, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068673536, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215112192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217662464, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218546816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218332928, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216412672, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069000192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070537984, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070407680, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067713536, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217472256, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218656896, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218847232, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218384640, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217062912, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069482496, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070655872, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070903040, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071052032, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071063296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070935168, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070241280, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066754048, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217279488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218165376, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218515712, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218781184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218771712, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218693120, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218617856, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218609152, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218709120, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218587392, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218304128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218118272, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217880320, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217672192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217693440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217699840, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217467136, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216997376, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215968256, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3213213696, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067507712, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068427264, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068685824, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068807168, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068795392, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068067840, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3211804672, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3212820480, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067639808, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068728832, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069192704, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068996608, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067935744, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065799680, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214235648, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3213250560, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068189696, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069053440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069235200, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069235712, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068872704, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068036096, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066541056, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1064599552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214419968, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215341568, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066692608, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069074944, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069352960, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068962816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068660736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068111872, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066381312, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066725376, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068683776, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069259264, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069385728, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069557760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069639424, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069914880, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070285312, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070391040, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070371584, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070432512, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070301440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070085888, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070125056, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070204928, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070181376, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069894144, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069549056, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069628672, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069985280, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070429696, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070744960, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070758144, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070588160, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070587136, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070590208, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070491648, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070707584, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070710272, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070186752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070010368, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070164736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070349568, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070816000, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071187968, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071255936, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071034112, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070797056, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070757248, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070714496, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070732160, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070997760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071220224, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071269248, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071143040, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070912640, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070748672, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070661632, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070675072, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070695808, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070702592, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070779520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070853376, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070927360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070986880, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071000448, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071023616, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071037440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071048832, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071095424, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071138304, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071116288, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071067648, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071028224, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071002624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071069312, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071091456, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070932224, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070705536, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070645248, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070790912, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070856320, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070818432, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070912512, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071108992, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071186688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071119488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071015552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070964096, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070929280, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070853888, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070731520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070675712, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070733440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070724352, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070663552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070544896, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070298880, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070348288, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070632960, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070709504, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070747776, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070797568, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070692992, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070413824, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070211072, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070359040, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070488832, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070433536, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070571776, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070752384, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070896128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070851200, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070704768, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070512640, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070196736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070227712, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070556416, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070509568, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070178048, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070025728, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069978368, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069755648, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069325824, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069159424, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069052416, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068967936, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069233152, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069165056, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068260352, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214297088, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216824320, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217190144, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216877056, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216522752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216249856, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216461312, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217132800, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217310208, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217213440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217253120, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217348352, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217259776, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217383424, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217879296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218156032, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218150784, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218193152, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218261248, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218142720, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217574400, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216871424, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216801792, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217101824, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217167872, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217499136, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217844224, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217856512, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217578496, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217174272, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217015808, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216914944, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217100032, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217480960, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218003712, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218285696, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218401280, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218480384, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218465280, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218336512, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218375552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218549504, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218577280, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218436480, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218374912, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218440960, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218583680, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218738432, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218744448, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218671488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218663424, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218675584, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218730240, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218816896, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218845440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218897664, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218959360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218931456, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218883328, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218891520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219043584, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219196928, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219208000, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219158656, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219140736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219122816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219106176, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219082880, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219154176, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219312512, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219420672, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219397760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219279872, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219118720, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218983296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218969344, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219056000, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219243776, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219390272, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219451136, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219402240, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219281344, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219182208, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219123584, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219148608, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219232896, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219320960, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219399488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219432576, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219426688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219395328, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219337536, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219291264, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219257280, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219246848, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219268416, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219313792, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219377920, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219441344, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219485952, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219494784, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219452544, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219380672, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219290816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219237184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219223360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219220480, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219205824, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219218752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219313984, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219376960, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219347648, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219319424, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219312128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219264064, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219189824, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219153024, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219136256, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219136704, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219134272, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219098880, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219129920, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219188736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219230784, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219196096, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219135744, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219073152, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218974592, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218890624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218885248, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218886912, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218850432, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218888704, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219001088, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219076864, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219163776, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219188160, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219050880, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218825984, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218767232, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218741376, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218623488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218556160, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218603648, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218658944, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218716288, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218760832, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218749568, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218688384, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218618752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218622208, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218663936, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218631552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218557056, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218478336, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218371840, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218306816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218251648, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218173568, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218152192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218157440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218136704, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218054144, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217977088, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218086400, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218154624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218172544, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218194432, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218301824, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218441216, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218384384, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218190720, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218046464, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217785600, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217410816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217112576, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217271552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217743616, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217935104, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217892352, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217611264, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217080576, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216420352, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216137728, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215423488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3213840384, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3213725696, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214530560, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3213910016, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214123008, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215496192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216227328, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216738816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217236736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217486336, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217572608, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217443072, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217114368, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216582144, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216126464, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215474688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214114816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1061879808, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065668608, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065590784, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3212075008, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214839808, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215580160, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215855616, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216110080, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216067072, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215580160, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214876672, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3211558912, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1064345600, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3212959744, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3212873728, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3211878400, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065263104, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067151360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068094464, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068949504, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069616128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069939200, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070090752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070046720, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070000896, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069996032, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069841152, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069555456, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069430272, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069591552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069587968, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069639936, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069911040, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070166016, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070364672, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070644352, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070731392, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070669056, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070522624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070190592, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070040320, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070015232, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070129152, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070393856, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070503680, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070724096, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070866048, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070722688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070692096, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070886912, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070913920, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071001216, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071075840, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071125632, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071237888, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071166336, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071225600, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071234944, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071291136, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071288704, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071277440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071419776, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071400064, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071384064, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071312000, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071336576, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071311488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071283200, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071434752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071402112, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071294848, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071299328, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071420416, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071678656, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071710208, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071496448, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071344512, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071433216, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071480320, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071551488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071714816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071722304, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071841408, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071951040, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071945216, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1072003264, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071944384, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071926976, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071894016, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071852032, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071876288, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071805952, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071735424, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071719360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071697280, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071666048, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071748096, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071857984, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071841856, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071788096, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071774336, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071776128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071662592, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071461120, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071541760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071693248, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071803136, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071847168, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071915264, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071887424, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071802688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071853376, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071757824, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071752896, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071757632, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071750784, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071816512, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071680448, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071833344, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071912192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071878784, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071889280, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071708288, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071790016, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071707264, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071626240, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071643776, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071456128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071709760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071691712, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071696640, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071727872, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071717952, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071667584, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071581440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071762560, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071651008, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071533056, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071648128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071708736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071663424, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071349376, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071323136, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071507328, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071601280, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071149440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071205504, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071361920, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071376256, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071333632, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071050752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071111296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070844672, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070783104, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070859776, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071053312, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071143552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070877696, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071133184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071229696, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070754560, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070633984, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070963840, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070736128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070808960, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070933120, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070673920, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070877440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070868992, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070754816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070679680, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071116288, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071045376, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070322432, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070743040, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070472192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070570752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070823424, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070650368, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070908416, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070771584, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070558976, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070718592, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070351360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070054912, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070069504, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069195776, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070151680, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070359296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069894144, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070848768, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070847744, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070472192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070318336, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070203904, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069743616, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069256704, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069704192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070495744, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070684416, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070805888, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070966912, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070010112, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070208256, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070443008, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069125120, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069720320, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069021184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214962688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214487552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066498048, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067620352, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066493952, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067532288, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069170176, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070014208, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070049792, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070310912, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070665984, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070420736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070268672, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069634560, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069236736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068679168, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069374976, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069895680, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068626432, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069563392, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069622016, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068855296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068889600, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069421056, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069952000, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069661184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069580032, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069832704, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069695488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069738496, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069891584, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069679616, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070033152, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069914112, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069669120, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069828096, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069832192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069846784, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069346304, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070110976, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070218752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069056512, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069269504, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069386240, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069738240, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069992192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069704448, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069505024, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070017792, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070356992, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069871104, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070170624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070080000, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069769728, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070036224, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070029824, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069984256, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070130176, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070413824, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069675776, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069669888, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069811712, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069730048, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070231296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069717504, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069466624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070066688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070517248, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070631552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070307328, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069585152, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069811968, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069227520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069384704, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070057216, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067801600, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067671552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068620288, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068655104, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068546048, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067482112, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214286848, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215106048, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067660288, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067821056, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068546048, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066590208, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066278912, "]", "[", "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 0, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066164224, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065930752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065820160, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1056964608, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1060634624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1060634624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1061748736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1062076416, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1061486592, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1061355520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1062141952, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1062436864, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1061748736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1063157760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1064812544, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065570304, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065705472, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065476096, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065271296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065472000, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065402368, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065369600, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065832448, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066086400, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066127360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066188800, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066135552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066278912, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066522624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066835968, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067200512, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067510784, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067744256, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067966464, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068101632, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068023808, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067813888, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067680768, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067508736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067345920, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067692032, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068115968, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068329984, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068329984, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068195840, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068035072, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067918336, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067772928, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067157504, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1063616512, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214106624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214600192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214768128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214911488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214860288, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214673920, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214489600, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214209024, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214176256, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215008768, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215765504, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216014336, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215929344, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216007680, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216124416, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216209408, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216391168, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216631296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216588800, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216660992, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217104384, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217314816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217254400, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217133824, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217202432, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217372160, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217448192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217414656, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217509632, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217646592, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217601536, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217580544, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217801216, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218018816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218017536, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218054144, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218148992, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218209408, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218229120, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218217600, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218158592, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218091776, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218091520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218104832, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218062080, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217962752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217999616, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218117504, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218184064, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218148992, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218105600, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218218752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218387328, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218408448, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218333568, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218291456, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218297984, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218332928, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218329344, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218257152, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218126336, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218131584, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218390400, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218526720, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218284800, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218018304, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218110720, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218318336, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218457472, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218194560, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217125632, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216594432, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217567488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218219648, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218480000, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218108800, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214884864, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068470272, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3212689408, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217371904, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218585984, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218576256, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217488640, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066688512, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070455296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070615296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068508672, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217419520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218680832, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218965632, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218715136, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217873920, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067717632, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070247680, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070824448, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071103616, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071192448, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071031552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070379520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067776000, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217088768, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218114944, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218471168, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218728192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218834688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218836608, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218785152, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218687872, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218621184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218574464, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218516864, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218340608, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218073856, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217793536, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217810688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217802496, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217490176, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217054720, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216219136, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214424064, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066899456, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068372992, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068851712, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068957184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068882944, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068529664, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066639360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1064525824, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067220992, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068522496, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069221888, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069261824, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068707328, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067782144, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065951232, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066797056, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068482560, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069052928, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069483008, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069512192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069157888, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068509184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1065783296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214618624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215630336, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215453184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1064747008, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068322816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068797440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068543488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067577344, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066356736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1064697856, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066878976, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068496896, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068923904, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068914688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069259264, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069889792, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070309888, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070491904, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070608128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070694016, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070649856, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070298368, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070148352, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070347520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070449920, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070500608, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070304256, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069810432, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069679360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069899264, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070422528, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070922240, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071056896, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070812672, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070640512, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070669440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070738304, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070859776, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070840448, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070604032, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070318080, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070266112, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070619648, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071040256, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071297280, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071337728, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071209088, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070929280, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070770688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070746752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070860928, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071121664, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071298560, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071346944, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071240960, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071012352, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070826496, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070678400, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070608512, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070597248, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070604160, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070652928, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070701696, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070748544, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070842368, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070940672, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070981632, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070950912, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070954368, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071021312, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071073024, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071092864, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071087360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071052416, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071025920, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071114752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071145472, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070990848, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070795776, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070705152, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070773504, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070849536, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070893952, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070980608, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071060736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071108992, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071180032, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071239168, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071180928, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071074048, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071000960, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070930944, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070889088, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070851840, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070704640, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070537472, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070544384, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070509824, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070439168, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070616320, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070771200, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070837632, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070760448, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070564864, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070372352, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070305280, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070199296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070135552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070292224, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070631040, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070747264, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070808448, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070777344, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070601856, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070285312, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070201600, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070457600, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070717312, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070746752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070678144, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070606720, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070421504, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070190336, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069888000, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069699072, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069624832, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069619456, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069722624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069712128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069530624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068709888, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3213746176, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216312832, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216413184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216036864, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215414272, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215777792, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216368128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216418816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216272384, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216432640, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216659456, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216716800, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217149696, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217744128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218099456, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218211840, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218433280, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218630272, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218639360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218335232, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217619200, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217085184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216960000, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217113600, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217435904, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217647616, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217595392, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217340416, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216909312, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216740864, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216933888, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217113088, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217326336, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217643264, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218041600, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218226432, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218398720, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218489088, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218454144, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218535296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218735616, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218832384, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218832768, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218794496, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218670848, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218651008, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218816128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218929408, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218914560, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218925184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219025664, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219130432, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219169536, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219202304, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219239040, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219230144, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219156608, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219111808, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219129920, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219151616, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219199680, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219225280, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219231680, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219195904, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219110400, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219129792, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219217088, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219315072, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219438912, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219524288, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219556992, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219501568, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219347584, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219226240, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219187264, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219262464, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219422080, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219545024, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219625792, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219618432, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219528128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219395520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219278656, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219318080, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219457152, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219538880, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219571520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219587328, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219592448, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219544256, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219447232, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219411136, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219429568, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219428800, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219414528, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219430016, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219489728, "_x_x_x_x_bach_float64_x_x_x_x_" ],
									"reg_data_0000000001" : [ 0, 3219537088, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219546688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219539520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219475200, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219376320, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219322688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219296064, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219240768, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219188736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219189568, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219259712, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219377472, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219382720, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219279488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219210496, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219193984, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219193664, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219173184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219174080, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219246784, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219309184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219281152, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219200640, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219168896, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219206656, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219230016, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219191040, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219164608, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219153280, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219032704, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218907136, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218879616, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218868352, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218854528, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218916096, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219011328, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219027968, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219065600, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219099136, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219008000, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218830464, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218753280, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218799360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218805376, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218748288, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218739200, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218735232, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218634880, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218560384, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218594688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218734208, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218847488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218902784, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218978944, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219066624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3219092992, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218928256, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218656256, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218489472, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218397312, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218268160, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218099712, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217944064, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218055936, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218159488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218174208, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218143744, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218181248, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218234240, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218203392, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218222080, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218325120, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218291072, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3218113152, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217691648, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217136896, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216685568, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216828928, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217115136, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217431808, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217487616, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3217176832, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216501760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215162368, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3212484608, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3211001856, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3212881920, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214108672, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214610432, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214833664, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214682112, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214516224, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214790656, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214532608, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214759936, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215609856, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216086016, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216171008, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216158720, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216330240, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216641024, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216628736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3216327680, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215997440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215546368, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3215000576, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3214278656, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3213733888, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3212861440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3212173312, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3213139968, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3213965312, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 3211345920, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1066694656, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067724800, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068574720, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068857856, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068893184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068851200, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068696064, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068822016, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069220864, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069466112, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069509632, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069470720, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069451776, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069561344, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069686784, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069833984, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069947904, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069971200, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070059264, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070147328, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070122240, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070193664, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070475520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070631296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070599552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070581248, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070674688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070734720, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070676224, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070616704, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070589952, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070620032, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070658944, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070756736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071006592, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071156480, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071200896, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071276928, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071278336, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071245824, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071307520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071393920, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071539200, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071653184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071666432, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071700672, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071663552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071651072, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071703680, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071717760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071666816, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071605760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071680896, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071677056, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071629056, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071597440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071591552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071578880, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071527168, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071650112, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071742912, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071756480, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071754688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071793216, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071856000, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071891264, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071873216, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071844736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071856384, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071861184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071812992, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071796032, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071752960, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071774848, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071878976, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071900288, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071919488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071857984, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071763712, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071737600, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071735744, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071747520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071698496, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071636096, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071617536, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071619840, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071621888, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071713984, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071822016, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071841088, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071827136, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071777472, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071717888, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071661568, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071506560, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071429888, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071454336, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071514368, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071495424, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071403136, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071328512, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071317760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071451904, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071415424, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071404800, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071534848, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071605888, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071717760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071694336, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071730752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071786304, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071730688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071740096, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071621760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071601408, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071599360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071389440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071313792, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071177216, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071317248, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071355776, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071262976, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071310208, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071256320, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071170304, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071077120, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1071152128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070950016, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070714240, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070785408, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070862592, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070820608, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070653568, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070667392, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070723712, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070730624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070458368, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070364672, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070606208, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070659328, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070731520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070624384, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070728448, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070654976, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070417920, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070470400, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070569216, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070729216, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070700928, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070768000, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070816384, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070599040, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070265088, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070408960, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070090752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069720832, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069732096, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069551360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069918720, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070191360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070158848, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070122752, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070580224, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070781696, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070417408, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070531072, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070389504, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069825792, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069629440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069046272, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069280768, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069147648, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068646400, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069346304, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069756416, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069837312, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070170112, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069980928, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070037760, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070233344, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069605376, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069902336, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070199296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069693184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069118464, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069244928, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069356544, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069119488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069693184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070144256, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069960192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069658624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069965312, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069638912, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069641984, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070002176, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068900864, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068687360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068931072, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067514880, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067521024, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068477440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068570624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067512832, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1064960000, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067884544, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069213184, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069395968, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069594368, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069992192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070038528, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070108928, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070002688, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069958144, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069811200, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069584896, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069938176, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069711616, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069432320, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069238272, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069257728, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069402624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068826112, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069251072, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069172736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068651520, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068995584, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069265408, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069501440, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069696512, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069581824, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069699584, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069679872, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069013504, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069001728, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068922880, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068977664, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068808192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069190144, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069620992, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068887552, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068644352, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068500992, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068862976, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069814528, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069922048, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069605632, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069465088, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069641216, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069180416, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069048832, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069311488, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069238784, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069485568, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069642240, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069849600, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069752064, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069786624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069619712, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069068288, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068790272, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068559360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069029376, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068535296, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1067930624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068633088, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068800000, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1068822016, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069086720, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069060096, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069384192, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069488128, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1069722624, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070550016, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070310144, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070177792, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070396928, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070491392, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070561024, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070287360, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070266880, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070125056, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070055936, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070097664, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070388736, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070343680, "_x_x_x_x_bach_float64_x_x_x_x_", 0, 1070089728, "]" ],
									"reg_data_count" : [ 2 ],
									"saved_object_attributes" : 									{
										"embed" : 1,
										"versionnumber" : 80001
									}
,
									"text" : "bach.reg @embed 1"
								}

							}
, 							{
								"box" : 								{
									"attr" : "chanoffset",
									"id" : "obj-43",
									"maxclass" : "attrui",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 477.0, 312.0, 150.0, 23.0 ]
								}

							}
, 							{
								"box" : 								{
									"attr" : "chanoffset",
									"id" : "obj-42",
									"maxclass" : "attrui",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 477.0, 364.0, 150.0, 23.0 ]
								}

							}
, 							{
								"box" : 								{
									"buffername" : "u319000452",
									"id" : "obj-37",
									"maxclass" : "waveform~",
									"numinlets" : 5,
									"numoutlets" : 6,
									"outlettype" : [ "float", "float", "float", "float", "list", "" ],
									"patching_rect" : [ 292.0, 360.0, 182.5, 39.0 ]
								}

							}
, 							{
								"box" : 								{
									"bubble" : 1,
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-13",
									"linecount" : 3,
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 458.0, 183.5, 160.0, 54.0 ],
									"text" : "A multichannel buffer needs a parenthesis level for each channel"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-36",
									"maxclass" : "newobj",
									"numinlets" : 2,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 292.0, 199.0, 164.0, 23.0 ],
									"saved_object_attributes" : 									{
										"versionnumber" : 80001
									}
,
									"text" : "bach.join 2 @inwrap 1"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-35",
									"maxclass" : "multislider",
									"numinlets" : 1,
									"numoutlets" : 2,
									"outlettype" : [ "", "" ],
									"parameter_enable" : 0,
									"patching_rect" : [ 437.0, 125.0, 115.0, 57.0 ],
									"size" : 882
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-34",
									"maxclass" : "multislider",
									"numinlets" : 1,
									"numoutlets" : 2,
									"outlettype" : [ "", "" ],
									"parameter_enable" : 0,
									"patching_rect" : [ 292.0, 125.0, 115.0, 57.0 ],
									"size" : 882
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-31",
									"maxclass" : "message",
									"numinlets" : 2,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 292.0, 280.0, 46.0, 23.0 ],
									"text" : "set $1"
								}

							}
, 							{
								"box" : 								{
									"buffername" : "u319000452",
									"id" : "obj-28",
									"maxclass" : "waveform~",
									"numinlets" : 5,
									"numoutlets" : 6,
									"outlettype" : [ "float", "float", "float", "float", "list", "" ],
									"patching_rect" : [ 292.0, 312.0, 182.5, 39.0 ]
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-26",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 292.0, 246.0, 109.0, 23.0 ],
									"text" : "ears.fromsamps~"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-25",
									"maxclass" : "multislider",
									"numinlets" : 1,
									"numoutlets" : 2,
									"outlettype" : [ "", "" ],
									"parameter_enable" : 0,
									"patching_rect" : [ 14.0, 131.0, 129.0, 62.0 ],
									"size" : 882
								}

							}
, 							{
								"box" : 								{
									"fontface" : 0,
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"hyperlinkcolor" : [ 0.741176, 0.356863, 0.047059, 1.0 ],
									"id" : "obj-29",
									"linecount" : 2,
									"linkstart" : [ "h" ],
									"maxclass" : "bach.hypercomment",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 10.0, 416.0, 377.0, 36.0 ],
									"text" : "Click here to learn about the common features of ears modules, including in-place operations and dynamic allocation."
								}

							}
, 							{
								"box" : 								{
									"hidden" : 1,
									"id" : "obj-27",
									"maxclass" : "message",
									"numinlets" : 2,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 14.0, 453.0, 152.0, 23.0 ],
									"text" : "load ears.help.commons"
								}

							}
, 							{
								"box" : 								{
									"hidden" : 1,
									"id" : "obj-17",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 14.0, 488.0, 57.0, 23.0 ],
									"text" : "pcontrol"
								}

							}
, 							{
								"box" : 								{
									"buffername" : "earsFromSampsHelp",
									"id" : "obj-23",
									"maxclass" : "waveform~",
									"numinlets" : 5,
									"numoutlets" : 6,
									"outlettype" : [ "float", "float", "float", "float", "list", "" ],
									"patching_rect" : [ 449.0, 10.0, 182.5, 39.0 ]
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-1",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 2,
									"outlettype" : [ "float", "bang" ],
									"patching_rect" : [ 449.0, 55.5, 178.0, 23.0 ],
									"text" : "buffer~ earsFromSampsHelp"
								}

							}
, 							{
								"box" : 								{
									"fontname" : "Arial",
									"fontsize" : 13.0,
									"id" : "obj-14",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 14.0, 293.0, 236.0, 23.0 ],
									"text" : "ears.fromsamps~ earsFromSampsHelp"
								}

							}
, 							{
								"box" : 								{
									"border" : 0,
									"filename" : "helpdetails.js",
									"id" : "obj-2",
									"ignoreclick" : 1,
									"jsarguments" : [ "ears.fromsamps~", 90 ],
									"maxclass" : "jsui",
									"numinlets" : 1,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"parameter_enable" : 0,
									"patching_rect" : [ 10.0, 10.0, 485.25, 100.0 ]
								}

							}
, 							{
								"box" : 								{
									"background" : 1,
									"bgcolor" : [ 1.0, 0.788235, 0.470588, 1.0 ],
									"fontface" : 1,
									"hint" : "",
									"id" : "obj-20",
									"ignoreclick" : 1,
									"legacytextcolor" : 1,
									"maxclass" : "textbutton",
									"numinlets" : 1,
									"numoutlets" : 3,
									"outlettype" : [ "", "", "int" ],
									"parameter_enable" : 0,
									"patching_rect" : [ 554.0, 125.0, 20.0, 20.0 ],
									"rounded" : 60.0,
									"text" : "1",
									"textcolor" : [ 0.34902, 0.34902, 0.34902, 1.0 ]
								}

							}
, 							{
								"box" : 								{
									"background" : 1,
									"bgcolor" : [ 1.0, 0.788235, 0.470588, 1.0 ],
									"fontface" : 1,
									"hint" : "",
									"id" : "obj-15",
									"ignoreclick" : 1,
									"legacytextcolor" : 1,
									"maxclass" : "textbutton",
									"numinlets" : 1,
									"numoutlets" : 3,
									"outlettype" : [ "", "", "int" ],
									"parameter_enable" : 0,
									"patching_rect" : [ 409.0, 125.0, 20.0, 20.0 ],
									"rounded" : 60.0,
									"text" : "2",
									"textcolor" : [ 0.34902, 0.34902, 0.34902, 1.0 ]
								}

							}
 ],
						"lines" : [ 							{
								"patchline" : 								{
									"destination" : [ "obj-14", 0 ],
									"source" : [ "obj-25", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-31", 0 ],
									"source" : [ "obj-26", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-17", 0 ],
									"hidden" : 1,
									"source" : [ "obj-27", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-27", 0 ],
									"hidden" : 1,
									"source" : [ "obj-29", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-28", 0 ],
									"order" : 1,
									"source" : [ "obj-31", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-37", 0 ],
									"order" : 0,
									"source" : [ "obj-31", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-36", 0 ],
									"source" : [ "obj-34", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-36", 1 ],
									"source" : [ "obj-35", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-26", 0 ],
									"source" : [ "obj-36", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-37", 0 ],
									"source" : [ "obj-42", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-28", 0 ],
									"source" : [ "obj-43", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-6", 0 ],
									"source" : [ "obj-45", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-14", 0 ],
									"source" : [ "obj-6", 0 ]
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
					"jsarguments" : [ "ears.fromsamps~" ],
					"maxclass" : "jsui",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 10.0, 10.0, 363.440032958984375, 57.599853515625 ]
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
							"minor" : 0,
							"revision" : 5,
							"architecture" : "x64",
							"modernui" : 1
						}
,
						"classnamespace" : "box",
						"rect" : [ 0.0, 26.0, 693.0, 460.0 ],
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
				"name" : "helpname.js",
				"bootpath" : "C74:/help/resources",
				"type" : "TEXT",
				"implicit" : 1
			}
, 			{
				"name" : "helpdetails.js",
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
, 			{
				"name" : "ears.fromsamps~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.hypercomment.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.join.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "bach.reg.mxo",
				"type" : "iLaX"
			}
 ],
		"autosave" : 0
	}

}
