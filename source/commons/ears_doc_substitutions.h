/**
	@file	ears_doc_substitutions.h
	@brief	Line substitutions for documentation with Doctor Max
	
	by Daniele Ghisi
*/


#define EARSBUFOBJ_DECLARE_COMMON_METHODS
// @method (mouse) @digest Open the buffer display window
// @description Double-clicking on the object will open the display window for the output buffer(s).
// If more than 10 buffers are to be output, only the first 10 are displayed.
class_addmethod(c, (method)earsbufobj_dblclick, "dblclick", A_CANT, 0);
// @method reset @digest Restart naming cycle
// @description If the <m>naming</m> attribute is set to 'Dynamic',
// the <m>reset</m> message will force the dynamic naming to cycle and restart from the first
// used name. This is especially useful in combination with iterative mechanisms.
class_addmethod(c, (method)earsbufobj_reset, "reset", 0);
// @method writeaiff @digest Save output as audio file
// @description See equivalent <o>buffer~</o> method.
class_addmethod(c, (method)earsbufobj_writegeneral, "write", 0);
// @method writeaiff @digest Save output as AIFF file
// @description See equivalent <o>buffer~</o> method.
class_addmethod(c, (method)earsbufobj_writegeneral, "writeaif", 0);
// @method writeaiff @digest Save output as WAV file
// @description See equivalent <o>buffer~</o> method.
class_addmethod(c, (method)earsbufobj_writegeneral, "writewave", 0);
// @method writeaiff @digest Save output as FLAC file
// @description See equivalent <o>buffer~</o> method.
class_addmethod(c, (method)earsbufobj_writegeneral, "writeflac", 0);
// @method writeaiff @digest Save output as raw file with no header
// @description See equivalent <o>buffer~</o> method.
class_addmethod(c, (method)earsbufobj_writegeneral, "writeraw", 0);


#define earsbufobj_class_add_naming_attr
CLASS_ATTR_CHAR(c, "naming", 0, t_earsbufobj, l_bufouts_naming);
CLASS_ATTR_STYLE_LABEL(c,"naming",0,"enumindex","Output Naming Policy");
CLASS_ATTR_ENUMINDEX(c,"naming", 0, "Copy Static Dynamic");
CLASS_ATTR_ACCESSORS(c, "naming", NULL, earsbufobj_setattr_naming);
CLASS_ATTR_BASIC(c, "naming", 0);
// @description Chooses the output buffer naming policy: <br />
// 0 (Copy): the buffer name is copied from the input (in-place modification).
// Notice that some objects do not allow this policy. <br />
// 1 (Static): a single buffer (and hence buffer name) is created, and always used as output. <br />
// 2 (Dynamic): a new buffer (and buffer name) is created for each new command.
// Beware! This may allocate a lot of memory!
// You can always cycle on a fixed set of names via the <m>reset</m> message. <br />
// You can use a shortcut to define the naming policy via a first symbolic argument: use <b>=</b> for copy,
// <b>-</b> for static and <b>!</b> for dynamic.

#define earsbufobj_class_add_outname_attr
CLASS_ATTR_LLLL(c, "outname", 0, t_earsbufobj, l_outnames, earsbufobj_getattr_outname, earsbufobj_setattr_outname);
CLASS_ATTR_STYLE_LABEL(c,"outname",0,"text","Output Buffer Names");
CLASS_ATTR_BASIC(c, "outname", 0);
// @description Sets the name for each one of the buffer outlets. Leave blank to auto-assign
// unique names.

#define earsbufobj_class_add_ampunit_attr
CLASS_ATTR_CHAR(c, "ampunit", 0, t_earsbufobj, l_ampunit);
CLASS_ATTR_STYLE_LABEL(c,"ampunit",0,"enumindex","Amplitude Values Are In");
CLASS_ATTR_ENUMINDEX(c,"ampunit", 0, "Linear Decibel");
CLASS_ATTR_ACCESSORS(c, "ampunit", NULL, earsbufobj_setattr_ampunit);
CLASS_ATTR_BASIC(c, "ampunit", 0);
// @description Sets the unit for amplitudes: Linear (default) or Decibel (0dB corresponding to 1. in the linear scale).

#define earsbufobj_class_add_envampunit_attr
CLASS_ATTR_CHAR(c, "envampunit", 0, t_earsbufobj, l_envampunit);
CLASS_ATTR_STYLE_LABEL(c,"envampunit",0,"enumindex","Envelope Amplitude Values Are In");
CLASS_ATTR_ENUMINDEX(c,"envampunit", 0, "Linear Decibel");
CLASS_ATTR_ACCESSORS(c, "envampunit", NULL, earsbufobj_setattr_envampunit);
CLASS_ATTR_BASIC(c, "envampunit", 0);
// @description Sets the unit for amplitudes inside envelopes: Linear (default) or Decibel (0dB corresponding to 1. in the linear scale).

#define earsbufobj_class_add_timeunit_attr
CLASS_ATTR_CHAR(c, "timeunit", 0, t_earsbufobj, l_timeunit);
CLASS_ATTR_STYLE_LABEL(c,"timeunit",0,"enumindex","Time Values Are In");
CLASS_ATTR_ENUMINDEX(c,"timeunit", 0, "Milliseconds Samples Relative");
CLASS_ATTR_ACCESSORS(c, "timeunit", NULL, earsbufobj_setattr_timeunit);
CLASS_ATTR_BASIC(c, "timeunit", 0);
// @description Sets the unit for time values: Milliseconds, Samples or Relative (0. to 1. as a percentage of the buffer length).
// Object usually default to Milliseconds, except for <o>ears.repeat~</o> defaulting to relative.

#define earsbufobj_class_add_envtimeunit_attr
CLASS_ATTR_CHAR(c, "envtimeunit", 0, t_earsbufobj, l_envtimeunit);
CLASS_ATTR_STYLE_LABEL(c,"envtimeunit",0,"enumindex","Envelope Time Values Are In");
CLASS_ATTR_ENUMINDEX(c,"envtimeunit", 0, "Milliseconds Samples Relative");
CLASS_ATTR_ACCESSORS(c, "envtimeunit", NULL, earsbufobj_setattr_envtimeunit);
CLASS_ATTR_BASIC(c, "envtimeunit", 0);
// @description Sets the unit for time values inside envelopes: Milliseconds (default), Samples or Relative (0. to 1 as a percentage of the buffer length)

#define earsbufobj_class_add_pitchunit_attr
CLASS_ATTR_CHAR(c, "pitchunit", 0, t_earsbufobj, l_pitchunit);
CLASS_ATTR_STYLE_LABEL(c,"pitchunit",0,"enumindex","Pitch Values Are In");
CLASS_ATTR_ENUMINDEX(c,"pitchunit", 0, "Cents MIDI Frequency Ratio");
CLASS_ATTR_ACCESSORS(c, "pitchunit", NULL, earsbufobj_setattr_pitchunit);
CLASS_ATTR_BASIC(c, "pitchunit", 0);
// @description Sets the unit for pitch values: Cents (default), MIDI, or frequency ratio.
