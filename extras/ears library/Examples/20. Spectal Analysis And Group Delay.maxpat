{
	"patcher" : 	{
		"fileversion" : 1,
		"appversion" : 		{
			"major" : 8,
			"minor" : 3,
			"revision" : 1,
			"architecture" : "x64",
			"modernui" : 1
		}
,
		"classnamespace" : "box",
		"rect" : [ 65.0, 108.0, 1102.0, 671.0 ],
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
		"boxes" : [ 			{
				"box" : 				{
					"id" : "obj-17",
					"linecount" : 2,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 722.0, 626.549958621754286, 150.0, 33.0 ],
					"text" : "this object is called ears.specshow~"
				}

			}
, 			{
				"box" : 				{
					"bubble" : 1,
					"bubblepoint" : 0.0,
					"fontname" : "Arial",
					"fontsize" : 13.0,
					"id" : "obj-12",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 701.0, 600.549958621754399, 169.0, 25.0 ],
					"text" : "Display in a spectrogram"
				}

			}
, 			{
				"box" : 				{
					"bubble" : 1,
					"bubblepoint" : 0.0,
					"bubbleside" : 0,
					"fontname" : "Arial",
					"fontsize" : 13.0,
					"id" : "obj-10",
					"linecount" : 3,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 752.0, 504.549958621754399, 200.0, 69.0 ],
					"text" : "Apply our formula (notice the quick-and-dirty +0.001 to avoid divisions by zero)"
				}

			}
, 			{
				"box" : 				{
					"bubble" : 1,
					"bubbleside" : 3,
					"fontname" : "Arial",
					"fontsize" : 13.0,
					"id" : "obj-8",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 398.0, 59.961723327636719, 165.0, 25.0 ],
					"text" : "Center first frame on 0"
				}

			}
, 			{
				"box" : 				{
					"bubble" : 1,
					"bubblepoint" : 0.21,
					"bubbleside" : 3,
					"fontname" : "Arial",
					"fontsize" : 13.0,
					"id" : "obj-69",
					"linecount" : 3,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 357.655774691412716, 418.549958621754399, 233.0, 54.0 ],
					"text" : "Finally, format them so that they have the same spectral properties as the left computation"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-67",
					"linecount" : 6,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 460.0, 176.459854014598534, 103.0, 87.0 ],
					"text" : "On the other side, we manually compute the STFT for the ramped windows"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-65",
					"linecount" : 3,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 311.0, 216.459854014598534, 103.0, 47.0 ],
					"text" : "On one side, we compute the traditional STFT"
				}

			}
, 			{
				"box" : 				{
					"bubble" : 1,
					"bubblepoint" : 0.21,
					"bubbleside" : 3,
					"fontname" : "Arial",
					"fontsize" : 13.0,
					"id" : "obj-61",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 508.0, 377.690446731892052, 185.0, 25.0 ],
					"text" : "...and collect them all!"
				}

			}
, 			{
				"box" : 				{
					"bubble" : 1,
					"bubblepoint" : 0.21,
					"bubbleside" : 3,
					"fontname" : "Arial",
					"fontsize" : 13.0,
					"id" : "obj-60",
					"linecount" : 2,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 554.0, 334.549958621754399, 186.0, 40.0 ],
					"text" : "...place spectrum vertically: channels are now bins..."
				}

			}
, 			{
				"box" : 				{
					"autofit" : 1,
					"data" : [ 1391, "png", "IBkSG0fBZn....PCIgDQRA...PD...fOHX....fjp1aE....DLmPIQEBHf.B7g.YHB..EXRRDEDU3wY6b1FhUUDFG+2tq2z8krsjJE0VHTzMs.QBKirSuPRAaID9gfr9XJ8kHHvnBK5SQ0WKnBxfhJzdAquXV1lIaQnlxVXaj8BIlkus1Ztt4c2sO7bN3rWmYtm6Yl4bt6s9CCG14Lyy+Gdtyybl44YlsIDrDf0.LUfx.i.LLvYTJmMtsk.t.foE29jmk.lBvP.aHtOtfd.VIv3wxZDMOaAnUf1he1Zr97w.efKju0Xh8U4tbQYhwIbf+imURaN94Iyp.zfw.9IOHmS3PeOXV6XSwO6.nafoCbg.yDY35ppR+GG3SAdeDivIA9YfCmUERAyD3pAVAvif3VXBeKvt.1CvdA9ZDWJuim.6CMeuPPpF7bF3+i.tgbRG.jeUr4KuybRO1VE7dDfaOm397vKg8QIyKv7u5J3a2.yIvbZEWG1MHOS.4dA.CpvUe.WT.4K0X+X1f7KbtIm8ItTfeTgmcfL4ecAVO1GkD4Y9ZE3KYhtI0MFC.lMvnX1f7ZdjqlA1rhr+UfY4Q46MXa0rCAztm34ETj6f.KxSx063dwtayZ7.GOjh79GfawCxLXnUl3L9UV1lixuGjMVlHu62Q4kK3kwrAYTf4lQ4tTfSoHqM3rllSX4X2s4wxfL6B32UjwF8ghlm3GvrA46qQY0Iv2oz+OAItJSpvii8QIKKkxoDxNkS5W+TmrJzZEyE6qI4ESobdck9bHfqv6ZZNhJ28oZ4XHgXzFdJk1ODRHLmTi6C6tM2ik99.JsqLvcFTMMmPa.+ElMHawP+tUjEbkzt0FbMMGwqhYCxYAtrJZ+hXhKr6YyMMMmvMhc2lGVosyBYSZIu6cHLgLnvgZ7Jprrm31zNx12SpemH4wogDOI1GkrDfOT4uG.XFEhllR35v1tPR6PZjyQ.tdfC3Hm1vq.baHY16uiKCi70rQQxYTKHYXrDxGG5.YT7P3o.cscrOJYbfSiDa1PiimBcwV4JmhGThMBbyVd+XHwJ4q7.WUCqC3pPxu6L.tVfqwPaSRr0oPVBw.3mLNRGHYIyjU+Q8AIYDK0fN8zgjzlv7hz5ih8yqOnFcZLfK2TGZ1zKpAzMR9f0g2JVIJJDoot9A9CScvGFjUX4ca2Cx2EDootdCMoIiBpr7mTrtKKvfdUsSzfy3PFHdygl3p.cyeLJvkXqSt5xLeLmDodcT1thHM00OU4zE4pAQGoI3ycT1thHM08Yglz2.8tKGkhc9iEZPut6PS7AMPbdcxhLg0h94Ot3p0QWbYlGRRv0gdcPt9.QZpaejhCxmKFDcjlfhd9iaRSc8FZReSz6tbLpOm+vGmcVqPM8ipEmNAwd.ll+nyzz4r5xrXjyQpNriLJSegHM0sWj.bWUjUCxJs7tuHixzWH2m+XpHq3Sm6xvTrIq1z7G8DBxZAIlnax.oIkkG21h.5l+nLdHI5qB4nIzORn09Mj3hl1XSVFY2tCf78+9PxiSnwaqQW1kOD76pQvtVVuOTrpfCqg2muVDfofLuONWbCRtPQpWbmQPxQaYjIlKwDuLQIkjKUzQI7KLZgnOzfAeCc0qvz7GSuVDhOBgX8BhzT22fD.7TiFcCx+YcW5F8Sj2PbXbxBVG5m+vT5QLhFEWlHM0sajDXWSnQvfzLdL+KMBFj6fy+3aAY79.NY6XMsLjTezIRR1WLxBH0c0T1JxNuGDYSm6G4RI0vf1PRTcV25vHjhakkONeH4EFC4f60Exob7LH+xe53mI+uIH4DBMMjQNsib0VN.xIJ5+Qsf+EA7HelwbLniB.....IUjSD4pPfIH" ],
					"embed" : 1,
					"forceaspect" : 1,
					"id" : "obj-51",
					"maxclass" : "fpic",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "jit_matrix" ],
					"patching_rect" : [ 908.07512953012224, 479.961723327636719, 17.612903225806519, 16.058823529411825 ],
					"pic" : "GrDYI.png"
				}

			}
, 			{
				"box" : 				{
					"bubble" : 1,
					"bubblepoint" : 0.21,
					"bubbleside" : 3,
					"fontname" : "Arial",
					"fontsize" : 13.0,
					"id" : "obj-50",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 539.0, 304.549958621754399, 204.0, 25.0 ],
					"text" : "...compute Fourier Transform..."
				}

			}
, 			{
				"box" : 				{
					"bubble" : 1,
					"bubblepoint" : 0.21,
					"bubbleside" : 3,
					"fontname" : "Arial",
					"fontsize" : 13.0,
					"id" : "obj-49",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 559.0, 274.549958621754399, 175.0, 25.0 ],
					"text" : "...apply analysis window..."
				}

			}
, 			{
				"box" : 				{
					"autofit" : 1,
					"data" : [ 2664, "png", "IBkSG0fBZn....PCIgDQRA....I...fRHX.....Ui2c7....DLmPIQEBHf.B7g.YHB..J7QRDEDU3wY6c1GjVUUF.+2BKKKKpHryFDIp3G.CSeXnVlRDoTni8wTFCYwPZhyTAkM8gZJMSX1GNSyz1zPkE0pSSgIZgTZaTnqFMlBohCeHYx.RHtoPQBrvtzd6Oddem26649be26489w68ceO+l4NruWtOmmm68dt2yy4bdNOWvgiXPSoXY2AvTANSfwArcf+RJpOGkXt.mCvQ.9W.uBvNANdMzlFRNefUArMfCC3Yrs+Zmo0Pwj.NAAu9eLfMB7sAlWMy5LnEfEA73DzfKtMHvdQLbGoOs.b+.6B4ZeX2W1DvUVirQ.3R.dQzMtc.rBj2J0Zsx.cvo.LafaG3ei98pm.3Mm0F10.LfhwrefECLhr1fbLjbJ.2BP+D791A.dqYkgLezq7rUfojUFgiplqDnOzqDcNosxaAweFSkebfom1J2QhwUfdyY2aZq3kDhh+Voshcj37GQuCOmaZpz6RQoGCXrooRcjJbYn+xfEaSgXqytZMS8bHCXki5K1.heOlLSaJDaq.MZk8sMKKCG4G1mx9ZylBHI5t8gSfxvQsg+mx9ZwlBvMdMNLwp4G0UAxQrvUAxQrvUAxQrvUAxQrvUAxQrvUAxQrvUAxQrvUAxQrvUAxQrn4ZsATkzLxJ93rQF4zWDYRc0FZdMZEXVHqVj8ArGfCk7loCSdJBN8+2YFp+2EvCgdXY9Z.qDohUXr.fmTQ9AQhOlqh52GppFx76m0pJPc.rdEcO.AW4A8A7ALjeb.q133NAvAUJy0BLpT8rI+PCQEnymxCi1CB7k.lFhObiGYICYV4XgEjez.Opu+uMB7toTnoLcBVI76jxmS4Ex76maIiU3L.dUJ0LSm.SHji8NMrqWA4MO+be6acTdGGFOvCSvyomOgOOxqj4Uf1QFpvI.raJ8FkOwPb7cnXa9s2mF3jLjYkJxTbq8D3bHuylI348pRSEdHEEdGojt9g9zwmOhxzK5UF5ifK4nQ.7Rgb78idzWNbCs299qSKkc5JJyC3yjB5ZVHcI2C39rPt+F513OV4X0diUwsMUsFdcFqgfm6uDoTOQ+pJJyC37RX8zDkVq8GE3zrP1cEhMFVfhuGkicPf2S0X30gbCne8ZgURnpg1QbJMKdR8p7U9eCKjqEjE3noMtwJHyES4mW6sf9aT3MfrhZLul8jjfyRwIiTQwTIm.IOzjz767U9S1B4t.zeZ5lGB4FAxPBLUR27kTdkaD8qa2MIPknohjTnzTvME2BODJ5n95rTtOM514aOQstgezD5KXTuB6upVznsiTAQKQQ0Gv0EOath7BHNPOeKkqKBZquFMVSMQ0Ry.eezymP8B7EAFSkJf4frt2+l.ci9bM0OxzAj14Rllo5xoPOOAs4tSP6pQf4idxyvC3+hLh9chjdelQQgdcnmtV7u8CPRcZ4Ud8na2ekZoQUmRy.uejNeTo5D6sn.sBzC5CRXwsAPljwKKaNGrlOJ5186nVZT0obAH99nkCg7PbunWTFaslPhulOBvOB8tD6U3+KukMNLmGLOD+2ZTlU8jfIi9HS6gLkRecfKjgvWH+LEjJKEGQX+aaGoou7B+cBZiqulZQ0Wbw.uLAuFdLfkSLeP7ZQ267MS93MQmF5O0bq0RipNhof9fDeHjlyRDtdEE3Q9HtYVL511kTKMp5HdDzGpl4jzJ5upnnAn12T1cSP65HXYZJoAk2B5O7E4HrvlAYqSjwAxT94A7KsnbRZz5Y3iiLtUQkoBrTj4.JpIZx1.9THizcq.OSA4+8HM4qw4hL8OSqveOJjHr7YKH2VsvlSB9XJ6a.fuaZnrlQuqcqHMTVDYFJ1iGhie1PWEj6Eh3wOAzcb2CI0wMQiieVHYOdsNj3eaMDLn2RSteEaHUqDucEE1YZpvgfkpXOd.uSKJiSlRSYyeHBG+HoT.9eXj3w96Yn++AklHxafxGn1dJHyR.tMBNxu8P1MotOAAu1YS7WYMOshBSxPZsMfOGRShqFoIhJMivcqXOGG6lJjqymrWeDN9EV3XGjxW8GlUDtbJOjY2DxxRxjoQv4b7CZg8GGzhI5txZElTUflL5wbcXsGG1WklcXgNaAoYKOjOSCQo4ihQmvCZr+dTrkha2CU1o90Zb7oZbI6Cs6mZQuYnjmVZyqDeSPmOVPHG+UizbhI6wBctLfypveeiLzILzYgLnafzrkeL86oHqFwY0J4TeuF+9rTOprAuzrvSq2.MVB2AyWU43aiRu4vb6AhnNeSTJZ79yQTlNKb7+GJuGrsg9DR+LDszl6CZH2lin8DWh88y7xaflAgaKOjx9tcB+ozn7z6DQV8AsgTwcYQPFnTrRudjlOKxEQvgD43Hu44nQnbOaie+Oin8TyIuTAJrulg8C7SM12UfzyFPljuOrw++ajJGEhSCwOlheYZVFxBlLJTbhDMiVRsQs8NP505PQ6D76Swthn8T2QZ5DsoCzGgfcG+KSIGmOJk9FWcOFx9rHALteZGwgb+QYfsig0DQ+iQROF5uOjUEaTnXu57usHKsqpkgUqM9Kmxu4N.RWJ+B.+DJef6N.kGxqSBoRie6pWfeFRTVdeH9s3+F70lP1cqHyZc03GFHug0urCR3NjmzLrpBDHUh1phN7usIfyPQ1SBo6uZgiq+aNO.xb.kTLGE8rzHJ6HQ9pJa53cVQruel2B37tKrMODGVOSjv03XH85ZcHyYjVhj5vHit6JPVaWSGwemQgLxv6D32Rz7KwFzF06GNhxdoHqPV+r13YN4ap0IXp7Hl4snW1BYWEAudpMVXoECa5Fe8JihRCrXQ5IhxNFBNHoaAIU84mYiLEK4xj8PdqIr5MtPBFUlOZDk8Cg7kT1O2kwuGOxD71FhuR13bdlf6MPwi4pruGKhxdMF+tefegw9VDkFI6nFpIYJtJPwi4Z76CPzG7Py.gacHwlreVRg+cKj8AaVjvUAp5Qy+mGinMYjEyui9wLpNW.kVEv2l0VWFgsUfNgx9ZDynE.71Hn+OORDkUKmT+T996Igjg1.4sO+F6LsHSru2YaEHsPRvpORqCiPKVr6IhxtcBFBGE6PyLKTNsiDuzWMoWHVb5oT4FJZwPqMAv0vILm+s8gcOQeyFxuSjHO3nTZt.unDzdMYxD7doGobykKODk9dSSklS4Vo7qAe1pnLVN54jfmizI4c4mkonWOj7LPpwLQOvuh5P2ObhQhzM6ag38lhlQbF+lPhG7KE8HsLIoCzyR+GjfiMUhSWJJ1C3qk1J1QhPSHi2j18vgJc.lHbpnmyD8PBhpF0dkUOvXQFt.s6cqgL7d2ohrJL0LjsA7wI8eMriny3PVhT6G86YcQ0kU3hEiAI1jCKOBsWfeEhyluOjrHlirgyC3Shjlj6lvuGsMjgInlxTP552toxAB1tqMlWCGsS3YWLOjwx6dIg5kWR2l2YfD9AyFIn06nv1nA9SjxcQzAfzTzFPhqnCh7fq+sMfcwrTE4+CrUkRJCyK03S.....jTQNQjqBAlf" ],
					"embed" : 1,
					"forceaspect" : 1,
					"id" : "obj-47",
					"maxclass" : "fpic",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "jit_matrix" ],
					"patching_rect" : [ 861.149235894551111, 236.543187347931848, 34.864864864864899, 17.916666666666682 ],
					"pic" : "GrDXn.png"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-46",
					"linecount" : 4,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 833.0, 180.0, 110.0, 60.0 ],
					"text" : "this portion creates\n\n\nfrom "
				}

			}
, 			{
				"box" : 				{
					"autofit" : 1,
					"data" : [ 3438, "png", "IBkSG0fBZn....PCIgDQRA...vK...fRHX....P.B5yq....DLmPIQEBHf.B7g.YHB..MTRRDEDU3wY6ctGsUTUF.+GvkKWu9.g6hGARJ9.IVUpnlluBQxGgOVpYlIYVpkkT1CekhqUZpkqxjpkkZVnqJ07AhTXHk5UiVFORTWBnYPnHhnxqDtWAtvze7clNyYOey4L64Lybtm6Y+asl05NyY1ee6y49M6Yu+1ee6M3vQCD8JCk8f.FIvdAzefk.72yP84n9jwAru.aF3sAdGfWAXK0v5Tr3fAtKfECrI.OiiUW6pZN5lxPA5hv1JuOvbA9g.SnlU6TnYfIA7rDtR6erCfUhT4c3HHMC7v.KGwNIJanE.LwZTc7+yQB75nWAWJv0gzpeK0pJni5J1MfiB3F.VO51UyC3iVKpbmOv1TpPqF37.5csnR4nGC6FvUCrUBaisVfCJOqLm.5F6uDvHxyJhid7LQfNQ2neeyiJPyH8G2rBrEf8OOp.NZ33jPu6MOPdn7KLBk+CxCk6ngk+B5NCY+xZEe2JJ98A14rVwNZn43Pug1yyVAY6fK051xKiLoANbjU7DH8a2jwXqfr0fueJWaw1pTGNR.qR4ZsZqPRC2GtoTPFNbTI1tx0Z1Vg37Wti5YrNVvbF7NZnvYv6ngBmAuiFJbF7NZnvYv6ngBmAuiFJbF7NZnvYv6ngBmAuiFJZpVWAhAiDIHgVCRpCZafp0JxJmvfA1EjrxZ4HoQVdPSHeG1GjYF70QB3NsoJWiV.FKxJ+vp.dMfMl9USGZ7bDNDMuiLRWmKvaQ3Xf9ugrzNTNZBI18aGI4TzBsz0A7yQdXHK3S.7XnmlZuGvsg7fPTbV.yWo76.I9vOSpOZvJsHOs8xckdCJ5w73qGQYOCjVPMu+sgzpp406B4gizhAALmHzuYl42IvoZT99CbeJ0w0oHyY.z2Trt2cldrF7mb.YuhBm2LvUon6fKcC6JviG3y1dgy+7.CGY7J6DxaGdUC4rCfKHEp6GLklBjqC3x.FUA8O.jkqDSi4ytP46GvSG3ylKvwRwvxd+I7CM+3TndWOPMwf+ExXk1Of+SA41AvAF3y5CvaZn6kfXHMXf+Yfq+FT9t8LbfMXHq0ijs7IkQC7tT7AnoBLvHt26vP2uCRK6+1.WalTpSEF.vSR3e+e0pnNWOQMwfeoYrROm.x8JT97mRQ+iGYoYy+7oSzFZA4tTjklNiCCD4sQ9sX+Epv8OHEcG721EgL.6fbaJkw+nsDVuqmXgD968ck0JciJJ8lSQ462BVGHsnYhVx7F73JsPWSRo7+wDVu+kAjw2LlkYMJ52ue8lK2I8lvucy+XqnmIZ8zP6saSOKU3GTQgd.esTR9Ckh8OMpmbWVD0AOf6zR8MQEYjjzUbrTbvvOjEkKXWvpz2Cs2H3erfDTmqG4AI7282jLzSUWqhB8nz9YWMbQAj4mT4yaKB86g3pRa8VwmQQNavRYzKJttZ1AvdXQYWth98H5DS90Tt2cf9uU8D4RQ+2qytbEJozFx.qxxVWdDJZ3n8J5SUQ+dHYy9PRf9tbEY8ZVJiyLPYuQKJWynO+.ysLk4Hnz+GrxB5uQggiLoil+lMeR4HFXWQLrMUTWT4I.JtzaJt.Z9XQbO2hRcvC3KmPc96Uj0SZoL9ST72hgYQ4NDEc6g350xQuQbw4HIaWa+6txUf9ua2CojQ+HQ1DCzThMCPrRLl.x8aDw8n0m2kg3txjfou3K2XGhB+AwOSKK2WUQ2d.GlkxoQidg9hAlWgqm3EDr1PLn01XC5jzYRZBxHPVAy1HxfiMY2QeVRu0Dpu8RQVI4g3kUndcBVVtoon62iFqPEHozDRHgnsdxuFfuCxDLVVNFjoW+l.lM5w+wVQlx6rZ84teD8.Oip+6mbB00WIB4c3VJmlHYq+8ZucY1IPNMxbBnuv95A7eQlw5ohrztO5fEbvnu7WG73Wf31vZE+Dk5TWj7YFc5Jxa8j7tGYCe.Ec6A7cyAc2Sil.NEjA6WN62UFrPsfDUgZSpj+w1PBToiK6+Nnh1zJOuDJqlHbXE3gc9PuZ3ypnaOfOdNo+dRbHH8cWaMj2Co6lqgHlildgDy1eZfamnCq1am7c0Bd.n2+8jtmQMdEY4Qx81isXFGMdHiUpQIpGSCFF5y7pGRHd78ANThQe4CxHPLt0L1VBRWgxCNME86g8CTzmaWQVaG6bqX0v+RQ+yImzcOANBBmmDdHN8XJjBMb7EQeDwKj7ok9aUQ2aMg5tIzm.smIUpoUl8PQ2d.WSNo+5cFA5++aiHcuI0H3z9G7HOhE6Eon2jtAGe7JxxCXxUe0LVbdQn+iLmze8NOEg+sqSDuLl57OTT11Ha6Zy.QuKU2TBk2uVQVam7yCT2ih92LIXYetAjC.8FKrJZcsYhNlJhe3MK+D.tWaTpEbLnOkwOcBjUKHCHWSVuUDk4agDmNWKxC2UKZd45YQ5hVbYj.WBRLjD2M1qVAtXjYxsEfmuP4+yHcWUi8CIzQFUg+tuHYv0KVnbujE04zfOmx01FIexGqHMgt6ettrRgHOjYputHbxQDGNaEY4A7kh39On.2SZ7JyQGg9mhkxYZEJ2xh48OPzGnrGxVIiYf2MVjcGas2rF73AIY+eHo7vJ0gL+gtknnzolg564Uz27SnrdLEY0IQO4U9OrsCRVzXZxknneOfi1BYrqTLbOd7Xb+8ghIT9lPbk6O0P++aJ9VzKkRmDx1KTlKD35I7La1N4WPrMOB+aWlO2IZCfLqxqv1P26P+nDHqAf9rIGUWBZhhYjzBi3dZEIP2tWjt5cwT9H1a1J5eKXWnIbAAJ6EEi62+sZ6fRWcDLMbOQJMEBW.xxLhIihvwW0oYQ8uZPaxGmVsPoYkA+oqnKOjoR1VN2Hjk4RjgOep.2iVJ6MLzyu2n5O4PQ5Jl48uTK9NzLEy3qUS75NgejtNKiq2tRcw+39o7ChdFF2elmWoEPy1y1rbqa8Rs23Ttl+BwjsbPJWasHC9RCei7NA9cJe9sgQ.IUfyJB4cNnGmN1jvISFXuK72WAUdyjarHSRCHciIHQ0Es6CYvgkaPzqw378V8txG7xZEjmsvqsjfrnDJKM2QFkmkBtt3XZn.xDdE0.5dWk6uUhNWbezXV++HTLaeh6C79iAYCTp23ZE8t287DusAxYYTtn5xWZSpX60csE9Af7OYSRh6HAwcZln0ch9hjYUfzBplOdGMQ+6lV1ZcCDcqfwo0wgfDcmsh7fVbmjL+bccNHcmxmCmvtidKHsr2QLj69Xb9aDy5S2B5tZveXnO5+mJgx6kUt1aabdeQZ0eTEN+pPxJdSVcD5XqHuIIHmDhmO.InlNCiO+CS4yxoQgzO78sv4SF4MewA+.mxLarzbw5Mi3AtJQaH9jOHKOl0m5Rxqtz78TzSWHY9TRn+D16BAcm5NSoupdVTd2sYNf0MSX2Kd4TbfpcPwwQb+Fk8EQRP4fzFx.fCFwp1NeGCgvFmP3Ar1IxuOwAs4xXRVVuRJ8XWaIA87VLowOiOmBkZ.0Ahw2rJ729WucpbXkdhFxZaHtH6aC7qnzI5YsTZjcNTDi7fe2VCvuAIjIdHJMd86DI.9RCZAIpBSx3HfviEJslih3POZC9KyPGcP5jZgSjv+COnQ6sP7iBySDYl9zjk+wB.1SkxtKHtySKUJCZL8nHwPRZwwnnmKIlksOHcCzbft4EohsW20jF9mg7Z8iFouu2IRqhUKyB3Cgzh4Afz5zqi38mGk30OVelcgiIfL.w8BI7eeeDuxLSD2dpswGrIjYu75PVaY1ej9q2WjY97UPV1+ro9DGzlU23tzjLdjU.sfLipq5z8mZxqUbjZXtt0GUPyog1hOq1bQjUzi1sjNRe5KEmHJeZOlkcmH7jp8BD16WGERHOzscwcs6ZWZbj9bnDd7IwcdMNcBGjc2sw4C.If1ZEou91LX3bCWK7MNLNkqE2Ta77MNeqHKUgAYRTblZianKm63L3abXbFmuVh+jMYl3JyDI2RCh+9j0KP9mbHwFmAeiAZ8e+YPF3Wkve+oJHlwgzYQQ2Fe8VW6xQr0fuKkq0HtJ1VuwGiv8eOtgog1dB6yE3uGJxNfBHst+H1U0hMohclsF7ZgMZbhvNG0Vzxk11iYYWBgCIXemcLlBxoMj.zyeO5JKPaQ1MyQKuBsIIFbTavL9cVE10ho4VF5qfDYn9gjwlw9EhVaXXD1tyibn6SSIBEe7YshcTUbMT5+uhZSctbLEzW+QeYRuMFinXxJ50CYc5LSYLnm7C1tyY3HeoOHtM7po5ZItIjA+dkH4y63I6WwkGD56B4qipae0M1LMEk6gDRuNbjlzKz2Zh7nxaOPoF6N564SdHIRfyqMNRC1YD2epYm8fjy1Y6Nx5CiVkYwHqR.4wlKfiddzejk7jUit80zHY65JUM6DR9ZF05H+JA9CHCX5jQ14Kb3vjCDY0e6FQB25nrmVLhaOq4LBD2CsBJexPrhZS0yQ2XZin28N7Pl2mGfTzKPoc+f1SjPD8nPR73AU3ne.+UxA2H4nthVPVeKGMhWWVgwwSfcwreE4+ATKB5LRZDP6X.....jTQNQjqBAlf" ],
					"embed" : 1,
					"forceaspect" : 1,
					"id" : "obj-43",
					"maxclass" : "fpic",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "jit_matrix" ],
					"patching_rect" : [ 861.149235894551111, 200.459854014598534, 49.432432432432506, 19.457446808510667 ],
					"pic" : "/Users/danieleghisi/Downloads/GrDnXn.png"
				}

			}
, 			{
				"box" : 				{
					"bubble" : 1,
					"bubblepoint" : 0.21,
					"bubbleside" : 3,
					"fontname" : "Arial",
					"fontsize" : 13.0,
					"id" : "obj-42",
					"linecount" : 3,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 573.0, 200.459854014598534, 161.0, 54.0 ],
					"text" : "...multiply each sample by its index to create the ramp"
				}

			}
, 			{
				"box" : 				{
					"bubble" : 1,
					"bubblepoint" : 0.21,
					"bubbleside" : 3,
					"fontname" : "Arial",
					"fontsize" : 13.0,
					"id" : "obj-41",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 551.5, 157.961723327636719, 190.0, 25.0 ],
					"text" : "For every frame..."
				}

			}
, 			{
				"box" : 				{
					"autofit" : 1,
					"data" : [ 1781, "png", "IBkSG0fBZn....PCIgDQRA...PE...fOHX....PsDvS+....DLmPIQEBHf.B7g.YHB..FvZRDEDU3wY6btGhUTEGG+yt9bWeb0j7QlYlFooRYEUt0ZaEjj0FRQAQoAQQAVQXkYXkQAEY0+DTQQgEj+QYlUVDkkrpqlklknkZYYlhuVe1t3l5U29iey3N6bOmwYNmyb268ZegCC6Ymy2e+lycNO983Lkgfw.LIft.jE3v.MC7uAJG06d6DPmA5p286esS.cDnQfY50FaPs.iGnEOtNrhqc.nBfJ8tVgm970.ehkx2J7UHJtqJ2jCzo8ag72mCjuQnbuqGzgbdbf+zA7reKZ61bf7MBk4cs6.i.nm.8.n+HC2l3Io8s.7s.yGoS7f.aFXmNP25OvnAFGvTQFVqCqCXU.qF3mA9AjoDJ3vSRzCs937jd7RZj+mCbE4IcvInRhdtr5yS5wBCI2F.tt7jrcNdCh9szgkxx+VCIueD3LSYYlp3xI5NzmKEk84Abf.xZ4.YRQ4k2v5QeG5eQqKt4Rb5.aJfbVBxhmkDX5D8ao03X4UAv2QaGlWxzYBv.ANF56PmsCkU4.eT.t2Bv.bH+ELHJqoZDnaNRNuR.dO.vHcDuEb31I5g8SxAxXJA36H.WiC3rfEUPaWwMbYgVxesHNlwmuIaIeEE3sPeG5w.Fjg7dw.MEfqYZslVjfpH5g8OgAbNXfcDfi20EJZwD9Mz2gtgDxUu.9k.s+aP7q5oTXFD8aoWVL4oSHdpxucqkRDqfRJFDQumzWOl77dAZy1ANKmqoEQHr2eBV1KRHRhBOSf6uQjPvbJMtShdX+sDQauq.2WVfIjpZZQBpD3ePeG5moocWKxF18uu6O00zhH71nuC8n.8Mz8ORZqgAuXdSSKRP0D8v9GNv8N.Dmb3++9.RGW9Uzif9qLbY0d2S2Pb+le80iDG++GJvSQzukNFfED3u2HPeZWzz7DrcX2fQBabb3oAfwB7GVJynvDQ1RVurjmlQBN49Pz2UfD9ksXIuwBKhneKsEfCgDapzFuZLzEaJ0i3lRslG6hEFlLhUO5vwAtMf44.YcxvY.b2HaqKCRxab0JtucALWjcc.RTB7yYqNiLxajHVuopOZcdxYkNT2OA5NRVZn6W0GKMDZLQFM5TMwr8cG3NnsdCKnQIWuaUWAkg9M4ubZe2dzDTnSGDIq8RBxfD2LUb47Pzb9JDje4AcsvRHlE4pSeggb0QD2SFluUE7lJO21kXLtH9eKxA7aCpQQc0YHWYQhFaXbQDXqftnC8pzTeC.+pC32TzSjG1vXwVv4NTTWYDnOHM6PWBxPh1Kbkj6bkMhX0loPmQImvw311gdtnOIDpyRtsEp1tT8HNG2Tna5sUoo9Di6E8KHMZWIDCwJIWcZZVvWeP7hVXNO.tYjN.79JDPK.6g12sKkg1Fie+Rbi2kJ7xJ3qEfGwJMMD1lFgjuxrYc3FHWcpQjs9XBFEpMdYw3v2NGlBA3WdHWIDCgpTH+KMjqggDDwv7sBf9YslF.2iBg3Wt.WJHCfp4Oe7DxQO7ZytUv0bPNOTNEyQgfZAIpmEhyeFWuc0WfmE0msfM.byQ0XSmSAzmYbKkBu8eBx1npNTckgb7cNauxfANMEs8m.dSf2A4GKmiQg9g6SMMDXBftifiIkYALzjHbSeCc7Q7+Vpgb5JTih5V.RNTEFkijc1CEX3jqm9uOfemzMJCzEj7PR0unMS6axdoa9y33hsJ.dTj0.B29T4jtzAjXBMWEBLXoJRt+FcETs+ycSxVj7RPNsyg4wprodhHoV3ZQhT4VQhKTbm6Iq2CxFAVChilCufPZ.UyepxkamL77J3Y91nXySAg1VltMJTLgp8e9.FvSsJ3YSwog5VTZMz5IQ1+CRPvC9+gQxQorzZ.tB9wHvu3+QIXOj9deJCpyhOSjqpMsWRmOApfp4Oa.yLx3ETvUrNx5Nyv9B.nx+ml5j6pTT21iSCKk5PqQQc0Y.OcE3RUTerxZjRkNzL.Wnh5MI9QUg54Pi0m8iRkNzpI289tWjs8kTLVM0u0HZSuQLLnjoCsFE0Y57m5rcW2hR0h7i2pgRmNTUVwXZ3hqPS859zEMMjcRbDnznCcDnd9SS+dn72ZpW0WkmIf3tP.9Pn3Ksr6Oxai8F4A7bPb3a374GDq89djnRlEwKXwwZmpQltHLlAhIo9X3.KCw+oaFwALMGmGhBILaL2z2OMAx40Tz9ch30+gfDyL+C6aSDXNba7Xe6A1k20Cg7fzDhYvYQRfgxPdl5HxavcCIkD6.xwfLtXJHVY8zz5n39g7Y6HH1IvMhcYixoTXHHmifkgrw9if3Oi0i3YqAFtA+GgJIFjffC1AK.....IUjSD4pPfIH" ],
					"embed" : 1,
					"forceaspect" : 1,
					"id" : "obj-39",
					"maxclass" : "fpic",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "jit_matrix" ],
					"patching_rect" : [ 584.07512953012224, 460.549958621754399, 21.580645161290334, 15.928571428571438 ],
					"pic" : "GrDYR.png"
				}

			}
, 			{
				"box" : 				{
					"autofit" : 1,
					"data" : [ 1604, "png", "IBkSG0fBZn....PCIgDQRA...XE...fOHX....PrwyNv....DLmPIQEBHf.B7g.YHB..EreRDEDU3wY6btFaTTEE.9qKsXa4gfnhZPwGn.FswGQQ7YMpQsFQSHPB+PSpFAQD+A9feXPi+yWs9HwefjZh+.ESzDMlnflnnBpIRALgffZ8AQCRjTQf9ztc8GmcRuyz6c1Yt67Xm18K4lcyz4dNmd1YN268bOyTC1w4CrBfFA9OfA.5Gnuhe1ewimCnNfIpzNAkuWGPu.OUw95G0.rZf4qnScs7E0Q8ZZ+CP6E+rhjM.THBaKK.57RiHcsjx3+6.SNK62QhPan.PWA375GX3HPW+bYJi.QMV1uIBbw.mHvTAlAvMfb0P8An+6C38.5D3qANT.0aS.WAvxAtxRbtE.1Nv2A7SHNzcC72ATWUTbsHw376VwG.6+A0g5.1iA4eXf0BblkoNp3XSX1oNHxU3QAa2iryCzFxcQiIYg3+UrqNBzwkALjhLODvMGAxshmNwri8Gn7BELSfeUQd+Fv4UFxKSQq3+Us2hkxcZHC.4HmC.b1koslondjAQL4X+.KjYi3Nt5AAlSTXrYMdVL6XyS3tRah.aVo+ciLcuwkLabO.i21yEP4jC3cT52wAtpn1XyZ79X1wdXB1hIVuReF.6iOOlhaB+GDq0Rze0vICAr3XyRyfrWL6X2gO8asdN26OdMyrGqB+upcgZ5yx8bNOQhXoYLlBvQwricidN+kh67M77IlklA4Uwric.jUSAvsgjOAm+VGItklwXtH4R0jycc.WCPOJG6cAlPZXrYM9D7OV6+p78OFYQAUI.rH72w5z9RfFRIaLSRNbmUJSS+JpxW63Jdb72wdqomoks4jP1taSN10mdlV1mNvri8XHy6Myisa+ssLAfywm+9jAtmDxVhUJ2cKMr5pCJchW1CoetV2.xdn0Oxbq6AoRcFBYUgCibQRsH6XbiHWTLIj65ZNIM11HXS2p.v0mjFlF5lfaq5ZmasIjg9j.qo32KfjjkW.YOrzwpPlOaZwJAtPj7EOCjhDoICm6N.1IRB3OJv9A9kDvF4gv8ulObwi2Nl+EePfSOILt.xkid67YRKCZY3NSUpFxbv+7G7zIpk5OqfQaeCyHIOJQoEbmopWSy47QX1w9mHCNTIvaynsuuOMLjqC2KDXSnepcsfYGaAjbyVIvAYz11qjzFwkfTlmNFvVvblpxgTEf9kPlzl4hda6tSRi3BPpiJGk+sHysyOVC9eUqoQiSJzEeMOxxySDlERcToFCZ5AneSG2I31a60iAaMLnK95tSJkex3d2X+QB2Hlp0Nf2VODrefhKzEe8kRBEOEjpl1Qo+NvYERYzDlcrE.dznxXCIyyf8bWwshqG3yTT3eg7D0XCeAlcrcQxmvH.dPM1Rdh46fpE2kPT2TdCzrD7+p16nbLVKQW0ouy3Tg0.7lJJ6X.KnLkYs.+AlcratLkuMnK9Z6wkxpC3MTTTe.2XDI60gYG6vXeXFavT70EEGJax3darySzFHelHEvgIm6KGg5pTXJ9poLxYMmBtG8u.xlCF0rQL6XOBkdAGQE5hu1YTqjqFYtodChGG6.QodpadjXPm5PW701hJgeZHCRoK8deXToDObFZzkZ6.HaERbho3q2osBzYy9ZEoLeTS6m2V+.2GxfYQAyB3wPlKreN1B.aE4Q9LtpsKcwWGhP9v4cpHEfVWHOt5k5eJusiCrMrqBVlJxbgUSbSXZ8hrRu8ArKf60BaPG5hu5WQRqkkpQH1zr4AtXwQjtcZukE1fNzcWyKFFATKRLqtQlFgyKwA0WnCChbKmyKwA0lyKzg8ijDlvxdQRxRiH2po9Rkv4yAKdbPB6z.i7hcnAF4EJQO.epE1fWlG5SfzmGAxdbMlhuFpPcoQhMpzoYMGaWHascfopiczzrliUMLPYx7Q+fhsjlF0XAVI5iuF5JfrZn.2zrli0IRZQCEUcriPNz6X2psBqJB2NxpP8x1rQXIY8wVIwBPRZ9zPxw7EgT.F5RI4V.9JjTV1GxqdkuIYLyrEMh+EiWoZCf7iguToTzYIICij3lYirTYmkv2awOcV9rSEaWOxUxSBYIzcgr74pjF7+.kgzPkAHe4GM.....IUjSD4pPfIH" ],
					"embed" : 1,
					"forceaspect" : 1,
					"id" : "obj-38",
					"maxclass" : "fpic",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "jit_matrix" ],
					"patching_rect" : [ 404.0, 328.569670761929501, 23.0, 16.581395348837209 ],
					"pic" : "GrDXI.png"
				}

			}
, 			{
				"box" : 				{
					"autofit" : 1,
					"data" : [ 2001, "png", "IBkSG0fBZn....PCIgDQRA...XF...fOHX....P1B3F9....DLmPIQEBHf.B7g.YHB..GfXRDEDU3wY6blFiTTDE.9aW1EWNWAT.AT.EUHnDOvCPQwqnBhwHALDilfFwDATiJPhDvKhGfPhl3OPyJ5OvfJ3QPA7L.BJJhnQDvCTAUb4ZE4l0EF+wa5rc26q5o6p6oWxNyWRkYRO86UuoecWUWu5UUIXGmNvcCzRf+C3v.GB3fY+7PYOdo.kCzbWkiy02KG3..SMqrAQI.iGnOtpSsxQxVGUnT9GfYk8ylj7R.YRvxnBQcdtITcMhX7+N0nTKka2InMjAXSg37NDvQSf55WhoNREJwR4ZNvYCTIPaA5.vkib2XEgP9MBr.f0.74.aKj0a+.t.fw.bg43by.rRfUC7yHNjuEXGgrtZRwkhzFePMkbWX+MDNTNv5Ln+cBLIfSNl0QSNlGlcJ0h7DVRvJ8o6i.LSjmhKhBCffehY7IPcbd.04RmaC3pS.81jm0fYGyFHdMk0Ifeyk99cfSKF5qfhQSvO0bMVp2iGoCbG8rEfdDSasfhJP5D1ji4csPmsDu8q72.8JIL1BMdZL6XNBQ6N8lCrDWxWCxqqWDKn63sCZ+kmIj5oTfW2kb6C3hSZisPi2AyNlcR3FL5rcIygw99mJhKtJB9k.FcNj2cyg0AL77lkV.x5wri4qCPtI46buy7qYV3wXI3mZFfhLiw24LwTwRKvnM.6AyNl4567GIdi21zSMKs.jmGyNlCiLZd.tNj3o47aUk5VZAFmIxboXx4LEfKAX+tN17AZVigwVnwGRv807ut99hQFTYQRAtQB1w3TVNPKZjrwBRJEuQE1zqOmTyWSQh.SffcLWaimoUXS6QRWISNlY23YZEoJL6X1Kx3dJ3w1zWxVZFPOC32aMvskR1xwzD2rUIp0UUj6.WtNZ7mqkaB3QQlwz3vAQx5yZPxctUgjtVaNl5MQYlDtWWNCvk0HYiNDTjJRhxJPZYnbSFPZ8DyjAlV1umAIHky.y2Q9F.2RJXWlnK.2AxzYWIR9ReEJm21.dSpOyT8mq1cGnu.mB5WqWW15Y0InsGZtG7d2x3xd7Yg46npE3jRcK0LUhtcN3PJeqAtUj7TvuNpC35SVyM2LJ7Fo3Gy0u0KBN9YORpZoAyPPODRQMVdUBLGC5puIkwlKFBdiT7KnbNKByNl+BnrTwRyMSmFZeuuk5pLjb2VKpG4cFDdGH47P+Uy0tSzcYjogwFB9JZnsMgXnuoonuihjb94MNGjNCcpvO.yQJtTjrv2jiY44SCMjzVzy1mbsZCBBSyp6MGKKM.NCj2TwohVEPqxgLOfAizozu7kwFRzdpdODu4JZpJ5LC4dLdVQ2PxiXmJ46.ZWHjqc3cBx7Wdw7fsFElAMzlVTL04GqnyLjGFX8If2rg4mn9oKNL3N2w7W1OgyAmuX0J1TbRNjNfrFR8qycSBGhr1fWieyHCnJJzOL6Xx.7fIkwFQpD89WtnXnymUQeI9+wJ.9TWJuZjUzrMrLL6X1Doe.WAXnJ1xdw9Wi+rPR9D+5bYjf++JCuo.aMDuNpGAlcLYPtHk1n0+xhsTW8BXqJ5aUDsl8CjR.dU7dWTbd7FDG8ehYGyRho9sAs9WlTD0QaxJy1Uz0bIb4ucnnbfW1kxOH5A3yFlBlcLGE6alzFL0+RXWoAcD3wQB6uecrQxwXVhZzkaMvaQ8Yb+QyVA1rPjznSHqRLSCH84.t+DptxECE38TN9Ci3vbSI.cFYM+zCjnJ2dEYWKxafVkhNrlSjF9ncbBKgIlKlepY2j6ArlTn0+hskoSDWmng8IlAB7J3soj0Bb9Yq3jjAfLKel39PlHq7MqFn+9N1BA9AkysTfthbwu2zv4YZO.ODxV8RhPmQ5jWK77KLopDezEk5xcYKHSfU9DS8uDlPy2BDmvtTj+Ir0fbRVhQi7ZgtCau+xgPl8MiSOZDoaH+gpNf5zorTjfHluxsYswurchVex8G4Zje8bkgUAcDIAt2D5gJHWk8gLG11jAksEYrPtC7YTJG.IRCaDoo0a2BaPCs9WluE54IUzyaGVgGohv1TrYAqN7DptcJulE1fFZieYbAJgNCSQOgZ2epLj1rqAoCKmMAN2aHb0hzjgyl.m6hSRG7iHAwLprdjfT1Rj1zcuoz47YsYONHMa1BpeigqET+FR29A9HKrA+TIxdileVpE5Ravi40ICqoLZ8urCrKihdJEcUcXDrwHvfGqiVTLVN1MrfApbrsFFAK5XZHCV4XK0B8TA5S+bnxByhNFuTIRNK3mkYgtFH58w7qgQ3hNFuLHZ3Xi1Ev2agtzVl7.7GAHS6H6poqniwKCV4X11+hoXiYpy+ggbSv2.EcL9QaT41zLFXdcjVigiOQj27qVnniwM8A89WVgk5aKFNtVb9FBxF8JHITept9XNVhNi7zQ6PtPcpHyqTGUN2E.7kHS4Pc.eFgaz6CB8DVbxHgpwgdirA40djEObeI26Z6MYYNXeXehxjB9BJxWMR3q5Iv8hDqQmXNNXGAOVIosSab1.tO.xEj8gDBn5PVcBkfbsoLjmnZExr21LfOIB0yXQhZvTo9Vm5DvW367pF3FP1jWKRJROQxmgUhL.yZQhC3FPhjcW8Kv+Cfl7HmvqUOGq.....jTQNQjqBAlf" ],
					"embed" : 1,
					"forceaspect" : 1,
					"id" : "obj-37",
					"maxclass" : "fpic",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "jit_matrix" ],
					"patching_rect" : [ 311.0, 328.569670761929501, 27.0, 16.411764705882351 ],
					"pic" : "GrDXI.png"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-32",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "" ],
					"patching_rect" : [ 300.0, 146.0, 200.5, 22.0 ],
					"text" : "t l l"
				}

			}
, 			{
				"box" : 				{
					"autofit" : 1,
					"data" : [ 2664, "png", "IBkSG0fBZn....PCIgDQRA....I...fRHX.....Ui2c7....DLmPIQEBHf.B7g.YHB..J7QRDEDU3wY6c1GjVUUF.+2BKKKKpHryFDIp3G.CSeXnVlRDoTni8wTFCYwPZhyTAkM8gZJMSX1GNSyz1zPkE0pSSgIZgTZaTnqFMlBohCeHYx.RHtoPQBrvtzd6Oddem26649be26489w68ceO+l4NruWtOmmm68dt2yy4bdNOWvgiXPSoXY2AvTANSfwArcf+RJpOGkXt.mCvQ.9W.uBvNANdMzlFRNefUArMfCC3Yrs+Zmo0Pwj.NAAu9eLfMB7sAlWMy5LnEfEA73DzfKtMHvdQLbGoOs.b+.6B4ZeX2W1DvUVirQ.3R.dQzMtc.rBj2J0Zsx.cvo.LafaG3ei98pm.3Mm0F10.LfhwrefECLhr1fbLjbJ.2BP+D791A.dqYkgLezq7rUfojUFgiplqDnOzqDcNosxaAweFSkebfom1J2QhwUfdyY2aZq3kDhh+Voshcj37GQuCOmaZpz6RQoGCXrooRcjJbYn+xfEaSgXqytZMS8bHCXki5K1.heOlLSaJDaq.MZk8sMKKCG4G1mx9ZylBHI5t8gSfxvQsg+mx9ZwlBvMdMNLwp4G0UAxQrvUAxQrvUAxQrvUAxQrvUAxQrvUAxQrvUAxQrvUAxQrvUAxQrn4ZsATkzLxJ93rQF4zWDYRc0FZdMZEXVHqVj8ArGfCk7loCSdJBN8+2YFp+2EvCgdXY9Z.qDohUXr.fmTQ9AQhOlqh52GppFx76m0pJPc.rdEcO.AW4A8A7ALjeb.q133NAvAUJy0BLpT8rI+PCQEnymxCi1CB7k.lFhObiGYICYV4XgEjez.Opu+uMB7toTnoLcBVI76jxmS4Ex76maIiU3L.dUJ0LSm.SHji8NMrqWA4MO+be6acTdGGFOvCSvyomOgOOxqj4Uf1QFpvI.raJ8FkOwPb7cnXa9s2mF3jLjYkJxTbq8D3bHuylI348pRSEdHEEdGojt9g9zwmOhxzK5UF5ifK4nQ.7Rgb78idzWNbCs299qSKkc5JJyC3yjB5ZVHcI2C39rPt+F513OV4X0diUwsMUsFdcFqgfm6uDoTOQ+pJJyC37RX8zDkVq8GE3zrP1cEhMFVfhuGkicPf2S0X30gbCne8ZgURnpg1QbJMKdR8p7U9eCKjqEjE3noMtwJHyES4mW6sf9aT3MfrhZLul8jjfyRwIiTQwTIm.IOzjz767U9S1B4t.zeZ5lGB4FAxPBLUR27kTdkaD8qa2MIPknohjTnzTvME2BODJ5n95rTtOM514aOQstgezD5KXTuB6upVznsiTAQKQQ0Gv0EOath7BHNPOeKkqKBZquFMVSMQ0Ry.eezymP8B7EAFSkJf4frt2+l.ci9bM0OxzAj14Rllo5xoPOOAs4tSP6pQf4idxyvC3+hLh9chjdelQQgdcnmtV7u8CPRcZ4Ud8na2ekZoQUmRy.uejNeTo5D6sn.sBzC5CRXwsAPljwKKaNGrlOJ5186nVZT0obAH99nkCg7PbunWTFaslPhulOBvOB8tD6U3+KukMNLmGLOD+2ZTlU8jfIi9HS6gLkRecfKjgvWH+LEjJKEGQX+aaGoou7B+cBZiqulZQ0Wbw.uLAuFdLfkSLeP7ZQ267MS93MQmF5O0bq0RipNhof9fDeHjlyRDtdEE3Q9HtYVL511kTKMp5HdDzGpl4jzJ5upnnAn12T1cSP65HXYZJoAk2B5O7E4HrvlAYqSjwAxT94A7KsnbRZz5Y3iiLtUQkoBrTj4.JpIZx1.9THizcq.OSA4+8HM4qw4hL8OSqveOJjHr7YKH2VsvlSB9XJ6a.fuaZnrlQuqcqHMTVDYFJ1iGhie1PWEj6Eh3wOAzcb2CI0wMQiieVHYOdsNj3eaMDLn2RSteEaHUqDucEE1YZpvgfkpXOd.uSKJiSlRSYyeHBG+HoT.9eXj3w96Yn++AklHxafxGn1dJHyR.tMBNxu8P1MotOAAu1YS7WYMOshBSxPZsMfOGRShqFoIhJMivcqXOGG6lJjqymrWeDN9EV3XGjxW8GlUDtbJOjY2DxxRxjoQv4b7CZg8GGzhI5txZElTUflL5wbcXsGG1WklcXgNaAoYKOjOSCQo4ihQmvCZr+dTrkha2CU1o90Zb7oZbI6Cs6mZQuYnjmVZyqDeSPmOVPHG+UizbhI6wBctLfypveeiLzILzYgLnafzrkeL86oHqFwY0J4TeuF+9rTOprAuzrvSq2.MVB2AyWU43aiRu4vb6AhnNeSTJZ79yQTlNKb7+GJuGrsg9DR+LDszl6CZH2lin8DWh88y7xaflAgaKOjx9tcB+ozn7z6DQV8AsgTwcYQPFnTrRudjlOKxEQvgD43Hu44nQnbOaie+Oin8TyIuTAJrulg8C7SM12UfzyFPljuOrw++ajJGEhSCwOlheYZVFxBlLJTbhDMiVRsQs8NP505PQ6D76Swthn8T2QZ5DsoCzGgfcG+KSIGmOJk9FWcOFx9rHALteZGwgb+QYfsig0DQ+iQROF5uOjUEaTnXu57usHKsqpkgUqM9Kmxu4N.RWJ+B.+DJef6N.kGxqSBoRie6pWfeFRTVdeH9s3+F70lP1cqHyZc03GFHug0urCR3NjmzLrpBDHUh1phN7usIfyPQ1SBo6uZgiq+aNO.xb.kTLGE8rzHJ6HQ9pJa53cVQruel2B37tKrMODGVOSjv03XH85ZcHyYjVhj5vHit6JPVaWSGwemQgLxv6D32Rz7KwFzF06GNhxdoHqPV+r13YN4ap0IXp7Hl4snW1BYWEAudpMVXoECa5Fe8JihRCrXQ5IhxNFBNHoaAIU84mYiLEK4xj8PdqIr5MtPBFUlOZDk8Cg7kT1O2kwuGOxD71FhuR13bdlf6MPwi4pruGKhxdMF+tefegw9VDkFI6nFpIYJtJPwi4Z76CPzG7Py.gacHwlreVRg+cKj8AaVjvUAp5Qy+mGinMYjEyui9wLpNW.kVEv2l0VWFgsUfNgx9ZDynE.71Hn+OORDkUKmT+T996Igjg1.4sO+F6LsHSru2YaEHsPRvpORqCiPKVr6IhxtcBFBGE6PyLKTNsiDuzWMoWHVb5oT4FJZwPqMAv0vILm+s8gcOQeyFxuSjHO3nTZt.unDzdMYxD7doGobykKODk9dSSklS4Vo7qAe1pnLVN54jfmizI4c4mkonWOj7LPpwLQOvuh5P2ObhQhzM6ag38lhlQbF+lPhG7KE8HsLIoCzyR+GjfiMUhSWJJ1C3qk1J1QhPSHi2j18vgJc.lHbpnmyD8PBhpF0dkUOvXQFt.s6cqgL7d2ohrJL0LjsA7wI8eMriny3PVhT6G86YcQ0kU3hEiAI1jCKOBsWfeEhyluOjrHlirgyC3Shjlj6lvuGsMjgInlxTP552toxAB1tqMlWCGsS3YWLOjwx6dIg5kWR2l2YfD9AyFIn06nv1nA9SjxcQzAfzTzFPhqnCh7fq+sMfcwrTE4+CrUkRJCyK03S.....jTQNQjqBAlf" ],
					"embed" : 1,
					"forceaspect" : 1,
					"id" : "obj-24",
					"maxclass" : "fpic",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "jit_matrix" ],
					"patching_rect" : [ 148.0, 317.549958621754399, 34.864864864864899, 17.916666666666682 ],
					"pic" : "GrDXn.png"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-23",
					"linecount" : 3,
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 861.149235894551111, 423.549958621754399, 210.0, 49.0 ],
					"text" : "ears.format~ @spectype stft @framesize 2048 @hopsize 1024 @audiosr 44100 @resample 0"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-72",
					"linecount" : 3,
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 611.365452110767365, 423.549958621754399, 210.0, 49.0 ],
					"text" : "ears.format~ @spectype stft @framesize 2048 @hopsize 1024 @audiosr 44100 @resample 0"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-16",
					"linecount" : 4,
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "" ],
					"patching_rect" : [ 300.0, 265.504276519126051, 112.0, 62.0 ],
					"text" : "ears.stft~ @framesize 2048 @hopsize 1024 @polarout 0"
				}

			}
, 			{
				"box" : 				{
					"bubble" : 1,
					"bubblepoint" : 0.21,
					"bubbleside" : 3,
					"fontname" : "Arial",
					"fontsize" : 13.0,
					"id" : "obj-13",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 551.5, 128.0, 190.0, 25.0 ],
					"text" : "Split into overlapping frames"
				}

			}
, 			{
				"box" : 				{
					"bubble" : 1,
					"bubbleside" : 3,
					"fontname" : "Arial",
					"fontsize" : 13.0,
					"id" : "obj-5",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 563.0, 89.0, 165.0, 25.0 ],
					"text" : "Center first frame on 0"
				}

			}
, 			{
				"box" : 				{
					"autofit" : 1,
					"data" : [ 3438, "png", "IBkSG0fBZn....PCIgDQRA...vK...fRHX....P.B5yq....DLmPIQEBHf.B7g.YHB..MTRRDEDU3wY6ctGsUTUF.+GvkKWu9.g6hGARJ9.IVUpnlluBQxGgOVpYlIYVpkkT1CekhqUZpkqxjpkkZVnqJ07AhTXHk5UiVFORTWBnYPnHhnxqDtWAtvze7clNyYOey4L64Lybtm6Y+asl05NyY1ee6y49M6Yu+1ee6M3vQCD8JCk8f.FIvdAzefk.72yP84n9jwAru.aF3sAdGfWAXK0v5Tr3fAtKfECrI.OiiUW6pZN5lxPA5hv1JuOvbA9g.SnlU6TnYfIA7rDtR6erCfUhT4c3HHMC7v.KGwNIJanE.LwZTc7+yQB75nWAWJv0gzpeK0pJni5J1MfiB3F.VO51UyC3iVKpbmOv1TpPqF37.5csnR4nGC6FvUCrUBaisVfCJOqLm.5F6uDvHxyJhid7LQfNQ2neeyiJPyH8G2rBrEf8OOp.NZ33jPu6MOPdn7KLBk+CxCk6ngk+B5NCY+xZEe2JJ98A14rVwNZn43Pug1yyVAY6fK051xKiLoANbjU7DH8a2jwXqfr0fueJWaw1pTGNR.qR4ZsZqPRC2GtoTPFNbTI1tx0Z1Vg37Wti5YrNVvbF7NZnvYv6ngBmAuiFJbF7NZnvYv6ngBmAuiFJbF7NZnvYv6ngBmAuiFJZpVWAhAiDIHgVCRpCZafp0JxJmvfA1EjrxZ4HoQVdPSHeG1GjYF70QB3NsoJWiV.FKxJ+vp.dMfMl9USGZ7bDNDMuiLRWmKvaQ3Xf9ugrzNTNZBI18aGI4TzBsz0A7yQdXHK3S.7XnmlZuGvsg7fPTbV.yWo76.I9vOSpOZvJsHOs8xckdCJ5w73qGQYOCjVPMu+sgzpp406B4gizhAALmHzuYl42IvoZT99CbeJ0w0oHyY.z2Trt2cldrF7mb.YuhBm2LvUon6fKcC6JviG3y1dgy+7.CGY7J6DxaGdUC4rCfKHEp6GLklBjqC3x.FUA8O.jkqDSi4ytP46GvSG3ylKvwRwvxd+I7CM+3TndWOPMwf+ExXk1Of+SA41AvAF3y5CvaZn6kfXHMXf+Yfq+FT9t8LbfMXHq0ijs7IkQC7tT7AnoBLvHt26vP2uCRK6+1.WalTpSEF.vSR3e+e0pnNWOQMwfeoYrROm.x8JT97mRQ+iGYoYy+7oSzFZA4tTjklNiCCD4sQ9sX+Epv8OHEcG721EgL.6fbaJkw+nsDVuqmXgD968ck0JciJJ8lSQ462BVGHsnYhVx7F73JsPWSRo7+wDVu+kAjw2LlkYMJ52ue8lK2I8lvucy+XqnmIZ8zP6saSOKU3GTQgd.esTR9Ckh8OMpmbWVD0AOf6zR8MQEYjjzUbrTbvvOjEkKXWvpz2Cs2H3erfDTmqG4AI7282jLzSUWqhB8nz9YWMbQAj4mT4yaKB86g3pRa8VwmQQNavRYzKJttZ1AvdXQYWth98H5DS90Tt2cf9uU8D4RQ+2qytbEJozFx.qxxVWdDJZ3n8J5SUQ+dHYy9PRf9tbEY8ZVJiyLPYuQKJWynO+.ysLk4Hnz+GrxB5uQggiLoil+lMeR4HFXWQLrMUTWT4I.JtzaJt.Z9XQbO2hRcvC3KmPc96Uj0SZoL9ST72hgYQ4NDEc6g350xQuQbw4HIaWa+6txUf9ua2CojQ+HQ1DCzThMCPrRLl.x8aDw8n0m2kg3txjfou3K2XGhB+AwOSKK2WUQ2d.GlkxoQidg9hAlWgqm3EDr1PLn01XC5jzYRZBxHPVAy1HxfiMY2QeVRu0Dpu8RQVI4g3kUndcBVVtoon62iFqPEHozDRHgnsdxuFfuCxDLVVNFjoW+l.lM5w+wVQlx6rZ84teD8.Oip+6mbB00WIB4c3VJmlHYq+8ZucY1IPNMxbBnuv95A7eQlw5ohrztO5fEbvnu7WG73Wf31vZE+Dk5TWj7YFc5Jxa8j7tGYCe.Ec6A7cyAc2Sil.NEjA6WN62UFrPsfDUgZSpj+w1PBToiK6+Nnh1zJOuDJqlHbXE3gc9PuZ3ypnaOfOdNo+dRbHH8cWaMj2Co6lqgHlildgDy1eZfamnCq1am7c0Bd.n2+8jtmQMdEY4Qx81isXFGMdHiUpQIpGSCFF5y7pGRHd78ANThQe4CxHPLt0L1VBRWgxCNME86g8CTzmaWQVaG6bqX0v+RQ+yImzcOANBBmmDdHN8XJjBMb7EQeDwKj7ok9aUQ2aMg5tIzm.smIUpoUl8PQ2d.WSNo+5cFA5++aiHcuI0H3z9G7HOhE6Eon2jtAGe7JxxCXxUe0LVbdQn+iLmze8NOEg+sqSDuLl57OTT11Ha6Zy.QuKU2TBk2uVQVam7yCT2ih92LIXYetAjC.8FKrJZcsYhNlJhe3MK+D.tWaTpEbLnOkwOcBjUKHCHWSVuUDk4agDmNWKxC2UKZd45YQ5hVbYj.WBRLjD2M1qVAtXjYxsEfmuP4+yHcWUi8CIzQFUg+tuHYv0KVnbujE04zfOmx01FIexGqHMgt6ettrRgHOjYputHbxQDGNaEY4A7kh39On.2SZ7JyQGg9mhkxYZEJ2xh48OPzGnrGxVIiYf2MVjcGas2rF73AIY+eHo7vJ0gL+gtknnzolg564Uz27SnrdLEY0IQO4U9OrsCRVzXZxknneOfi1BYrqTLbOd7Xb+8ghIT9lPbk6O0P++aJ9VzKkRmDx1KTlKD35I7La1N4WPrMOB+aWlO2IZCfLqxqv1P26P+nDHqAf9rIGUWBZhhYjzBi3dZEIP2tWjt5cwT9H1a1J5eKXWnIbAAJ6EEi62+sZ6fRWcDLMbOQJMEBW.xxLhIihvwW0oYQ8uZPaxGmVsPoYkA+oqnKOjoR1VN2Hjk4RjgOep.2iVJ6MLzyu2n5O4PQ5Jl48uTK9NzLEy3qUS75NgejtNKiq2tRcw+39o7ChdFF2elmWoEPy1y1rbqa8Rs23Ttl+BwjsbPJWasHC9RCei7NA9cJe9sgQ.IUfyJB4cNnGmN1jvISFXuK72WAUdyjarHSRCHciIHQ0Es6CYvgkaPzqw378V8txG7xZEjmsvqsjfrnDJKM2QFkmkBtt3XZn.xDdE0.5dWk6uUhNWbezXV++HTLaeh6C79iAYCTp23ZE8t287DusAxYYTtn5xWZSpX60csE9Af7OYSRh6HAwcZln0ch9hjYUfzBplOdGMQ+6lV1ZcCDcqfwo0wgfDcmsh7fVbmjL+bccNHcmxmCmvtidKHsr2QLj69Xb9aDy5S2B5tZveXnO5+mJgx6kUt1aabdeQZ0eTEN+pPxJdSVcD5XqHuIIHmDhmO.InlNCiO+CS4yxoQgzO78sv4SF4MewA+.mxLarzbw5Mi3AtJQaH9jOHKOl0m5Rxqtz78TzSWHY9TRn+D16BAcm5NSoupdVTd2sYNf0MSX2Kd4TbfpcPwwQb+Fk8EQRP4fzFx.fCFwp1NeGCgvFmP3Ar1IxuOwAs4xXRVVuRJ8XWaIA87VLowOiOmBkZ.0Ahw2rJ729WucpbXkdhFxZaHtH6aC7qnzI5YsTZjcNTDi7fe2VCvuAIjIdHJMd86DI.9RCZAIpBSx3HfviEJslih3POZC9KyPGcP5jZgSjv+COnQ6sP7iBySDYl9zjk+wB.1SkxtKHtySKUJCZL8nHwPRZwwnnmKIlksOHcCzbft4EohsW20jF9mg7Z8iFouu2IRqhUKyB3Cgzh4Afz5zqi38mGk30OVelcgiIfL.w8BI7eeeDuxLSD2dpswGrIjYu75PVaY1ej9q2WjY97UPV1+ro9DGzlU23tzjLdjU.sfLipq5z8mZxqUbjZXtt0GUPyog1hOq1bQjUzi1sjNRe5KEmHJeZOlkcmH7jp8BD16WGERHOzscwcs6ZWZbj9bnDd7IwcdMNcBGjc2sw4C.If1ZEou91LX3bCWK7MNLNkqE2Ta77MNeqHKUgAYRTblZianKm63L3abXbFmuVh+jMYl3JyDI2RCh+9j0KP9mbHwFmAeiAZ8e+YPF3Wkve+oJHlwgzYQQ2Fe8VW6xQr0fuKkq0HtJ1VuwGiv8eOtgog1dB6yE3uGJxNfBHst+H1U0hMohclsF7ZgMZbhvNG0Vzxk11iYYWBgCIXemcLlBxoMj.zyeO5JKPaQ1MyQKuBsIIFbTavL9cVE10ho4VF5qfDYn9gjwlw9EhVaXXD1tyibn6SSIBEe7YshcTUbMT5+uhZSctbLEzW+QeYRuMFinXxJ50CYc5LSYLnm7C1tyY3HeoOHtM7po5ZItIjA+dkH4y63I6WwkGD56B4qipae0M1LMEk6gDRuNbjlzKz2Zh7nxaOPoF6N564SdHIRfyqMNRC1YD2epYm8fjy1Y6Nx5CiVkYwHqR.4wlKfiddzejk7jUit80zHY65JUM6DR9ZF05H+JA9CHCX5jQ14Kb3vjCDY0e6FQB25nrmVLhaOq4LBD2CsBJexPrhZS0yQ2XZin28N7Pl2mGfTzKPoc+f1SjPD8nPR73AU3ne.+UxA2H4nthVPVeKGMhWWVgwwSfcwreE4+ATKB5LRZDP6X.....jTQNQjqBAlf" ],
					"embed" : 1,
					"forceaspect" : 1,
					"id" : "obj-7",
					"maxclass" : "fpic",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "jit_matrix" ],
					"patching_rect" : [ 9.149235894551111, 386.961723327636719, 49.432432432432506, 19.457446808510667 ],
					"pic" : "/Users/danieleghisi/Downloads/GrDnXn.png"
				}

			}
, 			{
				"box" : 				{
					"autofit" : 1,
					"data" : [ 3438, "png", "IBkSG0fBZn....PCIgDQRA...vK...fRHX....P.B5yq....DLmPIQEBHf.B7g.YHB..MTRRDEDU3wY6ctGsUTUF.+GvkKWu9.g6hGARJ9.IVUpnlluBQxGgOVpYlIYVpkkT1CekhqUZpkqxjpkkZVnqJ07AhTXHk5UiVFORTWBnYPnHhnxqDtWAtvze7clNyYOey4L64Lybtm6Y+asl05NyY1ee6y49M6Yu+1ee6M3vQCD8JCk8f.FIvdAzefk.72yP84n9jwAru.aF3sAdGfWAXK0v5Tr3fAtKfECrI.OiiUW6pZN5lxPA5hv1JuOvbA9g.SnlU6TnYfIA7rDtR6erCfUhT4c3HHMC7v.KGwNIJanE.LwZTc7+yQB75nWAWJv0gzpeK0pJni5J1MfiB3F.VO51UyC3iVKpbmOv1TpPqF37.5csnR4nGC6FvUCrUBaisVfCJOqLm.5F6uDvHxyJhid7LQfNQ2neeyiJPyH8G2rBrEf8OOp.NZ33jPu6MOPdn7KLBk+CxCk6ngk+B5NCY+xZEe2JJ98A14rVwNZn43Pug1yyVAY6fK051xKiLoANbjU7DH8a2jwXqfr0fueJWaw1pTGNR.qR4ZsZqPRC2GtoTPFNbTI1tx0Z1Vg37Wti5YrNVvbF7NZnvYv6ngBmAuiFJbF7NZnvYv6ngBmAuiFJbF7NZnvYv6ngBmAuiFJZpVWAhAiDIHgVCRpCZafp0JxJmvfA1EjrxZ4HoQVdPSHeG1GjYF70QB3NsoJWiV.FKxJ+vp.dMfMl9USGZ7bDNDMuiLRWmKvaQ3Xf9ugrzNTNZBI18aGI4TzBsz0A7yQdXHK3S.7XnmlZuGvsg7fPTbV.yWo76.I9vOSpOZvJsHOs8xckdCJ5w73qGQYOCjVPMu+sgzpp406B4gizhAALmHzuYl42IvoZT99CbeJ0w0oHyY.z2Trt2cldrF7mb.YuhBm2LvUon6fKcC6JviG3y1dgy+7.CGY7J6DxaGdUC4rCfKHEp6GLklBjqC3x.FUA8O.jkqDSi4ytP46GvSG3ylKvwRwvxd+I7CM+3TndWOPMwf+ExXk1Of+SA41AvAF3y5CvaZn6kfXHMXf+Yfq+FT9t8LbfMXHq0ijs7IkQC7tT7AnoBLvHt26vP2uCRK6+1.WalTpSEF.vSR3e+e0pnNWOQMwfeoYrROm.x8JT97mRQ+iGYoYy+7oSzFZA4tTjklNiCCD4sQ9sX+Epv8OHEcG721EgL.6fbaJkw+nsDVuqmXgD968ck0JciJJ8lSQ462BVGHsnYhVx7F73JsPWSRo7+wDVu+kAjw2LlkYMJ52ue8lK2I8lvucy+XqnmIZ8zP6saSOKU3GTQgd.esTR9Ckh8OMpmbWVD0AOf6zR8MQEYjjzUbrTbvvOjEkKXWvpz2Cs2H3erfDTmqG4AI7282jLzSUWqhB8nz9YWMbQAj4mT4yaKB86g3pRa8VwmQQNavRYzKJttZ1AvdXQYWth98H5DS90Tt2cf9uU8D4RQ+2qytbEJozFx.qxxVWdDJZ3n8J5SUQ+dHYy9PRf9tbEY8ZVJiyLPYuQKJWynO+.ysLk4Hnz+GrxB5uQggiLoil+lMeR4HFXWQLrMUTWT4I.JtzaJt.Z9XQbO2hRcvC3KmPc96Uj0SZoL9ST72hgYQ4NDEc6g350xQuQbw4HIaWa+6txUf9ua2CojQ+HQ1DCzThMCPrRLl.x8aDw8n0m2kg3txjfou3K2XGhB+AwOSKK2WUQ2d.GlkxoQidg9hAlWgqm3EDr1PLn01XC5jzYRZBxHPVAy1HxfiMY2QeVRu0Dpu8RQVI4g3kUndcBVVtoon62iFqPEHozDRHgnsdxuFfuCxDLVVNFjoW+l.lM5w+wVQlx6rZ84teD8.Oip+6mbB00WIB4c3VJmlHYq+8ZucY1IPNMxbBnuv95A7eQlw5ohrztO5fEbvnu7WG73Wf31vZE+Dk5TWj7YFc5Jxa8j7tGYCe.Ec6A7cyAc2Sil.NEjA6WN62UFrPsfDUgZSpj+w1PBToiK6+Nnh1zJOuDJqlHbXE3gc9PuZ3ypnaOfOdNo+dRbHH8cWaMj2Co6lqgHlildgDy1eZfamnCq1am7c0Bd.n2+8jtmQMdEY4Qx81isXFGMdHiUpQIpGSCFF5y7pGRHd78ANThQe4CxHPLt0L1VBRWgxCNME86g8CTzmaWQVaG6bqX0v+RQ+yImzcOANBBmmDdHN8XJjBMb7EQeDwKj7ok9aUQ2aMg5tIzm.smIUpoUl8PQ2d.WSNo+5cFA5++aiHcuI0H3z9G7HOhE6Eon2jtAGe7JxxCXxUe0LVbdQn+iLmze8NOEg+sqSDuLl57OTT11Ha6Zy.QuKU2TBk2uVQVam7yCT2ih92LIXYetAjC.8FKrJZcsYhNlJhe3MK+D.tWaTpEbLnOkwOcBjUKHCHWSVuUDk4agDmNWKxC2UKZd45YQ5hVbYj.WBRLjD2M1qVAtXjYxsEfmuP4+yHcWUi8CIzQFUg+tuHYv0KVnbujE04zfOmx01FIexGqHMgt6ettrRgHOjYputHbxQDGNaEY4A7kh39On.2SZ7JyQGg9mhkxYZEJ2xh48OPzGnrGxVIiYf2MVjcGas2rF73AIY+eHo7vJ0gL+gtknnzolg564Uz27SnrdLEY0IQO4U9OrsCRVzXZxknneOfi1BYrqTLbOd7Xb+8ghIT9lPbk6O0P++aJ9VzKkRmDx1KTlKD35I7La1N4WPrMOB+aWlO2IZCfLqxqv1P26P+nDHqAf9rIGUWBZhhYjzBi3dZEIP2tWjt5cwT9H1a1J5eKXWnIbAAJ6EEi62+sZ6fRWcDLMbOQJMEBW.xxLhIihvwW0oYQ8uZPaxGmVsPoYkA+oqnKOjoR1VN2Hjk4RjgOep.2iVJ6MLzyu2n5O4PQ5Jl48uTK9NzLEy3qUS75NgejtNKiq2tRcw+39o7ChdFF2elmWoEPy1y1rbqa8Rs23Ttl+BwjsbPJWasHC9RCei7NA9cJe9sgQ.IUfyJB4cNnGmN1jvISFXuK72WAUdyjarHSRCHciIHQ0Es6CYvgkaPzqw378V8txG7xZEjmsvqsjfrnDJKM2QFkmkBtt3XZn.xDdE0.5dWk6uUhNWbezXV++HTLaeh6C79iAYCTp23ZE8t287DusAxYYTtn5xWZSpX60csE9Af7OYSRh6HAwcZln0ch9hjYUfzBplOdGMQ+6lV1ZcCDcqfwo0wgfDcmsh7fVbmjL+bccNHcmxmCmvtidKHsr2QLj69Xb9aDy5S2B5tZveXnO5+mJgx6kUt1aabdeQZ0eTEN+pPxJdSVcD5XqHuIIHmDhmO.InlNCiO+CS4yxoQgzO78sv4SF4MewA+.mxLarzbw5Mi3AtJQaH9jOHKOl0m5Rxqtz78TzSWHY9TRn+D16BAcm5NSoupdVTd2sYNf0MSX2Kd4TbfpcPwwQb+Fk8EQRP4fzFx.fCFwp1NeGCgvFmP3Ar1IxuOwAs4xXRVVuRJ8XWaIA87VLowOiOmBkZ.0Ahw2rJ729WucpbXkdhFxZaHtH6aC7qnzI5YsTZjcNTDi7fe2VCvuAIjIdHJMd86DI.9RCZAIpBSx3HfviEJslih3POZC9KyPGcP5jZgSjv+COnQ6sP7iBySDYl9zjk+wB.1SkxtKHtySKUJCZL8nHwPRZwwnnmKIlksOHcCzbft4EohsW20jF9mg7Z8iFouu2IRqhUKyB3Cgzh4Afz5zqi38mGk30OVelcgiIfL.w8BI7eeeDuxLSD2dpswGrIjYu75PVaY1ej9q2WjY97UPV1+ro9DGzlU23tzjLdjU.sfLipq5z8mZxqUbjZXtt0GUPyog1hOq1bQjUzi1sjNRe5KEmHJeZOlkcmH7jp8BD16WGERHOzscwcs6ZWZbj9bnDd7IwcdMNcBGjc2sw4C.If1ZEou91LX3bCWK7MNLNkqE2Ta77MNeqHKUgAYRTblZianKm63L3abXbFmuVh+jMYl3JyDI2RCh+9j0KP9mbHwFmAeiAZ8e+YPF3Wkve+oJHlwgzYQQ2Fe8VW6xQr0fuKkq0HtJ1VuwGiv8eOtgog1dB6yE3uGJxNfBHst+H1U0hMohclsF7ZgMZbhvNG0Vzxk11iYYWBgCIXemcLlBxoMj.zyeO5JKPaQ1MyQKuBsIIFbTavL9cVE10ho4VF5qfDYn9gjwlw9EhVaXXD1tyibn6SSIBEe7YshcTUbMT5+uhZSctbLEzW+QeYRuMFinXxJ50CYc5LSYLnm7C1tyY3HeoOHtM7po5ZItIjA+dkH4y63I6WwkGD56B4qipae0M1LMEk6gDRuNbjlzKz2Zh7nxaOPoF6N564SdHIRfyqMNRC1YD2epYm8fjy1Y6Nx5CiVkYwHqR.4wlKfiddzejk7jUit80zHY65JUM6DR9ZF05H+JA9CHCX5jQ14Kb3vjCDY0e6FQB25nrmVLhaOq4LBD2CsBJexPrhZS0yQ2XZin28N7Pl2mGfTzKPoc+f1SjPD8nPR73AU3ne.+UxA2H4nthVPVeKGMhWWVgwwSfcwreE4+ATKB5LRZDP6X.....jTQNQjqBAlf" ],
					"embed" : 1,
					"forceaspect" : 1,
					"id" : "obj-4",
					"maxclass" : "fpic",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "jit_matrix" ],
					"patching_rect" : [ 216.0, 264.504276519126051, 49.432432432432506, 19.457446808510667 ],
					"pic" : "/Users/danieleghisi/Downloads/GrDnXn.png"
				}

			}
, 			{
				"box" : 				{
					"autofit" : 1,
					"data" : [ 2664, "png", "IBkSG0fBZn....PCIgDQRA....I...fRHX.....Ui2c7....DLmPIQEBHf.B7g.YHB..J7QRDEDU3wY6c1GjVUUF.+2BKKKKpHryFDIp3G.CSeXnVlRDoTni8wTFCYwPZhyTAkM8gZJMSX1GNSyz1zPkE0pSSgIZgTZaTnqFMlBohCeHYx.RHtoPQBrvtzd6Oddem26649be26489w68ceO+l4NruWtOmmm68dt2yy4bdNOWvgiXPSoXY2AvTANSfwArcf+RJpOGkXt.mCvQ.9W.uBvNANdMzlFRNefUArMfCC3Yrs+Zmo0Pwj.NAAu9eLfMB7sAlWMy5LnEfEA73DzfKtMHvdQLbGoOs.b+.6B4ZeX2W1DvUVirQ.3R.dQzMtc.rBj2J0Zsx.cvo.LafaG3ei98pm.3Mm0F10.LfhwrefECLhr1fbLjbJ.2BP+D791A.dqYkgLezq7rUfojUFgiplqDnOzqDcNosxaAweFSkebfom1J2QhwUfdyY2aZq3kDhh+Voshcj37GQuCOmaZpz6RQoGCXrooRcjJbYn+xfEaSgXqytZMS8bHCXki5K1.heOlLSaJDaq.MZk8sMKKCG4G1mx9ZylBHI5t8gSfxvQsg+mx9ZwlBvMdMNLwp4G0UAxQrvUAxQrvUAxQrvUAxQrvUAxQrvUAxQrvUAxQrvUAxQrvUAxQrn4ZsATkzLxJ93rQF4zWDYRc0FZdMZEXVHqVj8ArGfCk7loCSdJBN8+2YFp+2EvCgdXY9Z.qDohUXr.fmTQ9AQhOlqh52GppFx76m0pJPc.rdEcO.AW4A8A7ALjeb.q133NAvAUJy0BLpT8rI+PCQEnymxCi1CB7k.lFhObiGYICYV4XgEjez.Opu+uMB7toTnoLcBVI76jxmS4Ex76maIiU3L.dUJ0LSm.SHji8NMrqWA4MO+be6acTdGGFOvCSvyomOgOOxqj4Uf1QFpvI.raJ8FkOwPb7cnXa9s2mF3jLjYkJxTbq8D3bHuylI348pRSEdHEEdGojt9g9zwmOhxzK5UF5ifK4nQ.7Rgb78idzWNbCs299qSKkc5JJyC3yjB5ZVHcI2C39rPt+F513OV4X0diUwsMUsFdcFqgfm6uDoTOQ+pJJyC37RX8zDkVq8GE3zrP1cEhMFVfhuGkicPf2S0X30gbCne8ZgURnpg1QbJMKdR8p7U9eCKjqEjE3noMtwJHyES4mW6sf9aT3MfrhZLul8jjfyRwIiTQwTIm.IOzjz767U9S1B4t.zeZ5lGB4FAxPBLUR27kTdkaD8qa2MIPknohjTnzTvME2BODJ5n95rTtOM514aOQstgezD5KXTuB6upVznsiTAQKQQ0Gv0EOath7BHNPOeKkqKBZquFMVSMQ0Ry.eezymP8B7EAFSkJf4frt2+l.ci9bM0OxzAj14Rllo5xoPOOAs4tSP6pQf4idxyvC3+hLh9chjdelQQgdcnmtV7u8CPRcZ4Ud8na2ekZoQUmRy.uejNeTo5D6sn.sBzC5CRXwsAPljwKKaNGrlOJ5186nVZT0obAH99nkCg7PbunWTFaslPhulOBvOB8tD6U3+KukMNLmGLOD+2ZTlU8jfIi9HS6gLkRecfKjgvWH+LEjJKEGQX+aaGoou7B+cBZiqulZQ0Wbw.uLAuFdLfkSLeP7ZQ267MS93MQmF5O0bq0RipNhof9fDeHjlyRDtdEE3Q9HtYVL511kTKMp5HdDzGpl4jzJ5upnnAn12T1cSP65HXYZJoAk2B5O7E4HrvlAYqSjwAxT94A7KsnbRZz5Y3iiLtUQkoBrTj4.JpIZx1.9THizcq.OSA4+8HM4qw4hL8OSqveOJjHr7YKH2VsvlSB9XJ6a.fuaZnrlQuqcqHMTVDYFJ1iGhie1PWEj6Eh3wOAzcb2CI0wMQiieVHYOdsNj3eaMDLn2RSteEaHUqDucEE1YZpvgfkpXOd.uSKJiSlRSYyeHBG+HoT.9eXj3w96Yn++AklHxafxGn1dJHyR.tMBNxu8P1MotOAAu1YS7WYMOshBSxPZsMfOGRShqFoIhJMivcqXOGG6lJjqymrWeDN9EV3XGjxW8GlUDtbJOjY2DxxRxjoQv4b7CZg8GGzhI5txZElTUflL5wbcXsGG1WklcXgNaAoYKOjOSCQo4ihQmvCZr+dTrkha2CU1o90Zb7oZbI6Cs6mZQuYnjmVZyqDeSPmOVPHG+UizbhI6wBctLfypveeiLzILzYgLnafzrkeL86oHqFwY0J4TeuF+9rTOprAuzrvSq2.MVB2AyWU43aiRu4vb6AhnNeSTJZ79yQTlNKb7+GJuGrsg9DR+LDszl6CZH2lin8DWh88y7xaflAgaKOjx9tcB+ozn7z6DQV8AsgTwcYQPFnTrRudjlOKxEQvgD43Hu44nQnbOaie+Oin8TyIuTAJrulg8C7SM12UfzyFPljuOrw++ajJGEhSCwOlheYZVFxBlLJTbhDMiVRsQs8NP505PQ6D76Swthn8T2QZ5DsoCzGgfcG+KSIGmOJk9FWcOFx9rHALteZGwgb+QYfsig0DQ+iQROF5uOjUEaTnXu57usHKsqpkgUqM9Kmxu4N.RWJ+B.+DJef6N.kGxqSBoRie6pWfeFRTVdeH9s3+F70lP1cqHyZc03GFHug0urCR3NjmzLrpBDHUh1phN7usIfyPQ1SBo6uZgiq+aNO.xb.kTLGE8rzHJ6HQ9pJa53cVQruel2B37tKrMODGVOSjv03XH85ZcHyYjVhj5vHit6JPVaWSGwemQgLxv6D32Rz7KwFzF06GNhxdoHqPV+r13YN4ap0IXp7Hl4snW1BYWEAudpMVXoECa5Fe8JihRCrXQ5IhxNFBNHoaAIU84mYiLEK4xj8PdqIr5MtPBFUlOZDk8Cg7kT1O2kwuGOxD71FhuR13bdlf6MPwi4pruGKhxdMF+tefegw9VDkFI6nFpIYJtJPwi4Z76CPzG7Py.gacHwlreVRg+cKj8AaVjvUAp5Qy+mGinMYjEyui9wLpNW.kVEv2l0VWFgsUfNgx9ZDynE.71Hn+OORDkUKmT+T996Igjg1.4sO+F6LsHSru2YaEHsPRvpORqCiPKVr6IhxtcBFBGE6PyLKTNsiDuzWMoWHVb5oT4FJZwPqMAv0vILm+s8gcOQeyFxuSjHO3nTZt.unDzdMYxD7doGobykKODk9dSSklS4Vo7qAe1pnLVN54jfmizI4c4mkonWOj7LPpwLQOvuh5P2ObhQhzM6ag38lhlQbF+lPhG7KE8HsLIoCzyR+GjfiMUhSWJJ1C3qk1J1QhPSHi2j18vgJc.lHbpnmyD8PBhpF0dkUOvXQFt.s6cqgL7d2ohrJL0LjsA7wI8eMriny3PVhT6G86YcQ0kU3hEiAI1jCKOBsWfeEhyluOjrHlirgyC3Shjlj6lvuGsMjgInlxTP552toxAB1tqMlWCGsS3YWLOjwx6dIg5kWR2l2YfD9AyFIn06nv1nA9SjxcQzAfzTzFPhqnCh7fq+sMfcwrTE4+CrUkRJCyK03S.....jTQNQjqBAlf" ],
					"embed" : 1,
					"forceaspect" : 1,
					"id" : "obj-3",
					"maxclass" : "fpic",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "jit_matrix" ],
					"patching_rect" : [ 47.0, 251.549958621754399, 34.864864864864899, 17.916666666666682 ],
					"pic" : "GrDXn.png"
				}

			}
, 			{
				"box" : 				{
					"autofit" : 1,
					"data" : [ 1391, "png", "IBkSG0fBZn....PCIgDQRA...PD...fOHX....fjp1aE....DLmPIQEBHf.B7g.YHB..EXRRDEDU3wY6b1FhUUDFG+2tq2z8krsjJE0VHTzMs.QBKirSuPRAaID9gfr9XJ8kHHvnBK5SQ0WKnBxfhJzdAquXV1lIaQnlxVXaj8BIlkus1Ztt4c2sO7bN3rWmYtm6Yl4bt6s9CCG14Lyy+Gdtyybl44YlsIDrDf0.LUfx.i.LLvYTJmMtsk.t.foE29jmk.lBvP.aHtOtfd.VIv3wxZDMOaAnUf1he1Zr97w.efKju0Xh8U4tbQYhwIbf+imURaN94Iyp.zfw.9IOHmS3PeOXV6XSwO6.nafoCbg.yDY35ppR+GG3SAdeDivIA9YfCmUERAyD3pAVAvif3VXBeKvt.1CvdA9ZDWJuim.6CMeuPPpF7bF3+i.tgbRG.jeUr4KuybRO1VE7dDfaOm397vKg8QIyKv7u5J3a2.yIvbZEWG1MHOS.4dA.CpvUe.WT.4K0X+X1f7KbtIm8ItTfeTgmcfL4ecAVO1GkD4Y9ZE3KYhtI0MFC.lMvnX1f7ZdjqlA1rhr+UfY4Q46MXa0rCAztm34ETj6f.KxSx063dwtayZ7.GOjh79GfawCxLXnUl3L9UV1lixuGjMVlHu62Q4kK3kwrAYTf4lQ4tTfSoHqM3rllSX4X2s4wxfL6B32UjwF8ghlm3GvrA46qQY0Iv2oz+OAItJSpvii8QIKKkxoDxNkS5W+TmrJzZEyE6qI4ESobdck9bHfqv6ZZNhJ28oZ4XHgXzFdJk1ODRHLmTi6C6tM2ik99.JsqLvcFTMMmPa.+ElMHawP+tUjEbkzt0FbMMGwqhYCxYAtrJZ+hXhKr6YyMMMmvMhc2lGVosyBYSZIu6cHLgLnvgZ7Jprrm31zNx12SpemH4wogDOI1GkrDfOT4uG.XFEhllR35v1tPR6PZjyQ.tdfC3Hm1vq.baHY16uiKCi70rQQxYTKHYXrDxGG5.YT7P3o.cscrOJYbfSiDa1PiimBcwV4JmhGThMBbyVd+XHwJ4q7.WUCqC3pPxu6L.tVfqwPaSRr0oPVBw.3mLNRGHYIyjU+Q8AIYDK0fN8zgjzlv7hz5ih8yqOnFcZLfK2TGZ1zKpAzMR9f0g2JVIJJDoot9A9CScvGFjUX4ca2Cx2EDootdCMoIiBpr7mTrtKKvfdUsSzfy3PFHdygl3p.cyeLJvkXqSt5xLeLmDodcT1thHM00OU4zE4pAQGoI3ycT1thHM08Yglz2.8tKGkhc9iEZPut6PS7AMPbdcxhLg0h94Ot3p0QWbYlGRRv0gdcPt9.QZpaejhCxmKFDcjlfhd9iaRSc8FZReSz6tbLpOm+vGmcVqPM8ipEmNAwd.ll+nyzz4r5xrXjyQpNriLJSegHM0sWj.bWUjUCxJs7tuHixzWH2m+XpHq3Sm6xvTrIq1z7G8DBxZAIlnax.oIkkG21h.5l+nLdHI5qB4nIzORn09Mj3hl1XSVFY2tCf78+9PxiSnwaqQW1kOD76pQvtVVuOTrpfCqg2muVDfofLuONWbCRtPQpWbmQPxQaYjIlKwDuLQIkjKUzQI7KLZgnOzfAeCc0qvz7GSuVDhOBgX8BhzT22fD.7TiFcCx+YcW5F8Sj2PbXbxBVG5m+vT5QLhFEWlHM0sajDXWSnQvfzLdL+KMBFj6fy+3aAY79.NY6XMsLjTezIRR1WLxBH0c0T1JxNuGDYSm6G4RI0vf1PRTcV25vHjhakkONeH4EFC4f60Exob7LH+xe53mI+uIH4DBMMjQNsib0VN.xIJ5+Qsf+EA7HelwbLniB.....IUjSD4pPfIH" ],
					"embed" : 1,
					"forceaspect" : 1,
					"id" : "obj-1",
					"maxclass" : "fpic",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "jit_matrix" ],
					"patching_rect" : [ 172.075129530122183, 254.549958621754399, 17.612903225806519, 16.058823529411825 ],
					"pic" : "GrDYI.png"
				}

			}
, 			{
				"box" : 				{
					"autofit" : 1,
					"data" : [ 1781, "png", "IBkSG0fBZn....PCIgDQRA...PE...fOHX....PsDvS+....DLmPIQEBHf.B7g.YHB..FvZRDEDU3wY6btGhUTEGG+yt9bWeb0j7QlYlFooRYEUt0ZaEjj0FRQAQoAQQAVQXkYXkQAEY0+DTQQgEj+QYlUVDkkrpqlklknkZYYlhuVe1t3l5U29iey3N6bOmwYNmyb268ZegCC6Ymy2e+lycNO983Lkgfw.LIft.jE3v.MC7uAJG06d6DPmA5p286esS.cDnQfY50FaPs.iGnEOtNrhqc.nBfJ8tVgm970.ehkx2J7UHJtqJ2jCzo8ag72mCjuQnbuqGzgbdbf+zA7reKZ61bf7MBk4cs6.i.nm.8.n+HC2l3Io8s.7s.yGoS7f.aFXmNP25OvnAFGvTQFVqCqCXU.qF3mA9AjoDJ3vSRzCs937jd7RZj+mCbE4IcvInRhdtr5yS5wBCI2F.tt7jrcNdCh9szgkxx+VCIueD3LSYYlp3xI5NzmKEk84Abf.xZ4.YRQ4k2v5QeG5eQqKt4Rb5.aJfbVBxhmkDX5D8ao03X4UAv2QaGlWxzYBv.ANF56PmsCkU4.eT.t2Bv.bH+ELHJqoZDnaNRNuR.dO.vHcDuEb31I5g8SxAxXJA36H.WiC3rfEUPaWwMbYgVxesHNlwmuIaIeEE3sPeG5w.Fjg7dw.MEfqYZslVjfpH5g8OgAbNXfcDfi20EJZwD9Mz2gtgDxUu.9k.s+aP7q5oTXFD8aoWVL4oSHdpxucqkRDqfRJFDQumzWOl77dAZy1ANKmqoEQHr2eBV1KRHRhBOSf6uQjPvbJMtShdX+sDQauq.2WVfIjpZZQBpD3ePeG5moocWKxF18uu6O00zhH71nuC8n.8Mz8ORZqgAuXdSSKRP0D8v9GNv8N.Dmb3++9.RGW9Uzif9qLbY0d2S2Pb+le80iDG++GJvSQzukNFfED3u2HPeZWzz7DrcX2fQBabb3oAfwB7GVJynvDQ1RVurjmlQBN49Pz2UfD9ksXIuwBKhneKsEfCgDapzFuZLzEaJ0i3lRslG6hEFlLhUO5vwAtMf44.YcxvY.b2HaqKCRxab0JtucALWjcc.RTB7yYqNiLxajHVuopOZcdxYkNT2OA5NRVZn6W0GKMDZLQFM5TMwr8cG3NnsdCKnQIWuaUWAkg9M4ubZe2dzDTnSGDIq8RBxfD2LUb47Pzb9JDje4AcsvRHlE4pSeggb0QD2SFluUE7lJO21kXLtH9eKxA7aCpQQc0YHWYQhFaXbQDXqftnC8pzTeC.+pC32TzSjG1vXwVv4NTTWYDnOHM6PWBxPh1Kbkj6bkMhX0loPmQImvw311gdtnOIDpyRtsEp1tT8HNG2Tna5sUoo9Di6E8KHMZWIDCwJIWcZZVvWeP7hVXNO.tYjN.79JDPK.6g12sKkg1Fie+Rbi2kJ7xJ3qEfGwJMMD1lFgjuxrYc3FHWcpQjs9XBFEpMdYw3v2NGlBA3WdHWIDCgpTH+KMjqggDDwv7sBf9YslF.2iBg3Wt.WJHCfp4Oe7DxQO7ZytUv0bPNOTNEyQgfZAIpmEhyeFWuc0WfmE0msfM.byQ0XSmSAzmYbKkBu8eBx1npNTckgb7cNauxfANMEs8m.dSf2A4GKmiQg9g6SMMDXBftifiIkYALzjHbSeCc7Q7+Vpgb5JTih5V.RNTEFkijc1CEX3jqm9uOfemzMJCzEj7PR0unMS6axdoa9y33hsJ.dTj0.B29T4jtzAjXBMWEBLXoJRt+FcETs+ycSxVj7RPNsyg4wprodhHoV3ZQhT4VQhKTbm6Iq2CxFAVChilCufPZ.UyepxkamL77J3Y91nXySAg1VltMJTLgp8e9.FvSsJ3YSwog5VTZMz5IQ1+CRPvC9+gQxQorzZ.tB9wHvu3+QIXOj9deJCpyhOSjqpMsWRmOApfp4Oa.yLx3ETvUrNx5Nyv9B.nx+ml5j6pTT21iSCKk5PqQQc0Y.OcE3RUTerxZjRkNzL.Wnh5MI9QUg54Pi0m8iRkNzpI289tWjs8kTLVM0u0HZSuQLLnjoCsFE0Y57m5rcW2hR0h7i2pgRmNTUVwXZ3hqPS859zEMMjcRbDnznCcDnd9SS+dn72ZpW0WkmIf3tP.9Pn3Ksr6Oxai8F4A7bPb3a374GDq89djnRlEwKXwwZmpQltHLlAhIo9X3.KCw+oaFwALMGmGhBILaL2z2OMAx40Tz9ch30+gfDyL+C6aSDXNba7Xe6A1k20Cg7fzDhYvYQRfgxPdl5HxavcCIkD6.xwfLtXJHVY8zz5n39g7Y6HH1IvMhcYixoTXHHmifkgrw9if3Oi0i3YqAFtA+GgJIFjffC1AK.....IUjSD4pPfIH" ],
					"embed" : 1,
					"forceaspect" : 1,
					"id" : "obj-2",
					"maxclass" : "fpic",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "jit_matrix" ],
					"patching_rect" : [ 117.075129530122183, 254.549958621754399, 21.580645161290334, 15.928571428571438 ],
					"pic" : "GrDYR.png"
				}

			}
, 			{
				"box" : 				{
					"autofit" : 1,
					"data" : [ 1604, "png", "IBkSG0fBZn....PCIgDQRA...XE...fOHX....PrwyNv....DLmPIQEBHf.B7g.YHB..EreRDEDU3wY6btFaTTEE.9qKsXa4gfnhZPwGn.FswGQQ7YMpQsFQSHPB+PSpFAQD+A9feXPi+yWs9HwefjZh+.ESzDMlnflnnBpIRALgffZ8AQCRjTQf9ztc8GmcRuyz6c1Yt67Xm18K4lcyz4dNmd1YN268bOyTC1w4CrBfFA9OfA.5Gnuhe1ewimCnNfIpzNAkuWGPu.OUw95G0.rZf4qnScs7E0Q8ZZ+CP6E+rhjM.THBaKK.57RiHcsjx3+6.SNK62QhPan.PWA375GX3HPW+bYJi.QMV1uIBbw.mHvTAlAvMfb0P8An+6C38.5D3qANT.0aS.WAvxAtxRbtE.1Nv2A7SHNzcC72ATWUTbsHw376VwG.6+A0g5.1iA4eXf0BblkoNp3XSX1oNHxU3QAa2iryCzFxcQiIYg3+UrqNBzwkALjhLODvMGAxshmNwri8Gn7BELSfeUQd+Fv4UFxKSQq3+Us2hkxcZHC.4HmC.b1koslondjAQL4X+.KjYi3Nt5AAlSTXrYMdVL6XyS3tRah.aVo+ciLcuwkLabO.i21yEP4jC3cT52wAtpn1XyZ79X1wdXB1hIVuReF.6iOOlhaB+GDq0Rze0vICAr3XyRyfrWL6X2gO8asdN26OdMyrGqB+upcgZ5yx8bNOQhXoYLlBvQwricidN+kh67M77IlklA4Uwric.jUSAvsgjOAm+VGItklwXtH4R0jycc.WCPOJG6cAlPZXrYM9D7OV6+p78OFYQAUI.rH72w5z9RfFRIaLSRNbmUJSS+JpxW63Jdb72wdqomoks4jP1taSN10mdlV1mNvri8XHy6Myisa+ssLAfywm+9jAtmDxVhUJ2cKMr5pCJchW1CoetV2.xdn0Oxbq6AoRcFBYUgCibQRsH6XbiHWTLIj65ZNIM11HXS2p.v0mjFlF5lfaq5ZmasIjg9j.qo32KfjjkW.YOrzwpPlOaZwJAtPj7EOCjhDoICm6N.1IRB3OJv9A9kDvF4gv8ulObwi2Nl+EePfSOILt.xkid67YRKCZY3NSUpFxbv+7G7zIpk5OqfQaeCyHIOJQoEbmopWSy47QX1w9mHCNTIvaynsuuOMLjqC2KDXSnepcsfYGaAjbyVIvAYz11qjzFwkfTlmNFvVvblpxgTEf9kPlzl4hda6tSRi3BPpiJGk+sHysyOVC9eUqoQiSJzEeMOxxySDlERcToFCZ5AneSG2I31a60iAaMLnK95tSJkex3d2X+QB2Hlp0Nf2VODrefhKzEe8kRBEOEjpl1Qo+NvYERYzDlcrE.dznxXCIyyf8bWwshqG3yTT3eg7D0XCeAlcrcQxmvH.dPM1Rdh46fpE2kPT2TdCzrD7+p16nbLVKQW0ouy3Tg0.7lJJ6X.KnLkYs.+AlcratLkuMnK9Z6wkxpC3MTTTe.2XDI60gYG6vXeXFavT70EEGJax3darySzFHelHEvgIm6KGg5pTXJ9poLxYMmBtG8u.xlCF0rQL6XOBkdAGQE5hu1YTqjqFYtodChGG6.QodpadjXPm5PW701hJgeZHCRoK8deXToDObFZzkZ6.HaERbho3q2osBzYy9ZEoLeTS6m2V+.2GxfYQAyB3wPlKreN1B.aE4Q9LtpsKcwWGhP9v4cpHEfVWHOt5k5eJusiCrMrqBVlJxbgUSbSXZ8hrRu8ArKf60BaPG5hu5WQRqkkpQH1zr4AtXwQjtcZukE1fNzcWyKFFATKRLqtQlFgyKwA0WnCChbKmyKwA0lyKzg8ijDlvxdQRxRiH2po9Rkv4yAKdbPB6z.i7hcnAF4EJQO.epE1fWlG5SfzmGAxdbMlhuFpPcoQhMpzoYMGaWHascfopiczzrliUMLPYx7Q+fhsjlF0XAVI5iuF5JfrZn.2zrli0IRZQCEUcriPNz6X2psBqJB2NxpP8x1rQXIY8wVIwBPRZ9zPxw7EgT.F5RI4V.9JjTV1GxqdkuIYLyrEMh+EiWoZCf7iguToTzYIICij3lYirTYmkv2awOcV9rSEaWOxUxSBYIzcgr74pjF7+.kgzPkAHe4GM.....IUjSD4pPfIH" ],
					"embed" : 1,
					"forceaspect" : 1,
					"id" : "obj-56",
					"maxclass" : "fpic",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "jit_matrix" ],
					"patching_rect" : [ 103.0, 225.549958621754371, 23.0, 16.581395348837209 ],
					"pic" : "GrDXI.png"
				}

			}
, 			{
				"box" : 				{
					"autofit" : 1,
					"data" : [ 2001, "png", "IBkSG0fBZn....PCIgDQRA...XF...fOHX....P1B3F9....DLmPIQEBHf.B7g.YHB..GfXRDEDU3wY6blFiTTDE.9aW1EWNWAT.AT.EUHnDOvCPQwqnBhwHALDilfFwDATiJPhDvKhGfPhl3OPyJ5OvfJ3QPA7L.BJJhnQDvCTAUb4ZE4l0EF+wa5rc26q5o6p6oWxNyWRkYRO86UuoecWUWu5UUIXGmNvcCzRf+C3v.GB3fY+7PYOdo.kCzbWkiy02KG3..SMqrAQI.iGnOtpSsxQxVGUnT9GfYk8ylj7R.YRvxnBQcdtITcMhX7+N0nTKka2InMjAXSg37NDvQSf55WhoNREJwR4ZNvYCTIPaA5.vkib2XEgP9MBr.f0.74.aKj0a+.t.fw.bg43by.rRfUC7yHNjuEXGgrtZRwkhzFePMkbWX+MDNTNv5Ln+cBLIfSNl0QSNlGlcJ0h7DVRvJ8o6i.LSjmhKhBCffehY7IPcbd.04RmaC3pS.81jm0fYGyFHdMk0Ifeyk99cfSKF5qfhQSvO0bMVp2iGoCbG8rEfdDSasfhJP5D1ji4csPmsDu8q72.8JIL1BMdZL6XNBQ6N8lCrDWxWCxqqWDKn63sCZ+kmIj5oTfW2kb6C3hSZisPi2AyNlcR3FL5rcIygw99mJhKtJB9k.FcNj2cyg0AL77lkV.x5wri4qCPtI46buy7qYV3wXI3mZFfhLiw24LwTwRKvnM.6AyNl4567GIdi21zSMKs.jmGyNlCiLZd.tNj3o47aUk5VZAFmIxboXx4LEfKAX+tN17AZVigwVnwGRv807ut99hQFTYQRAtQB1w3TVNPKZjrwBRJEuQE1zqOmTyWSQh.SffcLWaimoUXS6QRWISNlY23YZEoJL6X1Kx3dJ3w1zWxVZFPOC32aMvskR1xwzD2rUIp0UUj6.WtNZ7mqkaB3QQlwz3vAQx5yZPxctUgjtVaNl5MQYlDtWWNCvk0HYiNDTjJRhxJPZYnbSFPZ8DyjAlV1umAIHky.y2Q9F.2RJXWlnK.2AxzYWIR9ReEJm21.dSpOyT8mq1cGnu.mB5WqWW15Y0InsGZtG7d2x3xd7Yg46npE3jRcK0LUhtcN3PJeqAtUj7TvuNpC35SVyM2LJ7Fo3Gy0u0KBN9YORpZoAyPPODRQMVdUBLGC5puIkwlKFBdiT7KnbNKByNl+BnrTwRyMSmFZeuuk5pLjb2VKpG4cFDdGH47P+Uy0tSzcYjogwFB9JZnsMgXnuoonuihjb94MNGjNCcpvO.yQJtTjrv2jiY44SCMjzVzy1mbsZCBBSyp6MGKKM.NCj2TwohVEPqxgLOfAizozu7kwFRzdpdODu4JZpJ5LC4dLdVQ2PxiXmJ46.ZWHjqc3cBx7Wdw7fsFElAMzlVTL04GqnyLjGFX8If2rg4mn9oKNL3N2w7W1OgyAmuX0J1TbRNjNfrFR8qycSBGhr1fWieyHCnJJzOL6Xx.7fIkwFQpD89WtnXnymUQeI9+wJ.9TWJuZjUzrMrLL6X1Doe.WAXnJ1xdw9Wi+rPR9D+5bYjf++JCuo.aMDuNpGAlcLYPtHk1n0+xhsTW8BXqJ5aUDsl8CjR.dU7dWTbd7FDG8ehYGyRho9sAs9WlTD0QaxJy1Uz0bIb4ucnnbfW1kxOH5A3yFlBlcLGE6alzFL0+RXWoAcD3wQB6uecrQxwXVhZzkaMvaQ8Yb+QyVA1rPjznSHqRLSCH84.t+DptxECE38TN9Ci3vbSI.cFYM+zCjnJ2dEYWKxafVkhNrlSjF9ncbBKgIlKlepY2j6ArlTn0+hskoSDWmng8IlAB7J3soj0Bb9Yq3jjAfLKel39PlHq7MqFn+9N1BA9AkysTfthbwu2zv4YZO.ODxV8RhPmQ5jWK77KLopDezEk5xcYKHSfU9DS8uDlPy2BDmvtTj+Ir0fbRVhQi7ZgtCau+xgPl8MiSOZDoaH+gpNf5zorTjfHluxsYswurchVex8G4Zje8bkgUAcDIAt2D5gJHWk8gLG11jAksEYrPtC7YTJG.IRCaDoo0a2BaPCs9WluE54IUzyaGVgGohv1TrYAqN7DptcJulE1fFZieYbAJgNCSQOgZ2epLj1rqAoCKmMAN2aHb0hzjgyl.m6hSRG7iHAwLprdjfT1Rj1zcuoz47YsYONHMa1BpeigqET+FR29A9HKrA+TIxdileVpE5Ravi40ICqoLZ8urCrKihdJEcUcXDrwHvfGqiVTLVN1MrfApbrsFFAK5XZHCV4XK0B8TA5S+bnxByhNFuTIRNK3mkYgtFH58w7qgQ3hNFuLHZ3Xi1Ev2agtzVl7.7GAHS6H6poqniwKCV4X11+hoXiYpy+ggbSv2.EcL9QaT41zLFXdcjVigiOQj27qVnniwM8A89WVgk5aKFNtVb9FBxF8JHITept9XNVhNi7zQ6PtPcpHyqTGUN2E.7kHS4Pc.eFgaz6CB8DVbxHgpwgdirA40djEObeI26Z6MYYNXeXehxjB9BJxWMR3q5Iv8hDqQmXNNXGAOVIosSab1.tO.xEj8gDBn5PVcBkfbsoLjmnZExr21LfOIB0yXQhZvTo9Vm5DvW367pF3FP1jWKRJROQxmgUhL.yZQhC3FPhjcW8Kv+Cfl7HmvqUOGq.....jTQNQjqBAlf" ],
					"embed" : 1,
					"forceaspect" : 1,
					"id" : "obj-55",
					"maxclass" : "fpic",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "jit_matrix" ],
					"patching_rect" : [ 48.0, 225.549958621754371, 27.0, 16.411764705882351 ],
					"pic" : "GrDXI.png"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-84",
					"maxclass" : "newobj",
					"numinlets" : 4,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 300.0, 487.961723327636719, 601.0, 22.0 ],
					"text" : "ears.expr~ ($f1*$f3+$f2*$f4)/($f1*$f1+$f2*$f2 + 0.001) @normalize 0"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-81",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 3,
					"outlettype" : [ "bang", "bang", "" ],
					"patching_rect" : [ 641.0, 53.961723327636719, 103.0, 22.0 ],
					"text" : "t b b l"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-80",
					"linecount" : 2,
					"maxclass" : "newobj",
					"numinlets" : 4,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 892.333333333333485, 374.549958621754399, 141.0, 35.0 ],
					"text" : "ears.assemble~ 0 @mode 2 @normalize 0"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-79",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 933.0, 334.549958621754399, 69.0, 22.0 ],
					"text" : "ears.trans~"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-63",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 300.0, 62.961723327636719, 74.0, 22.0 ],
					"text" : "cherokee.aif"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-66",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 300.0, 117.961723327636719, 94.0, 22.0 ],
					"text" : "ears.channel~ 1"
				}

			}
, 			{
				"box" : 				{
					"color" : [ 1.0, 1.0, 1.0, 1.0 ],
					"fontface" : 0,
					"fontname" : "Arial",
					"fontsize" : 10.0,
					"id" : "obj-68",
					"maxclass" : "ears.specshow~",
					"maxnumbins" : 128,
					"maxvalue" : 2397.40966796875,
					"minvalue" : -699.48516845703125,
					"numinlets" : 3,
					"numoutlets" : 0,
					"patching_rect" : [ 300.0, 517.549958621754399, 409.333333333333485, 142.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-75",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 5,
					"outlettype" : [ "", "", "", "", "" ],
					"patching_rect" : [ 300.0, 90.961723327636719, 66.0, 22.0 ],
					"text" : "ears.read~"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-48",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 741.0, 89.0, 190.0, 22.0 ],
					"text" : "ears.shift~ 1024 @timeunit samps"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-28",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 741.0, 274.549958621754399, 168.0, 22.0 ],
					"text" : "ears.window~ @wintype hann"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-25",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 741.0, 334.549958621754399, 69.0, 22.0 ],
					"text" : "ears.trans~"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-54",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 3,
					"outlettype" : [ "", "", "" ],
					"patching_rect" : [ 741.0, 157.961723327636719, 55.0, 22.0 ],
					"saved_object_attributes" : 					{
						"versionnumber" : 80105
					}
,
					"text" : "bach.iter"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-14",
					"linecount" : 2,
					"maxclass" : "newobj",
					"numinlets" : 4,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 711.0, 374.549958621754399, 141.0, 35.0 ],
					"text" : "ears.assemble~ 0 @mode 2 @normalize 0"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-11",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 2,
					"outlettype" : [ "", "" ],
					"patching_rect" : [ 741.0, 304.549958621754399, 211.0, 22.0 ],
					"text" : "ears.fft~ @fullspectrum 0"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-6",
					"linecount" : 2,
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 2,
					"outlettype" : [ "", "" ],
					"patching_rect" : [ 741.0, 117.961723327636719, 194.0, 35.0 ],
					"text" : "ears.split~ 2048 @mode duration @timeunit samps @overlap 1024"
				}

			}
, 			{
				"box" : 				{
					"autofit" : 1,
					"data" : [ 14836, "png", "IBkSG0fBZn....PCIgDQRA..DfD....qHX.....ZgmYT....DLmPIQEBHf.B7g.YHB..f.PRDEDU3wY6cmGsjTTl2G+6s2a1Z1aVksVQDbAADnEZZEDj0AgQEvWPQk8EGQAkAGTPGEQAwYTbPEQX.EUPDkQPPjlEaP1jlEa1DPVjtYoYG5866e7T4oxJqHqaVYFYlUl4uOmSdt2ptUEwSs7D2nhJxHFMh.6Bv5B7XkbbHYyn.1OfkA3oJ4XQF7n775AkmKR4SsmVOn1SaFT9Z8fxWkBwxCbI.CCLefwWtgijQuKrWKWJvY.L5xMbjADJOudQ44hTdT6o0Kp8z5MkuVun7UI2sV.yB6MZCC7aK2vQ7fIBbuz90zqDaTVklKkmW+n7bQJGp8z5G0dZ8kxWqeT9pjqVGfGh1uA6l.lPoFQhuLYfYS6WaudfkqTiHorn775KkmKRwRsmVeo1SqeT9Z8kxWkbwpQmMZ7v.qZoFQhus9.yg1uF+m.FWYFPRgS440eqOJOWjhfZOs9a8QsmVWn705u0mbNecnLd+ey.GF1zaYQ.K.6b75MZ8y4255GEvXwB9fiwG52GKvqCbxstuiTLeL.aRn5z0wRZUGSvwwK.bls9YSxDAtNfst0kmGv1B7fkVDI4ksBXFzdpmcw.G.VCI9vlCbfX4XKFKmKbdePtOzN2eBzNmb7st9w.7J.e4V2mrXOwVHtBNWSWfieNZr7fko0OmXq34pA9MYr9GTn77li7NOWFbn9aUNT6oMGpeSU+9Mo70liA59+7in8n23ii8OA04l6o55CmgG2UU+bZ+3ew.SqbCGImsuz464OIOV1+A7SdXvwd4gX5ExP8OOOT+CJTddyRdlmKCNT+sJGp8zlE0uopc+lT9Zyx.a+e9V3uD8kB7dRPctIXeaEYst17L73tJ5HnymC9Fka3HEjym1uluDf2umJ2eIYKGL7wR.1LODSORFhg61C0+f.km2LkW44xfC0eqhmZOsYR8apZ1uIku1LkK4qY8TrYb.ucfIArB.qBvNf8sEjjECm6G3RAtCfYBL2DVuuCroVygxH+O4GF3OCbaXmSZOLvcA7rIrtpCdm.+EZu8Vc+XaURKnzhHonrh.2C1BVE.OCvaC34yX4tbXcddEv1F0VCroo4dOB2ugAtVfKC6eL+R.OJ14RXVsFXsGMMfiiduxVeu.2Nvch0dvsR0OeP44MW4UdtL3P82pXo1SatT+lbaPteSJes4pR0+msiQ9ac3PH6CPyXoys6mvGOGvW.XcyXcT0MZrFzBO5ZSsTiHoWVCfiEaZhs8dpL2U5L23h7T45xWhdm2+qyw5Nr391VuBf2aAECEIkmWsT0yykAGp+V9mZOsZop2dp52T1n70pkpd9ZlcwDex9BwFAUe3OGorWBvYf8srH1HBG94mue4FNROroXeqdAuVsTrjdeXlz46C1COUtQsLz6yo0aJmp2ntlH06yBryETcWFTdd0QcHOWFrn9a4Wp8zpi5P6opeSYixWqNpC4qY11RuGQziwC0w6FaQ3InLmKvN4gxstXsAdMZ+7y7AVqRMhjd4xn67jyxSk89DobeDxusvte.8N2eJ4T8F3CGo9tCZO06piTdd0RcIOWFbn9a4Op8zpk5R6opeSoixWqVpK4qY1cP7I6ylrMkOmL14fWP48X.aTFJu5nnM398J2vQ5gUF2SS5ulmJ+QQm6K7CC7Y7TYG01P748CC7Uyo5EfMF3ECUWyj522tYTJOu5nNkmW1VQfCB3eE6z+noS82xOT6oUG0o1SU+lRGkuVcTmxWyrCldmv+ARY4thXKPPAkyiCr9YLVqa1ProVavyQKfpwnA2TsW3NGwGaiaAhtBe+r3uodcTyl3y6eLx94CuKqF1hBXP8bCXKJZ0YJOuZotkmWl9Iz9w3QWxwxf.0eqrSsmVsT2ZOU8ap+n70pk5V9ZlLArEuq3R3u7TTlKCcddv9zj+S8rpnyiNet9GUtgiLBNC5N+XoXi3puLQrFKBWGGuGK+v9hDed+v.S2y02DAt4Pk+cP04exmEJOuZotkmWlBmue1kbrLHP82J6T6oUK0s1SU+l5OJesZotkulYmFwmruD5uuIhwAbUgt+yCa6oR5zJB7Fz4y0aaoFQxHI7Jvcvwrxg54qDoNt+bnN.67BsW6rBmmGqqQAbIgJ6+AvZ5wxePkxyqdpa44kov6nJ+ukbrLnP82J8T6oUO0s1SU+lRNkuV8T2xWyr0iNWXuhd7MSX4LJfeQn62qhcN6Ic6voymienxMbjQvJf6bjuaNTWqFvqGodlVNTO.7GH979WAXY8T8DdToeQrUI6l.kmWsTWyyKKZ.R5l5uU5o1SqVpqsmp9MkLJesZotlulY+FhOg+4vlZnijyIz8YAj9ym1lfakNeN9jK2vQFAQ2+tCN16bp9tfH0yEjS0y9S748CCbfdnNNpPk2BAd+dnLqJTdd0RcMOurnAHwM0eqzQsmVsTWaOU8aJYT9Z0RcMeMy1Q5cB+AOB2+vSazECru4VjV8soz8yuaXoFQxHw0zh12mWdgEcw760wlth91DoyUF8nGWSFK+8jNGQ5CJikWUhxyqdpq44kEM.Ito9a0+T6oUO001SU+lFYJes5otlu5E+MhOg+16w86KD419ox2vrxK5hfyrK2vQRfvKRVAG+0br9lhi56nxo55G5ntBNVBv5lxxcKvl12Ak0WNyQZ0hxyqdpy44kAM.IwS82p+n1Sqdpysmp9M0aJes5oNmulYgmRWtNbs35bnQtMmPgDoUWCALG574Ls59OXaYoysorfiuSNWuOcj56Fyo5Ypz679SLEk45Qmw+O0GAZEhxyqdp644kAM.IwS82J4T6oUO081SU+lhmxWqdp64qY1xC7xDeB+EE41+QnyUy4SuvhzpqMited8esTiHYjrS3Nevm6K3t7qhTeKfjctomFOHwm22uqtzqHv8E59+GAFq2hzpAkmW8zDxyKZZ.Rhm5uUxo1SqdZBsmp9M4lxWqdZB4qY1+Ewmvu.fI2518AoyQa5bK7HsZ5HnymSWJvpli02PXq18Esg.dK.6N11hVU1WktyEVB4+4J2w5nd2gbptNIG0U3isNgkyXAt1P2u6AXR9NXq.Jx7bki6GMg77hlFfjdS82JYT+lpdZBsmp9M4lxWqdpj4qiImBp378ANZrW3iZbXSwy+DvkR6Q27Rac8xHK51XzCfsp0mGlN14I4aB6Mg+vbpdhZiANeZ+OGFF68G+3Bp96GqI1Jz7jI9bsOhiqaT.e9Xt8K.35AtgLFatlpYSqUY6aW.voR7+SlONveIAkyOl1q15OM1+33kxbzU8TT44SGkimDJOWFDo9akLpeSCVT6oF0uI2T95fEku5QWM8dTQeoP+9Uh8Oxkj4IoymKujbpdVUfmIT8TTuYaU.9Gz86YV.46HHmVmM8985o83E8PrMZ5LWaXrby7x0P7OdddF477uRna+q.r44UfVATD44JGO4Tdd4PyfjQl5u0HS8aZvhZOsM0uoto70AK0170xXZC88Fg+9Jz5m2Hv9fM0OkQ1FR2SCq6KmpqSAX0Bc4rtsikT6G1H4F03v8HTV1b8M24COkGJikfspRG11R9MqxN+d72VYrsdt37wo8ps9R.9njuq90CxJp7bkimbJOWFTo9a0apeSCdT6oso9M0IkuN3o1luVFcR5J.dLf0uG2l6.XO.diBHdpK1JGWWdzvwaC3vBc4YA7Myg5wkozi+1FWPwP+3KBbYD+hBzoArIQttES6ELOWdd72VZ1s.rKgt7x0JdtGOU9g8qwFo4kOl+9GGa5cG0NB7iBc4iF3262PqRoHxyUNd+Q44xfJ0eqdS8aZviZOsM0uoNo70AOJe0yNd58TqYWh+tJw3Do6mG2rbnd98gJ+4C71yg5HNeLh+8L8Zj1GTcKz8iiYVf0+g6n9yyUU5eri5K3XQ.qdja+lhMM6BtME0+fZPVQjmqbb+pokmWTzoXSxn9aEO0uopmlV6opeSso70pmJa9ZYbJ1.1pjdu91J1mhJPpQ1.GW2b7bcrK.6ZnK+UnXGEteN11SlK40z7Jur7.agiqeFEXLLOGW25mi0WuZbeL.GPnKulX+SpfUa8eI1HU2zk244JG2uZh44xfE0eq3o9MUszDaOU8apMkuVszDyW8hyk3GkrWg3mRYhatVLm789b9MFprmEkyon0JC7vz8i0yqDhkrX2v86824BLFbs2j+cx45z0qcAG2YqayxhMsuCt9aBX74bbUUj244JG2uZp44EAMCRRN0eK2T+lpVZpsmp9MYT9Z0RkNesrlAIiF2iDXfkC3.KnXotH5ymuN1zuyW1XfsKzkOLryirh17v1RoVZjqetkPrjEuOGW2h.9yEXLTFirZu91P17VGWLv6t008f.+KXqf2R9lmqbb+qolmKCNT+shm52T0RSs8T0uIixWqVpz4qkwHiMD1hGjqm3B6HvVbhJS6M1zqZEyX47F.u.1KT+cryIqYhsUN4CCQ2q5w9deN+SE52uRrGCkk6E35vVHpB7PkTrjVS2w0ca.uVAFCkw+n+BvVcviapBdmg98mEaDne9bNlTdtQ4392zcbcMg7bYvPUp+V+Hrusu4ike7ZXsotXrEyukhMXOiA6aMdYvFbmkEaVvLc5u1CU+lpdsoNcGWWSn8zAw9Mo707kxW8iJU+eNChephE8XZkTLF3+hjGqo43lv9lax5TDaYbT1+sLVlgMFryyuAkWW.64svOdmZ4FN8kIg8OPh9Z1+YAGGKuiX3wJf58O4ndid75.aSADKfxyAkimGZ5444McJ1LxpR82Zdj7X00wF1m0m52T0pM0ld6oCZ8aR4q4OkulcoNesnmAImDvw052GF3PA9VD+2b6QAbCEPbEmSC3YvRLmD1VCjquIl4B7qvV4nA6TWZrX6a0iCX8vVYoeSz4H.+dacbB.eRrQVKMVFGWmOGY08.Xxs98alr8ZxVBrCXi380kgxI78cQTs1e2mFtO81lQAGGuB1n8Gtc.WuWx29oz6uQykh8OFJpQuW44JGOOzzyykxUUq+VGA11q4D.VErszy2QL21aGq8kWE3kAd.fGoOqO0uopUapM81S+oLX0uIkuFOkup709xQRmifyQ255OSheDFWH1px7fhIg63b5I79ubXagSOsixXwz4Jmb+X8bTdWYJKKW9kgJ28LCkyNgcNQFTVaZFJqkMT4bqYnbJCt9V8VHkSR6biDGuZATmKGc99fnGGeADC8RSLOW439WSOOOuoYPR7pC82ZKvcbdJdp7U+lpVZ5smNn2uIkuZT9poomulX6OcNUaBmvLErQ9Ltj9ubgFo8lqUj2WB67rqeLIrUiXWkUZRl1DGk0EmhxINAu45dIaayTyjNiwrr8B9lBUNe2LTNkg6jte8pHWzhBa1QhikT.04PXeKBtx2mIk+VYVSLOW439WSOOGrNC86.9mXme597H7imE64x90wV6f9Z9+ojbWco+VGFcGeKk1eKuYk52T0RSu8zA89Mo7UixWMM870DY2vF0nf.6663176wcR+v.OEkyhIqKmNcGe+eorrFCv86n7t8TTVatix4Glx3JpvMJ8ExP4DNQO3XqyP4s8gJm8KCkSQakXv37xKvMGINddx+c2p2Fty0GF3Xx45NIZZ44JG2+TdtY53NOupbr.Juc6uznN0eqeNcGeyxiku52T0gZOcvueSJeU4qApE4q4cB81CbIzdwI7Wf6D4uWOJi0hrMBb9zzcbcyHkk0hwdtIp2M14yW+v0ng4q8F7cHzueMYnb1NGW2ClgxK7BnTYMpjoQbmWdWWQGHsDsyvqbATm8Zwu5OU.0+HY5NttYjxxpJjmqbb+S44lmtfpm7xyP2a0hCpT+s5OpeSUGp8zlU+lbQ4qUG0h7077aJ3cgM0ZmXqKe0.GDt6rwUgssXtQwTVGM14GVYZEn8dLdXWeFJSWcdbHrj0eceTNutiqaRoJh5VPB5yQ1Vffht5KOKrsD0z5Cz5m2GvSjgxonMcGW2hvlVdkgnuOYgj+effcHlq+YwuqJ4oQSLOW4392zcbcMs7bvVL9lF1hVmu9eRANRfUq0uOWf+GOV1KsUYdsdrLyS0s9asw.qgiq2mcvV8ap5X5NttlV6oCx8aR4qFkuZltiqqokuFq2BctnnbKXK1L8xwQ7SergI9UG4hhq0kfWl9ecIHrS1QYNLvA2mkyZ4nL9iYHtB6IaUdYYTUAqgxvw2omgxZxzdZDeZYLtJZ+U590papDimnKdQYow7j5eh6226ZlVTzZh44JG2+Tdd9SKRql5X+sbsdFrD762Tu52T0gZOcvteSJe0n7USsHeMONEaVGru8hUu0kuarcsgWaDtemGtGgv.GcO9aEAWauV2DYawdItoLW+t9DjWir53AV6V+dVmpzukHW9xyPYcHzdp0cEYnbJZqDt634LJ33Hrna4i8JGzGdyD+NkvLx45NIZZ44JG2+TdtTTpq82Z5Ntt6AXddrNT+lpFT6oC98aZ5NtNkup70vlQAGGgMPz+mUEapdELJMOH82JX74f6QHcXr+g+J4yfsOcaNhoSHCk2pfMkihVluH8+.WMNGkSVNu2BDdAGJKiD5XoyX6wI8qRziAaplML1BsSV9l8KZ+K39816TIEOSvQr7P4bcdHNpyfi2dNW2IQSKOW439mxyKFM8YPRct+Vt1hz+NdtNT+lpFT6oC98aR4qJeMPsIe0myfjkGaOodSZc4GG6Ij41GkgqUb8.KCvmLcgVlMIrUP4nxx5RvIh60.luJ8+4F0BwRhByGirZ3yovgyP4DsiVWbFJuCE6aMCrQUcfY6ZJAbMSBVHCNmWd.LmbtN2gXt9mG6C7TlZh44JG2+Tdtj2py825sh60yfY345Q8apZPsmNX2uIkuZT9pQ4qQLArUQ4fQmYNXSIrz35w8nOML1BKVYrk6s6NhkWgzuH2tYXaefQKyqmz+3K52787SY4D1dEp7tfLTN6DcFaabJKmUBaQTJnb14LDSkAWyNgaLg22MF3L.9XdLd1XGwSVdcNIBNWOidzOKVo4klXdtxw8OkmWLZpyfj5d+sNbGwxRHelQKpeSC9T6oC18aR4qFkuZT9ZHiA32DphmGYaA95Ci6FBBN18rDroz2xQbbkorrlBtWrktE5uoGaT+JGk43xP4A1nXFTVWWJKiUitab+skxxJ7TB9IoZMsyVdrs70nuF8US38+Zac6yxJrcTuGGwyo3wxOpo3n9BNN1brdSplXdtxw8KkmWbZhCPRSn+VAeyqgOtybptT+lFro1SG762jxWU9ZfZU9ZV+1AFB3bwNmi.3UwVfvt6LTlWFvS0i+9wjgxNsltiqaF8YYr7.eAroYTzEaoeVq5neldrQ8nNtt0JCkGXKdQAlJ147b+XzXO1V6HW+djhX4.wZHKvYS0ZZms03tgtYjv666u0u6yUBZWivuq2G4KSuG+srbZr3KS2w0Mi9rLpZ44JG2uTdtjWZJ82ZGbbcyHmpK0uoAap8zA+9Mo7UkuFP4qsLVfeBsGQl2.26.Dow+AwOhoKkzOcRSiIg6QDaaR38e0ANUrsUnnkw8CrOdJNORGk+GLik49Eo7Nt939NFbOxxCC7.XSS3jZqvV0gCt+OGcupD2uFM14N4FjwxIo9Rz8yCK.XhI399qo8682jQ311ONZGwjq+YmubQNpugwNuRS6hYkuzTyyqy43fxyghOOunzjlAIMk9a8ViIN1qbp9T+l5eEYapp8zA69Mo70A67U0+mRn+OKG1VKWPksDZ+sZ3CSF2m69AGmkGqqQhq0kfgwV7EO9HGm.vYh8B8ch0.lq66chsugm101.Wd+NpmOaFKyoGo7dDR1nqNNruYp3d8qedM78A7RQtuYYZENZrj3WKT48n.GPFJyj3Rn6mCRxnjtsXMXLLVCw9zY6Hlxxo+wHw0Jc9vXSY7xVSMOe5QJq5PNNn77vJ577hRSY.RZR82Jt0y.e7A6cQ8aJ4Ji1TU6oC18aR4qCl4qp+OsUn4qqFcuHrb74P8D2nlNL1Vj4xlC0oKtVWBR6woCrQ4Tbtrz81I54jwxz0nCeOXea4wYW.tOG2unGKE3jn897cTiEa58N+H2uGrG2mj3DhIdVL14icdYlNpyyXDtOiF6bwaXrNvFcOVOqht.84isLs3rYD+6E5mQrOuzTyyqi43fxyCqHyyKRMgAHoo0eKWear2QNVepeSIWYzlZSu8zA89Mo70Ay7U0+m1Jr70o1pvCWY2I4yz7Zao2uwqnVbhbsh79aA9FNN9l.WHvMi6oZ+KgselmW9KQpurddbMAfWlteb7vXmNA6D1Vj0TwFI4euia6vXOG9eFye6tANXfsDXU.1BryAunuOK3HqeyYgWcnid73jeqZ+AK9PgOFo2KbpgtsmbNDSOaj34byg5Hvmi3edeqxw5MoZp440wbbP44gUj44Eo59.jzD6ukqus7QpC1Yk52TxTFso1zaOcPueSJecvLeU8+osbOecM.NeZOEXBe767ck0xZ4nth9h7xjS0cf3VWB1zDbemHvmG2S+9jth91uh9sf+Fj8U34vm2yo43kv91z2xLVNCC7emwGKqYBpird9LFmy0QccP831u2zNe6NH6uNF055Hd93dtNBLdrQj20y2uAY+a1JqZ5440obbP44gUj44Es55.jzT6uUbqmA6YNWupeSirxpM0lb6oC58aR4qCl4qp+OskK4qiFaQc4fw1lKWniJI3X9.eR7Wx55f8ANlSOpyfiYfsE9jWaCRtVWBdF5uuAmsjtm9TCS6UqWeJ794cvw1kwxb5NJyjd7Z.6XqxYHrsPqzVVWCYesbXRXmej8pd9tYrNhymvQc84h41tuz9bBeNXI491A4Hd1POWGiF6alz0VyV3ioR4tUl0zyymtixqpliCJOOrhHOurTWFfD0eKiq0yfEikOmmT+lFYkUapeBG0Scu8zpR+lT95fY9p5+SadKec0wVfU96z843URNdUro5zJjh5dEvVvglaJp2gwV4e+GX6VD+U58HV0ObstDbIonb95NJmKySwXXqLc+sNchYrLGBaZ81uul7J.SKRYME5bUZNoGOHt2plRi6XDpqz75aRLA5982yHxsYcnyyoy4gs0WkGNuHwxSlwxauA9iXeiGO.vSP+8Z8hwFThG.XVXmKiaeFiojpommW2xwAkmGv244CRpxCPh5uU2bsdFb6dpr6E0uojoLZSst2dZUteSJecvMeU8+w3s70OB8+KntNR51hYX6qmp6fieVJhAWbstDbzonb1SGkyC6oXLpaMR8bkdnLWMR1BRT3D83dC+GC2eS6wcbgXMH5K6L8dE6eldrthZmn6NCe4Xm2hWJVmdCt9GAaJLlWdrHwwYmwx6RI4ullziuXFiojR440qbbP44AdrHwQVyyGjTkGfD0eqt4ZVr7s8TYORT+lFYkUap041Sqx8aR4qCt4qp+OlGKRbj570sA67neIXSYnmGazVdXrNhbmXiP4rwdP8TXKDLuBseg39IceiFaB1SZKEa5k9ZXipz+DaqIZ1sp6au0wrvdy4iiM5oAwvRa86GbJhgnhacIXyRQY8gcTNufGhQWNrH0yqisMAlUqAibiGyC3yvHO8ee2XeyY8prlC1nqmG9.XeCZuH166lA1Jg8vj8E7oQxthkWE2i62.aKjcUxwXXibTuaYFKySFqsikfkK9xXKNROAVaH2G1213sgM512M125w+.605W.68pAMr9rjtN+2uTdda0obbP444Qd9fjp7.jn9a0o3VOC1cOT1Ig52TxTVsoVWaOsp1uIkuN3mup9+Tu6+Sox05RvyR5VA4+FNJq43mvrKq.ct2WOLv96oxdLXIymCvUgsKdbQXiX8tQ+s2mONr896uNVh6sfs.zc1.eHx+EDtnBVsvuqBntVdr2ecpXi78Mf8b5wfM8yxa+6z46Ot6BnNGTo77NUmywAkmWmTkGfDoSwsdFjlA.JMT+lRuhpMU0d5fCkulLCZ4qp+OhW7so6F.tzTVVQ2GlGF6aHJuDcEY9xyw5pt3rvdt5ZJ6.o.La578Ge1xMbJUJOuYQ440GZ.RpObsdFbaEbLn1SSmlRap081S6GJesZpojqBJeMWc6zcC.GSJJmIfMchhVV4wh2XfoFotlO4+JKcUWv4BpuNepGTsEz46MVH14dYSkxyaVTdd8gFfj5CWqmAmdAGCp8zzoIzlZSn8z9gxWqlZB4pfmxWGkmCp5hIA7tbb8WeJJqoh8gmh5QRQYkTyD3uE5xiGX+xw5qNXKZ8y6oTih72AF4xWA1oTRSjxyadTdd8wRC86KozhBIq1DfI635mQAGGp8zzoIzlZSn8zjR4qUWMgbUP4q4p8ftGczmizstDbRNJqgA925w8Yk.lXJpqvNpH02ChFPr3r0z94o2aIGK4oIgsfME98EaaoFQkKkm2rn775kql1O9NsRNVjz6Hn61MWL14vdQSsm1eZBso1TZOMoT9Z0TSHWET9Zty05RvuNkkUzyStfi3Foy8D6aFa1or9BLNru85v04GNikYc0Yh87yKi87Vc0+Ac99geW4FNkNkm2rn775kOFvSisSArUkbrHo2ufta27uTRwhZOs+zDZSsozdZRo70polPtJn70b2cR2M.7YRYY8ycTVCisOU6xM15uOqTVegc.Qpya2CkYcyDv1dCGlhaObuLr7XakjAuWXo.uyRMhJeJOu4P44hL3YT.ykta27aVhwjZOMYZBsop1S6jxWqlZB4pfxWycaB1SpQa.XK50cpG9lNJqgw8de8tE5ueRor9BaHfaMR8tadnbqSB1JnV.vZWxwRd5Doy2G7SJ2vozo77lEkmKxfGWay5CiMC6JKp8zjoIzlpZOsSJesZpIjqBJe0qVCrQe7n.Ndfe.tGczgAtjV2lCA3fAlRBqisOlx6eOxs6sR6Q95QH6qMAAdGXqfuA06C6wxtpaM.dErmW9wkbrjmVa57bxat.qboFQEKkm2ro7bQJeaMv+OfiF3KBbg.uJta27pvF73iB3SRweNjq1yOtjD3..PFelDQAQU6slPapM81SU9Z8PSHWET9p2cd3NYOIG8ydt822w8eN.aCvF.brztgmWEX5Y5QU2N0H0sVP6rQc9xwd9XI.ab4FN4peKc95eSak9V44MWJOWjx2xf6YrWROV.vxUvwrZO0slRapM41SU9Z8PSIWEZ14q4hSC6IxWCazl96XaaT2MveE3t.tWf6G3ww9leW.1J17w1m00WgQtAmmlzOE+6kwgcd4ETOKBaDWax95z94iypjik7z9SmuG6hJ2voTn77lKkmKR4aB.OJVaiK.3kvF73GA39vZC9tv19Imcqa6yf0l8RwZuNM6tXYgZO0slPapM81SU9Z8PSHWET9ZsvFfsB69mA9GXSIr4i0.y2h78bCaC.lGseCzsi0HXSzAQ6mGtKr8O85nUGau+N3w58SwOp9MQJOevfxyEQxB0dZmZBsop1SqtT9ZaMgbUP4qhmr6XSypf2HcwT7ix6fffyGuGm56BVz3o8NkxvXeC.aZoFQRQQ44FkmKhjUp8z1p6sop1Sq9T9potmqBJeU7rifNmJRe0xMbJEWHvUCrtkcfjSFBaJlE7Z7BA1wRMhjhlxyUdtHhen1SM041TU6o0GJesdmqBJeUxImBc13wAUtgi3YgWrpVL14mmz7n775MkmKRwQsmVuo1SqWT9Z8lxWkbyWlNG4MsugWO7ooyEoJsRN2ro775IkmKRwSsmVOo1SqmT9Z8jxWkb2gi8lqgAdxRNVjraJz9bu7k.14xMbjADJOudQ44hTdT6o0Kp8z5MkuVun7UovLUZukcIUaqJviAbsXMhHR.kmWen7bQJWp8z5C0dZ8mxWqOT9pHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHR9ZnV+7OBr1kYfHhHhHhHhHhHkfsB3UGSqKLEf0qDCFQDQDQDQDQDoLLZ.FUYGEhHhHhHhHhHRYK3TrYLg9cQDQDQDQDQDooXQkc.HhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHhHkoQW1AfHhH0NKCvGB3sB7H.KobCmFscAXcAdrRNNjrYT.6GVt0SURwfxqEQDQDQDQ5SmNvvsNNoRNVZpVdfKA60f4CL9xMbjL5cg8Z4RANCJmufKemWOZfcC3r.tQfG.3Y.tefqB3G.bB.uYOTWhHhHhHhHkhKm1ePpKnjikln0BXVz90fea4FNhGLQf6k1uldkXynihjOyq2Vf6JT40qikh83cWAFJi0qHhHhHhHRg5Fo8Gt4JJ4XooYc.dHZ+7+MALgRMhDeYx.yl1u1d8.KWAV+9Hudk.9wXC5QPY8B.+JrYRxEC7vD+fkbc.qVpeDHhHhHhHhTvz.jTNVM5bvQdXfUsTiHw2Vef4P6Wi+S.iqfp6rlWODveHTY7T.6OtG.uUAafTbMHIONvFkh5WDQDYDMlxN.DQDQRo2LvggcpFrHfEfsda7Fs947ac8iBXrXePxfiwG52GKvqCbxstu8xP.GCvlDpNccrjV0wDbb7B.mYqe5KSD32ALkVWddXquCOmGqCo78X.6IvLvde+6C37AN.rAOXP1WDXma86+QrX9Yi4197.eZrGm+O.Kan+15BboXmlNiT9pHhHhHhHRopnlAI+Hhep3mli8OA04l6o55CmgG2t7yCU1KFXZdt7kAK6Kc99ohXwPNK40aE1.JNLvChMfdI0Qh6bnudeFChHhHinQU1AfHhHRJ8hdrrFF3umfa27wV+DxZc8vYrLB6Hv1BXC7s.tAOV9xfmKkNWnTOUf2eIEKIwGk1yZ4+OrcYoj5GB72bb8GJZ80QDQDQDQjAbE0LHYb.aA1GLbuA9TXenw2fjMKNlMvWC3Cgs.XlTuiV00eIA0wRwd93LwFHiO.9cQl7chMnMgeLosz2lgUD3In8q8yEas6Hujk75YRm4EyCK2Mo9H3N+Ze5y3PDQDQDQDoPU1KRqaG1Z.RuF3hCgrukgNV5bqWM7wyA7EvVuDxKiF31CUmKAXp4X8IYyZ.brXmNLaumJyckNee2E4ox0kzlWOd5bP7BN9E8QYr9Nt+CCbh8QYHhHhHhHhT3J6AHArsKz3FbjEBrBdpd9yQJ6k.bF.SxSkeubbQp6ueATmR5roXyvivyrnc0SkczYmwd3oxMpzlWuN3NO7O2GkwP.ulix3b5ixPDQDQDQDovMHL.IaKwO.ICisSzjUuarED0vmhC6jGJ2jXsoyOv37AVqBptk92kQ2uG7r7TYuOQJ2Gg7Yq+MK40OIc+3+76yx34cTF+z9rLDQDQDQDQJTCBCPB.2AwO.IylrcJ1LYfGMT48X.aTFJu90OfNe778Jv5V5OqLtOku9Zdp7GEvCEor+LdprCKK40+aXyZlvmBZaVeb+WEbmGex8YbHhHhHhHhTnFTFfjCl3GfjgwVvTSiUD3tBUNON1ZjPQYCwNMgBp+EfcZLHCl1Kb+9u8xi0wQDoreV72oQVfrlW+d.9rXKJx86LbY6w8yg6WutShHhHhHhHksAkAHYBXeS0wM.IWdJJykgNW2QdZfo3ifsObdz4iieTAW+R+4Ln626sTrYVhuLQrAEIbcb7dr7gxMu9rn6mCWH1oZlHhHhHhHx.qAkAHAfSi3GfjkP+MyOFGvUE59OOf2tGi0jXEo6sw3ssfiAo+DdmFJ3XV4P87UhTG2umK+xJudkAdU594vKn.iAQDQDQDQjTYPZ.RVO5bgTM5w2Lgkynv1VRCteuJv136fMANb5L9enRHFjjaEv86+9t4PcsZ.udj5YZdr7Kq75Skte9a9.usBLFDQDQDQDQRkAoAHAfeCwO.IOG1ohyH4bBceV.oe8KIqtU5L90hT4fscE2uuauyo56BhTO9bVVTF40uSr7snO+cjET8KhHhHhHhjICZCPxNR7CPxvXKlq8R3SSmECru4Vj1aaJcG6aXIEKRx35T7x2q+HgEcgI90wNsr7ghNudB.2Kc+724U.0sHhHhHhHhWLnM.I.72H9AH416w86KD419ox2vrmhtXeN6RLVjj4lo62u8Wyw5aJNpuixSkcQlWODvES2OVNKx11ysHhHhHhHRgZPb.RNJ58rHw0Bc5gF41bBERj51P.yIR7b1kX7HirkkN2NlCN9N4b89zQpuazSkaQlW+Mo6m29B4bcJhHhHhHh3cChCPxxC7xD+.jbQQt8eDrc4lf+9oWXQpaaFcGy+qkZDIijcB2uWaux458WEo9V.Iac1YjTT40eC5L9WDvGOGqOQDQDQDQjbyf3.j.v+EwO.IK.XxstceP57a9+bK7HsaGAcuNVrp4X8MD1N2SQaHf2BvtCr1kP86SeU598YKA+slfDmi0Q8tCdnbKh75SmNi6mGXmyo5RDQDoKiorC.QDQjBx2G3nw8ZXv3vNkZ9S.WJvXac8WZqqurEc6Z8Av1AdxCSG3GB7lv9v1+vbpdhZiANefst0kGF649ebAU+8i0DamnYxDeeo9HNttQA74i41u.fqG3FxXr45TpYZsJ6AYmAvwE5xyB3CA7nkS3HhHhHhHhjcCpyfD.tZ58ZQxKE52uRrANYPvSRmw4kjS0ypB7Lgpmh5CUuJ.+C590iEP9NSYRqyld+9nzd7hdH1FMc993gwdeeVkm40mEcFu+Bfkwy0gHhHxHpLl9rhHhHkku2H72WgV+7FA1GrS0lx1FR2mtI2WNUWmBvpE5xWSNUOQseXyXknFGtmIFks7ZmT4o7PYrDrcOmv1VFLm0vCA7eC7YZc4gA9R.eTrsn33bJ.WE1ohkHhHh2LH9OKEQDQxKWAviAr9831bG.6AvaT.wSRrUNtt7X.Rda.GVnKOKrcSjhvT5weaiKnXne7EAtLhewO8z.1jHW2ho8h+qKOO9aqa9V.1kPWd4ZEO2imJeeXHrS6sin0keCfChjM6n9nXuu3tA9+xknSDQDQDQDwCFjOEa.33o2mlC6R720RwIR2w3lkC0yuOT4Oef2dNTGw4iQ7udb9EXb3K2Bc+3XlEX8e3Np+rt643y75g.NmPk2b.dOI79tBXKRwCC7IxXbHhHhzAcJ1HhHRSy4RumcH6SQEHIzF335limqicAXWCc4uBE6rM3mC7Gi4ukWmNK4kkGXKbb8yn.ig4435V+Br96kgvV3eCV7iuefsA3VS38eGo86I9a9MzDQDooSCPhHhHMMyC6CjGmC.6C4Nnv0.j7RdtN9Rg986F3a64xejrTrSah+ti+VbmRJCp1dbeJLOiBLFbM.ItdeTQaTXCP4mt0kuQfohcZukDKG1NQU.ecJIIhHh.n0fDQDQZdFM89CKtb.GH1NUxffnw5qCrHOV9aLv1E5xGF15kQQadXactyhN+BblaIDKYw6yw0sHf+bAFCChyfjQAbdXqyHAVOrSGo3LDV95XwFzxIQ6YOxS.7J9OLEQDQDQDQ7mA40fjg.9ILxa2pCJKlkCgsS5DN19mdtNN8Pk8u2ykcZ7GoyGuepxMb5a2Fc+9ohbvQ.avPhFC+0LVlYIudz.WniXJKGWYldzHhHh3fNEaDQDoI4aCbvI31sY.SKmikjXhXe64g8hdr7GCc9M5eZdrrSqnKJqUoSihIA7tcb8ynfiim2w0sREbLD1OAag30mz5OhHhHdmFfDQDQZJNIfiq0uOLvgPuGrgiJ2inQ1x335745Oxd.L4V+9MCbCYnr1RfOGtOES5GWWneeQj8Y9PQZZ3tuUynfiiWgtOMob8dohvpQmCBmunAHQDQDQDQjAdChmhMGIcN87CVnGOSheJ7uPf0rvizNsdjumZA+xPk6dlgxYm.VPnxZSyPYsrgJmjtylLn3Lv86iJiAmXtQhiWMikWZyqWI578F95Xay3iGQDQDQDQjb2f1.jr+X6DJAwzoD5uMErcPk39PXe4BMR61lP2wzE6wxO3CQeujssS2YRmwXV1pjeSgJmuaFJmxvcR4u9iDX1Qhirta.MnkWKhHh3c5TrQDQj5rcCaMsH3+2c1z4fd7v.WUOt+GJk6N91DbbcurmJ6MAX0a86+uXev2z3MQ2ea9OUZCJrYMSfaNCkSQak.dmNt9YTvwQfnm9XuHpeehHhH8j9GkhHhTWs8.WBsWjS+E.Giia22qGkwZQ1lMDYkqu0+nKZqo0ND52ulLTNamiq6AyP4Edwwsrl8EoQbq+HWmiqqHDcf8V4RIJDQDQpPz.jHhHRcz6B32gsKv.vUisPQtTG21qB3u2ix5n6weKu85NttI4oxNXfHdNx1Bg5Tib4YA7BYn79.s948A7DYnbJZS2w0sHrS+nxPz2mrPb+9eQDQDoEM.IhHhT27V.9Cz9CH9WvlEHKLla+RwN0ahy1C7N7Vz0ebM.IqfmJ6fAH4tH8mdMP2KHqWcFJqIS6AbopsNWLcGW2sh6WCKBQGfjxJNDQDQpLz.jHhHRcx5f8AzCVaMtafcE30Fg624Qu+.jk0rHIulAIiGXsa86OcFKq2RjKe4YnrNDZeJDUkFfjUB2Ch1LJ33HrULxk0.jHhHhLBz.jHhHRcwphM3HAKxmODvNSxNcOdAfKrG+8OF1GBtnkWCPxjC86yICkyXwVmVB7Dj9Sojw.bXs984Q0ZAZMt0ejYTvwQfI.LtHWmFfDQDQjQfFfDQDQpCVdfqDamYAfGGXmv1FaSpueO9aKCvmLcgVlrPfmOx04iAHYMB86Y4zqI5fFcwYn7NTrY.DXydjrtszVjlliqagL3r9i.YafvDQDQZDz.jHhHRU2DvNsN1xVWdtXCNxi2mkycCbC83uejTN+eyGMxk88.jrlYnbhdZkbtorbVIfSMzkunTVNkEWCPRRW+Q1Xfy.aVJ4KQO8ZfteejHhHhDgFfDQDQpxFC1rV3805xu.1oUyCkxxqWa4uaH15YRQ6whb4wS2m9D8qvCPx5lxxX0.9oQttQmxx5z.VkV+9SAbsorbJCKOvl635mQBu+mMvwA748U.g6AQSCPhHhHxHPCPhHhHUUCgMiE9WZc4WEa.Lt6LTlWF1GPONGSFJ6zx0Grcsbbc8iwG52mJ152R+Xz.+LZuPuFXORQrbfXmdMANapVmdMaMtGXnYjv666u0ueS9Jfv85kiFfDQDQDQDQjB1MhsNTLL42NQxXA9Igpm2f1yhjr5+HT4F8Xo.uYOUOI0Q5HN9fYrL2uHk2w0G22fYsiqmed.rS4ojZqvNMTBt+OGtO8P5GiF3sBrAYrbRpuDc+7vB.lXBtu+ZZ+9pMYDts8ii1QLsCYrLKh7ZQDQDQDQjZk79CRsbX6VMA0wRn8rHwGlL1GvMtAI4r7XckDueGwvmMik4ziTdOBIaVjLNrYYSbO2zOO+79.doH22iMoO.bXzXCVwqEp7dTfCHCkYRbIz8yAIY1frsXCLxvXC3jOc1NhoI2y6wHSCPhHhHhHhHReJO+fTqFvsQmevui2y0AXKRnwM..uHvxlC0YbVVfEEIFNmLVluU59w08.r5839rK.2mi6mqYYyIgMKebYrXmpRyOx86A6w8IINgXhmEC7gyP4NRloi57LFg6ynA9qzd1l7V7bLc8z8ysYkFfDQDQDQDQj9Td8AolJ1GzK7G76NwVKR7sskdOH.YYlNjF+kH0eVWuJl.vKS2OtdXrcSlcBagbcp.GNvu2wscXrAq5+Ll+1cCbvX6tPqBvVfsViD80vfirNKfdtXJ2gw1Qixq0csq0Q8cHiv84TCcaO4bHld1HwSZ2cgBSCPhHhHhHhHRex2ePp0.37o8oiP3iemGJeWVKG0UzOv8xjS0sKeqH0+aP12IaBuFtjliWBXivF.jrTNCC7emwGKqYBpirttsDmy0QccP831u2z98x2AY+0wnVWGwyG2CkqFfDQDQDQDQj9TV+fTiFaA17fAtRfER7en24C7IIamZFgsNXa2pyoG0YvwL.dOj9s119wd4n92tLVlS2QYlziWCXGaUNCAbcYnrtFrE90rXRXqEM8pd9tYrNhymvQc84h41tuzd8sYNj9sX4d4fbDOanGJWM.IhHhHhHhH8oz7AoVcrE6x+NcudajjiWE6zNYERQ7tB.+Ff4lh5cXrcgk+Av8istRzqYOPZsxz8Ln4DyXYND1onT+938U.lVjxZJz4tQSROdPbukzlF2wHTWWhmpmnl.c+dmYD41rNz4N+y7v1heyCmWjX4I8T4pAHQDQDQDQDoOklOH0Gg9+CW65XaRQ7tudptCN9YoHFRhaMR8bkdnLWMR1BuZ3AzHtOX+GitW3U60wEhMvO9xNSu28gloGqqn1I5df8tbr0mkKEa.7Bt9GAaQxMu7XQhiy1SkqFfDQDQDQDQj9TZ9fTaCvyicZR7Zs98mDaQC8dwloCyBX1Xe.ymBaQ47Un8GJ99IcyfjMA6CvtTrSmmWC6a3+ehsMwN6V08s25XVXCTviC7Lghgk152O3TDCIwgQ2ybkkyCk6ZvHOHIyC3yvHepL8twlEP8prlC15vQd3CfMafdQrWSmA1N9yvj8E11Qxth8d13db+F.mI1BVadYibTuaomJaM.IhHhT6k0y4WQDQDe3VHe+fi8xrwOCzPd6mi8ArCVbXmHvd155yh4.7NA1CrOj+5gsld7HXa8u2M1ru3ESPYcmXC3z6EavJlJ1ofxyB7DXq2H+ArA2IObMsNBKX8.IueM9JwdrOMrYYyVgsEMOarA46xvemtKw4iF4x2C1f5IhHhHhHhHk.8MMmehtyyb4ka3TIbVXOWEcfSpilMc99iOqGKakWKhHRs2nJ6.PDQDQRrebjKuKXy1CIdA6TLOaoFE4usfNWaSVD157hHhHhjPZ.RDQDQpNlIveKzkGOv9URwRUwVz5m2SoFE4uCLxkuBp+CJjHhHhWoAHQDQDoZI5tRxmC8+yiyVislp.vMTlARNaR.ehHW22pDhCQDQjJM0gJQDQjpkeD1tqSf2L1VUrzsfEszWA31Jy.Imcrz4oZ0U.bykTrHhHhHhHhHsnEyw72APmKFmZmJoaS.aqZdXfucIGK4okGaawN38BKEaWIx2TdsHhHhHhHReRePp72P.2JcNHI6VoFQCd92wddYA.qcIGK4oSjNeevOImpGkWKhHhHhHhzmzGjpX7N.VHset9gAlXoFQCNVCrSqlgo6c9m5j0F3Eo86AlKvJmS0kxqEQDo1SqAIhHhHUS2MvoE5xaDvWtjhkAICAbN.KG1oaRcdwJ8GPmq8HeFf4URwhHhHhHhHhDg9llKNiCa8GI346EgMyRZx95z94iypjik7z9SmmZMWTNWeJuVDQDQDQDoOoOHUwZCvl0.gWvVmPoFQkmCh1OObW.iubCmbypC7rz9w58iMiYxSJuVDQDQDQDoOoOHUwa2AVBsed+hwNUSZZBV2QdbpuKLqimNywdIfMs.pWkWKhHhHhHhzmzGjpbbDz4obwWsbCmRwEBb0.qaYGH4jgvNUZBdMdg.6XAU2JuVDQDQDQDoOoOHU44TnyAI4fJ2vQ7rSk1u1tXr0gjhhxqEQDQDQDQ5S+IZ+AotrRNVZh9xz4LLX2J2vQ7jOMctX7teEb8q7ZQDQDQDQj9T3SAfytjiklpCG6CQOLvSVxwhjcSg1qwLuDvNWBwfxqEQDQDQDQ5S6NviB7v.SubCkFsohsStbWkcfHY1pB7X.WK1fkTFTdsHhHhHhHhHhHhHhT28+WaD.9yJkaWC.....PRE4DQtJDXBB" ],
					"embed" : 1,
					"forceaspect" : 1,
					"id" : "obj-53",
					"maxclass" : "fpic",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "jit_matrix" ],
					"patching_rect" : [ 9.0, 179.0, 248.0, 38.919708029197082 ],
					"pic" : "GrD2.png"
				}

			}
, 			{
				"box" : 				{
					"autofit" : 1,
					"data" : [ 7376, "png", "IBkSG0fBZn....PCIgDQRA..APC...flHX....vIfSas....DLmPIQEBHf.B7g.YHB..bbXRDEDU3wY6clGtbTUk.+2KuvKAHaOHABKgsfrEQXTPDBDdHfxNn3.HHHn.JtgiK3BNFATGDW.2XTFUFTPAmAT.Uj8GCBgcCKZPBDBBHfX.RBgPRHud9iSWVUeqS0ccqtV598N+99teu9UccumS0c025dO2y8b5ECiNCd6.SCXgUrdXTLLJfiBXs.d5JVWLLJLFOv+KPMfWEXLUq5XTPriHeGODv2Dn2pUcLLxe1Pf6G4F8Z.WU0pNFEHqIvCQ320WCxn0LLFVvFCLeBuA+O.L1JUiLJZVef4Q3242Bv3pTMxnsXh.aIvtAbH.GGvAhXegxlI.7tA11JP1SgF6L6QAlbEnGFkOaFvyR3282DPeUoBYzZ5CX1.WAvb.dbfkS3WhtkCnBzwYWW1ObIK20D3NH7ZeQ.aUIqCFUK6LvxH7dfeAPOUpFYzT1aRtyKsxNVA53cVW12PIK2eAgW2uFvrJY4azYvgSi+F3zqV0wnYrOj9NyVDk+TNm.RmI0.NiRTtmBMds+eThx1nyiKhv6EVMvasZUGilw1Ar6.6Gv6gPWSvs7qq.c6.hH+x5lnc.wsLBj67vbQiQ5LIfmjv6IdNf0sR0HiTyzQuCsOdEnKmScYuBDaZUzzKv8PiOMd2JA4ZjuLUfOFxzC2ibpM2eZ72CWRN0tFELGL5cn8uTA5xcQnqRTF7Inwq4ueIIWi7iYfLBpfuCGBoyn7famFu+3fxo10n.4aP7NydApV6m8UJA4sQz3JZ8pHNTqQ2E+Jhe+64kSs86zocW.lqbzwy8R7aHtxJPONvHx+sUBx6+jFul+dkfLMxWVGDyD3d+6WNmZ+QQi9kXMfSMmZaiBf9Q+Fh+sJPW950k8p.V6BVVaAvJI75cEH6P.itKNDzMWxgjixvcEvedjYSXzAxgh9MDuwJPWt65x9NJAYcgz306+UIHSi7muIwu2cHjQtkWrlHchEUFe5br8MxQNWheCwKR4a+rIRn8yN6BVVSh36NhcsfkoQwPzUnNnb+Efb9RNxnr2EKFoj4R7aHphHKwAEQ940JTkDePZ75c9Er7LJFhtHRQKe6BPVSA3UbjiW6jjpXSQORi0E3Mnb7AKY8.fAp+2UCbaErrdeN++OqfkmQwvLQO1kcyEfrddDGPOJmXAHGi1.2kjNn7lp.cIXpC2cAKmYP7q2snfkoQwvYSwa+rnbBNx5UPLegQGBeGheCwKQ4O53IQ3Js9MJXY4ZD44Uvxyn3XND+92+XAJusTQde3BTdFdxCR7uft5JPOhtSEN3BTN8Piw6pZ.meAJOihi0lFc6lfx4Vvx8Ybj2sl1JZ1PqXYJHS+xkAKY8.Bse1P3wMHYfYfDYRixMUfxyn3XWAVCkiWD1OKJtaIu2LoLZFacnkc1PjNI1OjgIqY3zAPOv0cKElV0HqMxMCGAxND.fkBr8.qWAIS2MsbMJ1Nv6EIp6dPH+.rpiVCaCxm0Sqh0i7f8T4XCA7+Uvx08At8ArKoohiN+0kgsLJjop8QP9vc7Nu+xPVJ6uNhMxfvQEEkESwYCh9ANRjTB21iXHd2NTmHg2P9hHaL3eNx1vZY4fN3tL6+Ef+QNzttr9HFr9HowHFxpQbIlSDYuxlElJhyPut.WOoaQT1DjOGmYD83SQ9seGya1.fCC4ywj5G3HTN1nPttzXEHOrtc6vSaFDyhxaf.C64DAdLzWsR2szzBPt4Ff+rx4+axYcaTH+36xQtgRSGSSYYHaV81cSA+TNsq6xvmGb.HOzHpbbs0ybIaiVaSQB5lAsyqQqcH3wRiYupn0caxfNTFb9j86UZV4kn8oWjG7GscutbncGwyTPdZu6WZOLxSu1.jQ.MCfeYj2+w.1Zk5Uije5VVX.f6SQFuLvE.brHAZxKfvezOIjPVzIidDT3gPh46YgsPo8x6Hh69RXfhb0.mERrlqWfK1Q1yEca.0LtUheM7EaQcNLk5TVFPOq3Fz.xqxeJmzueuS6tTrYT1VLKhuZKqFI4hjznXhFRgc2WZAkr1YQT1Bz6L5IANMh62NOb82WyYZ2Ojj3h6HKxxFm+HUzIsosjUlFgO49EQlZcThFIdCJ+qdz9anR8qgDTCaFelDpWMx+QjmWLQjnsxgjPQa1EqB3czj5LSxO+S6KoH+sOmZ6QbrGHixI5GlCA79aQ8FCwCVcQKKl1OiQePDe5VCgX6NsNZ2fHmWRw+r0Bw9Otsout2wmi3WyudOailQzmZeTJuulSL+C7n8eOJ0uFxBqzLZVRwIuFwRYSzLyUP41KQ46t04pQ9FcOFwvthL7V2OL+nor96gRcCJ+11Pu5AYpOC4zlOKMOtlcTQN28sIm23H9H0VL9YCnfo1FsjW4ayn6C0eUBmyWPQ997Y9OVo9ocpNmmRcqgrnHcaLdjQi4ds7UKQc3HTjeqForgCSm3FirFRpWyGzFtdMxd3PYTDeDT0PrUWqh9q+f5m6JQFIVyPKaU4y1j55Upuu1vJIBBa3KEYUH03JUj+k6gLVfR8u1TV2dPOQ3bmdH+NEzl5dMJm.BZ.Z2K1oZOxNR5E8s4whvee1JHIj36TWRBssO0eizs+Hal8yz3RUjUp7AHjLfdz5kGtABHiZNnM+tIbN8i9p7l1QUroJ0sF9kuHmJvRbpeQrJuEMAAAznkURwGPPixaTQGRZj4FJLazug9DxPao0AzRHa1O6yqzVuD56.AWhZ+rzFtj+PJx6+NE0qGh65D+sTJyVQzoxt0IbNZ1boFxJPlFduIT+c2Sc8K5T+xbZZ4EAAAznkhN5r3xlonCE4dHcXE6.51L3wIacBoEP79cYncbSwWAkVs3DAD09Y6SJqyrTj2xo01QZsTp2eNkxrYLJBy1PM6F5aSQ9KlTtkYP5z1s9uB96Wd6lSajmqxaYvDQObwWFITmnLdEcXgkrNz0xUfdGGe1LzVQipEQKmlmsyZgtMctAOZCereV.qihLqQqmd6jUpyb7PWShsKR684R3bzhPC0.9QdHmEpTee9rNfwSiKbylkg1nJIozsXyVPohB2AY72q.cnqism3qbXs5GKK6Gujxe.o0NTAnEGpdUfM2i13uTud9l+MccKjZz5Lr9lpTmqwS4pwzPttWBIu.HmghrqQ5SFtaVB0+KjQc9uUu9OWFqeUhV9CvmGHlmDMOfVCwUpLZAWF52Lm0QWns78KA+7x4MC8o.egdzFYw9YftsvpAb7sndaqRctTOjayXLz7UK0MEnUCYEfSKGuR8qg+1OCjO+Bb6mpHLq2tnsySJa6mEv7bziU2pJLReqDzOvgmv6kEadAvdobraCw66SKGG5e23yFcdfHudPOpW+n24QqzeMaUsDOjayXEM481Yjob5xO0i1e.kisbDWEwW1bDe5iLV+pj9QrmrKCVx5Q.t6Kzf.i5PIUgQ5gOn8ijM5eVbB1Ii91yXPOZidPVwMWtE7KS6LP8+tR7yCuSZJsKrE0S6om4kOn0LRxn6WhGsw.JG61Q9ryWhNptaLC0uJYVn2mvMW1JRcbendK2VUiz6P6fR33OKYaIhGf1O9mMSzM.uu9fy.0+6cirZcok8LgiuvVTOMYLQOjaVQysLtWDehKMr4H1+ykAyn9r20+6KP22HzFP4XqhxcKOEE26eVIMYzYvH6NzFExHzz3ZQlytunMcyWFwMNRKIk7g8Iputg.aU8W66SW0L9+xo09TlVGZEcludFnOcyKyi1XfDN9f9pL0I3yuqmTXymNLFP4X2E98.w7D2NzZodLRtCsoRxCgMqOQJOrel1T9ddjv5SZI5nrFzi5MFzyChWEs3IiTMiPKIml0mQyp8cVVse1L.135u92mg5WkzOcNoaw.biZLVGZMglsGHuiLzdSEYk9bYPOamMS4XAIp3zx.0+qu1O6nIdj3EjvhTqnJ5PSauENOR+zMg709YAgpnZz80gVR1OavRVOBXrD2olsNzZBI0g1KS1B4KZOoG7OrAqswq80gBCzk6BYzFokSU4XOCoKZgtRj88ZTJxNzFMvNobbehroqG59Z3fYQgHbAJtSD6v1Mg1Hy88Ah4IZ26zxOSGI2g1FjvwuGxlsOz5PaYna+rIfDoU0x2ftcJ.90g1FB75p+5A8nduUzWx9uFo+yiG24+KxNz1Azc1Sehm86XBGePu0FY0sCFgtOtLRmBZcnkV6ms0HNj6wji5iVxE189qXLRtCsjlB2Cjw1KI6msJki+NQ7B8uKwcajEpb9sx9UQYfHudvTVm0.8Pyx8A787P1Kz4+GCseNJHIdcIbbeVcZsX81pQVkTeIHHFrBxOGJtrX7HgkcWFLk0+7A9DjugWdsGFZcn0DdhDN97yPaMMzWssAS37Of5+cADezOZeoMEOzkAp+Weltvmg3FDdHjHXgOiVUS2aU7ZKqn8YxRRPGRBsEE5IwuooChiz99p+5eCR3AuahcAc+wbvTV2fU102sXWynekiYcn0DR5CmmICsURt+gl8y5kvM5qV55RKf.psXCIw.0+aZse1LPeOKdV3WvcDzGcYQk0izV7Besa03TNVVr806kvQTjlEPoUzKxma9rucaGdKJGKsOP7yT+u0PFoVdg1Hv84gUi3XLnuozeWd1N8gLMU214UP2S4mYjy4XSnMeHm1ZHhmMx0XihTmyLEm+zPFopqt+iSQc03spzVYIYqjFNAEY4yNo.zSka9tCQ5iv.a47n8xYD8h7vkkEQedbjUetHQKR6llQasqD9an7dZ1Ze2jleCLhFsNh9PdT+dPL.r1FaNoaHB175qlji29ZAYwzrAyO5HmeqhNFSl3a92ZHSYJq6w20l3ap9eXFaqVw.DW2SxLBZLNjEawsM7MjY+YiT21M1mcZJ5SMD+XzmrWkunkXe9lsnN8hXuxZH1MbqZ9o6M2hi97H4b6OrjSh3eQ5SLzJoDiQRixYrHaIlZnOcy.FGxWft2TOylTGHLxttBZLah6xVPnusEsbYsndog6zoMyS6pDkniFMZYSSY80RnJ0PbhYezgfrC1bQeau4C+iDzoZ.+UJNSDciJx6jZQcNyHmaqxaoYA2z.YVm0vHJVSh+A2hHcap5jhAWAkKPoNQSSZs5FlYP7To2iSy2NQAgQml45BGMwSDLqlPagzt3FO5WNE2JcFrE0hVzbEFW1NzyrWAEMaJ4x3HLTUuRjHUa6Pzv8TRkjrUa6hV1t53Zx4eXDNUy6k7+62oonOu2bVFCa4zI9GdmRSN+wQiIT3mB8Drq6dubcPdJaMjeLoYTaWNBhamu4f9VToU1OaaPOyQ8Lju+P4PTjQVhqXogATj0SRy8+s9ow3m1UqzF2bKj6XnwNS+HY8BHBIE5qiV914fbz33Uj0mLgy8vILYz7rjsffZq33TzmzjPfLPllvESie3sRBiZBALJjnyQzeLLeBcWiaxoMVDg+vZzzXJVymP689hbiSz1dUH9MVze3dLQd+.6m0CRtD32R7NFWNRbhWak9ZGVGEYkTnyNOPaZ+2G5qP3tPi1M76iXKH2oIWC3mg9ng2GByjV0Hech16UQOhVJpLH0XIdzgcPmyYiowLB1Kf+Qg4zxE5nKOUAImgsLZh+j5gPb8guFxSFeLm2+xAV2HswzQFsSzy4dQrIWziem3+PzWejvYs6M3qptL9NznQTu.Daz4lR0B5H6hH81ZJKDjCMCJ4Qn3tYnkksdMjMU+Ygj4ktl5GK38+eHbEI6WQmqA7zHwUsyD4yz61o8+xjuw7s2F5ohufRQtMj1GhufNWIxC8tbZz7GKfhycbf342g7zcPFwPeHNRpVhIIZY9j7Tz1Rzy3SAk4PJBRcMgcFoCRWaq0pxPHc3chTNwnrOfi7eEx+QB5xti9p04VVFx2ytzGhcDWTJZiEPqWflrx9B7qQhNquLxHk9g0kaQs.KAr+HiFJoq6kC7snwGjm2LcE4psmcUocWUlgiLZDaWs6HQ9h0A4lq4g7D+aklGNf5AYpoyDY+8MID+T55HzFVsKS.IDAss0KyjPGQ7ui7DtGF3AqWlKkaB6XBHiJM5ds7nw+LPeVX5HcJrSH6lf0EwbAOOhYAtbZ9TX5CYjR6BvqG46+wfzQ2SfLRuqilGVvya9j.eCD+rKo8eZdw3Q1Wm6BxCPWaj68eHjvxTQO8uOOMlx7dPzsYrwvXdGD9zr8uh0k.9IDe5KFYi.6Dd8UshTB35ajEkiYazAy4RnM0J5o1kVbS3tuJkyzcGNxki7Y3OupUjBl2DMdOyJwu8wrwvDBR8X4Qh8MO4OQi2f9ApV0oqkEh74WQtZwcB3th0WQ0pNFUAQ8eouZEqKt7gowaPeDFYGPDxB6Bge9UTKDQm.Sj3I35csR0HiJgCjva.1mJVWboOhupwE4dRb3HeKjO2VBE2NtnSf+cZ79jqtZUGiphyA4FfUfdDbspI5FluF9kArFoyXQx1V0PVkygqLdZzcYFB8nmrwH.BbHTeB8zkI8PbmV8.ZZMLB3yS3Cq1nJVWJR9bz38G+jpUcLpJlHgd+9YTw5Ry3MfrhUA2v9nz9Q0ig6LUB2.89DAX51XinQam8bzdNetQWLGLg2HrmUrtzJhFtYpAb1Uq5zQSODt2eWMRRHY3JWEMdewQUspiQURf+msbDuYuSl9nwsE1pv7.7j3qR3mSmWEqKEIuaZryrKoZUGiplffz3MV0JRJYyIL.WFr.AisR0nNOhF5blKc9OnJqrdzXrH7goywovMp.5mvvzyrqXcwGNPZLtecoX6o3nDX2r+JCeWHfwfrmnCtGXwHAzTiQvDcqE0oa+LWNEZbpFmU0pNcTbwHa98hHvI1IPOHSsL51axM1CZLBj9A9kHIB31ISCUU3FByaVHe1X3CQWbnWCwNZFFCKX1z3SpM+Sa3MmHMtnP1JZZLriOHgQJUKTKO7ksjPamtXjXNmgwvR1MjUzatUshXTXLYjnFxMRXd3vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvnIzCRTa3xPhGQcxwx6g.dQjrH8o1FsyOEK2PZXLbikCbR8.7Nn6JCEWCI4frzLV+WityvsiggQxrTfILJfmtp0DOYoH8FaXXXz.iFImItuHIrhITspSSYHjTb0shLJqrxLwB4yFFC2X0UsBXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXTtXgQGitMlHvT.VOfICLIfEAbMHgXJCCCiNN5CX1HQS44.73HA1yZITNfpQMMLLLZM6MI24kVYGqF0znSBKYgXzohOlC4E.dfhRQL5dXzUsBXXj.2.gYhrwgXurCC3vUN2aEy9YFFFcYLczmt4GuJUJiNGrobZzMw1kvwukRUKL5Xw5Pynah8T4XuHv8W1JhQmIVGZFcSrWJGyrelw+DqCMitE5GcWyXvRVOL5fw5PynagYg98ql8yL9mXcnYzsv.JG6k.laIqGFcvXcnYzsfY+LiVh0glQ2.qKvaP43CVx5gQGNVGZFcCrmnuUnL6mYz.VGZFcCLfxwVLverj0CCCCi1lGj3a2oqtR0HiNRrQnYzoyTP1j5tLXIqGFcAXQaCipjMDXq.FKvihDDGWsy4L.4i8y5odorWUzd.dc0KyE3oKY4aXXTPLJfCE35AVBwmF4KC7UPBq1A78UNuWBnWOj6..OBvqBbxsyEfmr0.2Ag58P.mXIJeCCiBhSD3wPO7+rZm+eA.aR858mUN+eiGxcx.+8H0srVYz0E3IHttuh55jggQWHSA3pH9OreXfi.XCPlV1L.9kQd+GCYDNZc.9o7P9tiv6KztWPojOL55dMfOTIoCFFF4HyB3YH9nwlMRRPQiKJx497n2gvNmR4uc.uVj5MWf0HCWGYgykj6P6aWR5fggQNwdfXSrn+PdHf2eKp2X.tcRtyfES5se1uKR8dUfs2qqf1iigjuFtnRTOLLLZS1UfkR7eH+QSY82Ck5FT9sorMd6N06ylx5kWLJjE+P6Z3mVx5hggQFY5Hihx8Gw+BOaGsECnFvmNk0+ViTm6mpwEkVGD2Qw8Z3Bq.cwvvvS5EIo.69C3Egjoy8gyQocpA7lSQccWLg2hmxNO40S7Uw8rqP8wvvHkLaz6D5DxPa8cTZmkP5reVzNC+cYP14M2.MdczJ6HZXXTwrC.qh3cB833mSvFv8nzVooyoQC7rQpyrxfryaNVZ75X2pV0wvvnUbEnO5rrXL9IQ7ooUC3zRQcOrHm+smAYGkcB3ShdvkzG13H5zJAVy1r8LLLJP1dDWxvsCng.lVFZuCUospArKontQcN2CNCxNf8Awq9CZKsMHeZYsizN2UazNFFFk.WF5c.MmL1dmmRasDR2JU9b0O+GB8M0dZw0W3dmsQasIQZGyoZKPrvGjQ6R+.GdBuWVMHu1T7tMDu9uYrsDtZp+LjNPxBaBhuzEk1IJYroQdcV6j2HEXcnYztrejrQ+SqSvFkIitG8OXJp6dF40WeFjc.6txwdj1n8htvD2VazNFFFELWB5S27YHaS46ckP6kFeI6mW+be9LJ6.9dNxtcSUdCR3zfMJPrQnYzNLJjQnow0R1lxm1zMeYD23nUDLRn4lQYGf6B.bcsQas9D5lF9D1iLx.VGZFsCSEY68nQVcYhrZ+rw.rQ0e8yjQYGvV47+WYazVmDgQ3CqCMCiNX1IzmdXMzyilsholPakFeYK5JIdNYP1ArFNx9uR1m95nAdRB29WYwAiM7.aDZFsCaXBG+kA9SYn8RxAVSSTlcpQdc6Lcy9c9+KsMZuSFwoZAYzYt4KAibFqCMi1gMHgieOjse7p0g1xP29YS.3rPhLrPicnkjdkFbGY4ONisS+.mYj++RxX6XXXTRbxnOEwr57nyWost1DN2imvciPuN5xMmQ4OEfmxQ9aWFaqeXj13ovltYofMBMi1gmHgiO+LzVSCXKUN9fIb9GP8+t.jQCNlHu2tg+IhjdQb6iMx43GjmsCHaF8nYWpyGa5lFFc7rUnOBsj14.MiSJg1RKxTzKvKV+8uv5G6nbp2mvCYOZBsUla4ufj2PSK6LvqDo9+CZLs7YXXzgxXPeSo+t7rc5C3ATZmWA8jZxLibNGa8iMfScW.oaTZ8A7qTjczx4kxqi8h3Qq2OVJqqggQG.ZcD4SZZqGjXruVGI+gDpSvlWe0D1o01nT+GjlGobe6HqFay5LKvNcmNImwnVCj7kvq5TuGoI0wvvnCDsoJ9i7n9ZQVifh1JLNVfWn96eqNGWKar+nHq139frRn6FvGjFyHTQK2MR1aW68d.jnu6NgjHgeSH1J6QR37OTO9bvvvnCf0j34OyEQ5FYxYPxclUC3BTpy6Ix6eRNu2OoEsWqJKFIIuzLGFNskuaJt9MLL5.4zI9OnOklb9iiFSnvOEvQpzF2jS8VGDO2uFRZxa7Nu+.JsQZKKCXuq2N8f35GYsstdplrLkggQNPO.WLM9i5URXGDALJD2fHp+lMeBcWiaxoMVDvDq+diFYOUF7dZaGpd.tO7uCnkR7bOvVRiqVYZKOBw2sAFFFcYLZfqlF+w8PHgb5uFhy19XNu+kiXKp.lNxFKO54buH1jK5wuSjUmTioP5LzezNfRJzdeLD2P+Mqbwj7l02vvnKi9PL39Bn4+ve9jbXGZKQOiOETlCstSioRq6T6E.NUZss9diDuiX2xyhjbVLpXZmffmgQRLZfi.IxutYHc.8R.yC3pPVcxlENf5AYpoyDI50NIjUq75HLHNlFc3f.1ejPf8DQ5n8AQVsxautNkF5qttruHqR5XQVHjmDwdYWKxzSMLLLLLxG9+AoEjznHMtB3O.....IUjSD4pPfIH" ],
					"embed" : 1,
					"forceaspect" : 1,
					"id" : "obj-52",
					"maxclass" : "fpic",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "jit_matrix" ],
					"patching_rect" : [ 83.365452110767365, 103.5, 73.0, 36.5 ],
					"pic" : "GrD1.png"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-33",
					"linecount" : 37,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 9.0, 50.961723327636719, 262.0, 503.0 ],
					"text" : "Group delay is defined as the negative derivative of the phase spectrum of Short-Time Fourier Transform:\n\n\n\n\nThis would require phase unwrappinig. A more convenient way to compute it is via the formula:\n\n\n\n\nwhere          and         are the real and imaginary part of the STFT bins of the original signal          , while          and           are their \ncounterparts with respect to the signal\n\n\nIn this patch we are using a built-in STFT to compute the transform of           , but we need to implement our own \"hand-made\" STFT \nimplement this formula by building ourselves a Short-Time Fourier Transform step by step \"by hand\", because we also need the transform of\n              , which means that each window frame must be modulated with a ramp.\n\nThis is a good exercise that gets us acquainted with a number of objects in the ears library that come in handy when working with spectral analysis. \n\n(The first thing to know is that a spectral buffer, in ears, is a special buffer whose spectral bins are placed in different channels, and whose frames are placed as different time samples.)"
				}

			}
, 			{
				"box" : 				{
					"fontsize" : 32.0,
					"id" : "obj-9",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 9.0, 6.961723327636719, 680.0, 42.0 ],
					"text" : "Spectal Analysis And Group Delay"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-26",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "float" ],
					"patching_rect" : [ 760.5, 216.459854014598534, 39.5, 22.0 ],
					"text" : "* 1."
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-27",
					"maxclass" : "newobj",
					"numinlets" : 3,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 741.0, 248.466625288421085, 76.0, 22.0 ],
					"text" : "ears.collect~"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-29",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 3,
					"outlettype" : [ "bang", "", "int" ],
					"patching_rect" : [ 741.0, 188.459854014598534, 59.0, 22.0 ],
					"text" : "ears.iter~"
				}

			}
, 			{
				"box" : 				{
					"angle" : 270.0,
					"background" : 1,
					"bgcolor" : [ 0.847058823529412, 0.847058823529412, 0.847058823529412, 1.0 ],
					"border" : 1,
					"bordercolor" : [ 0.0, 0.0, 0.0, 1.0 ],
					"id" : "obj-44",
					"maxclass" : "panel",
					"mode" : 0,
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 734.0, 182.0, 93.149235894551111, 90.466625288421085 ],
					"proportion" : 0.5
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"destination" : [ "obj-25", 0 ],
					"source" : [ "obj-11", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-79", 0 ],
					"source" : [ "obj-11", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-72", 0 ],
					"source" : [ "obj-14", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-84", 1 ],
					"source" : [ "obj-16", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-84", 0 ],
					"source" : [ "obj-16", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-84", 3 ],
					"source" : [ "obj-23", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-14", 1 ],
					"source" : [ "obj-25", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-27", 1 ],
					"source" : [ "obj-26", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-28", 0 ],
					"source" : [ "obj-27", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-11", 0 ],
					"source" : [ "obj-28", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-26", 1 ],
					"source" : [ "obj-29", 2 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-26", 0 ],
					"source" : [ "obj-29", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-27", 0 ],
					"source" : [ "obj-29", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-16", 0 ],
					"source" : [ "obj-32", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-81", 0 ],
					"midpoints" : [ 491.0, 178.0, 540.75, 178.0, 540.75, 48.961723327636719, 650.5, 48.961723327636719 ],
					"source" : [ "obj-32", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-6", 0 ],
					"source" : [ "obj-48", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-29", 0 ],
					"source" : [ "obj-54", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-54", 0 ],
					"source" : [ "obj-6", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-75", 0 ],
					"source" : [ "obj-63", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-32", 0 ],
					"source" : [ "obj-66", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-84", 2 ],
					"source" : [ "obj-72", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-66", 0 ],
					"source" : [ "obj-75", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-80", 1 ],
					"source" : [ "obj-79", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-23", 0 ],
					"source" : [ "obj-80", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-14", 0 ],
					"midpoints" : [ 650.5, 372.461723327636719, 720.5, 372.461723327636719 ],
					"source" : [ "obj-81", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-48", 0 ],
					"source" : [ "obj-81", 2 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-80", 0 ],
					"midpoints" : [ 692.5, 364.461723327636719, 901.833333333333485, 364.461723327636719 ],
					"source" : [ "obj-81", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-68", 0 ],
					"source" : [ "obj-84", 0 ]
				}

			}
 ],
		"dependency_cache" : [ 			{
				"name" : "GrDnXn.png",
				"bootpath" : "~/Downloads",
				"patcherrelativepath" : "../../../../../../../Downloads",
				"type" : "PNG",
				"implicit" : 1
			}
, 			{
				"name" : "bach.iter.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.assemble~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.channel~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.collect~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.expr~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.fft~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.format~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.iter~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.offset~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.read~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.specshow~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.split~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.stft~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.trans~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "ears.window~.mxo",
				"type" : "iLaX"
			}
 ],
		"autosave" : 0
	}

}
