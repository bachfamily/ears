/**
	@file	ears_doc_substitutions.h
	@brief	Line substitutions for documentation with Doctor Max
	
	by Daniele Ghisi
*/


#define EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD
// @method (mouse) @digest Open the buffer display window
// @description Double-clicking on the object will open the display window for the output buffer(s).
// If more than 10 buffers are to be output, only the first 10 are displayed.
class_addmethod(c, (method)earsbufobj_dblclick, "dblclick", A_CANT, 0);
// @method reset @digest Restart naming allocation cycle
// @description If the <m>alloc</m> attribute is set to 'Dynamic',
// the <m>reset</m> message will force the dynamic naming to cycle and restart from the first
// used buffer name. This is especially useful in combination with iterative mechanisms.
class_addmethod(c, (method)earsbufobj_reset, "reset", 0);
// @method stop @digest Abort computation
// @description When a <m>stop</m> message is sent to an object with <m>blocking</m> attribute
// set to 0, the computation is aborted as soon as possible.
class_addmethod(c, (method)earsbufobj_stop, "stop", 0);
// @method write @digest Save output as audio file
// @description See equivalent <o>buffer~</o> method.
// Additional optional arguments specify the buffer index (if more than one buffer are stored in the objct)
// and the filename (otherwise a dialog menu will appear).
// Additional optional <m>format</m> message attributes specifies the output sample type, if applicable. <br />
// @copy EARS_DOC_ACCEPTED_SAMPLETYPES
// @marg 0 @name bufferindex @optional 1 @type int
// @marg 1 @name filename_or_path @optional 1 @type symbol
// @mattr format @type symbol @optional 1 @default int16 @digest Sample Format
class_addmethod(c, (method)earsbufobj_writegeneral, "write", A_GIMME, 0);
// @method writeaiff @digest Save output as AIFF file
// @description See equivalent <o>buffer~</o> method.
// Additional optional arguments specify the buffer index (if more than one buffer are stored in the objct)
// and the filename (otherwise a dialog menu will appear).
// Additional optional <m>format</m> message attributes specifies the output sample type, if applicable. <br />
// @copy EARS_DOC_ACCEPTED_SAMPLETYPES
// @marg 0 @name bufferindex @optional 1 @type int
// @marg 1 @name filename_or_path @optional 1 @type symbol
// @mattr format @type symbol @optional 1 @default int16 @digest Sample Format
class_addmethod(c, (method)earsbufobj_writegeneral, "writeaiff", A_GIMME, 0);
// @method writewave @digest Save output as WAV file
// @description See equivalent <o>buffer~</o> method.
// Additional optional arguments specify the buffer index (if more than one buffer are stored in the objct)
// and the filename (otherwise a dialog menu will appear).
// Additional optional <m>format</m> message attributes specifies the output sample type, if applicable. <br />
// @copy EARS_DOC_ACCEPTED_SAMPLETYPES
// @marg 0 @name bufferindex @optional 1 @type int
// @marg 1 @name filename_or_path @optional 1 @type symbol
// @mattr format @type symbol @optional 1 @default int16 @digest Sample Format
class_addmethod(c, (method)earsbufobj_writegeneral, "writewave", A_GIMME, 0);
// @method writeflac @digest Save output as FLAC file
// @description See equivalent <o>buffer~</o> method.
// Additional optional arguments specify the buffer index (if more than one buffer are stored in the objct)
// and the filename (otherwise a dialog menu will appear).
// Additional optional <m>format</m> message attributes specifies the output sample type, if applicable. <br />
// @copy EARS_DOC_ACCEPTED_SAMPLETYPES
// @marg 0 @name bufferindex @optional 1 @type int
// @marg 1 @name filename_or_path @optional 1 @type symbol
// @mattr format @type symbol @optional 1 @default int16 @digest Sample Format
class_addmethod(c, (method)earsbufobj_writegeneral, "writeflac", A_GIMME, 0);
// @method writeraw @digest Save output as raw file with no header
// @description See equivalent <o>buffer~</o> method.
// Additional optional arguments specify the buffer index (if more than one buffer are stored in the objct)
// and the filename (otherwise a dialog menu will appear).
// Additional optional <m>format</m> message attributes specifies the output sample type, if applicable. <br />
// @copy EARS_DOC_ACCEPTED_SAMPLETYPES
// @marg 0 @name bufferindex @optional 1 @type int
// @marg 1 @name filename_or_path @optional 1 @type symbol
// @mattr format @type symbol @optional 1 @default int16 @digest Sample Format
class_addmethod(c, (method)earsbufobj_writegeneral, "writeraw", A_GIMME, 0);
// @method writemp3 @digest Save output as MP3 file
// @description Save the buffer as lossy compressed MP3 file.
// Additional optional arguments specify the buffer index (if more than one buffer are stored in the objct)
// and the filename (otherwise a dialog menu will appear).
// Additional optional message attributes specify the encoding properties (variable bitrate mode, bitrates). <br />
// @copy EARS_DOC_ACCEPTED_SAMPLETYPES
// @marg 0 @name bufferindex @optional 1 @type int
// @marg 1 @name filename_or_path @optional 1 @type symbol
// @mattr vbrmode @type symbol @optional 1 @default VBR @digest VBR mode ("VBR", "CBR" or "ABR")
// @mattr bitrate @type int @optional 1 @digest Bitrate in kbps
// @mattr minbitrate @type int @optional 1 @digest Minimum bitrate in kbps
// @mattr maxbitrate @type int @optional 1 @digest Maximum bitrate in kbps
class_addmethod(c, (method)earsbufobj_writegeneral, "writemp3", A_GIMME, 0);
// @method writewv @digest Save output as WavPack file
// @description Save the buffer as lossless WavPack compression, or as lossy WavPack compression along with a
// correction file (if the <m>correction</m> message attribute is set to 1). In this last case, a <m>bitrate</m>
// message attribute specifies the bitrate of the lossy .wv file in kbps.
// Additional optional arguments specify the buffer index (if more than one buffer are stored in the objct)
// and the filename (otherwise a dialog menu will appear).
// Optional message attributes specify the encoding properties (correction, bitrate, sample ). <br />
// @copy EARS_DOC_ACCEPTED_SAMPLETYPES
// @marg 0 @name bufferindex @optional 1 @type int
// @marg 1 @name filename_or_path @optional 1 @type symbol
// @mattr correction @type int @optional 1 @default 0 @digest Write correction file along with a lossy wv file
// @mattr bitrate @type int @optional 1 @digest Bitrate in kbps
// @mattr format @type symbol @optional 1 @default int16 @digest Sample Format
class_addmethod(c, (method)earsbufobj_writegeneral, "writewv", A_GIMME, 0);

#define earsbufobj_class_add_outname_attr
CLASS_ATTR_LLLL(c, "outname", 0, t_earsbufobj, l_outnames, earsbufobj_getattr_outname, earsbufobj_setattr_outname);
CLASS_ATTR_STYLE_LABEL(c,"outname",0,"text","Output Buffer Names");
CLASS_ATTR_BASIC(c, "outname", 0);
// @description Sets the name for each one of the buffer outlets. Leave blank to auto-assign
// unique names.

#define earsbufobj_class_add_ampunit_attr
CLASS_ATTR_CHAR(c, "ampunit", 0, t_earsbufobj, l_ampunit);
CLASS_ATTR_STYLE_LABEL(c,"ampunit",0,"enumindex","Amplitude Values Unit");
CLASS_ATTR_ENUMINDEX(c,"ampunit", 0, "Linear Decibel");
CLASS_ATTR_ACCESSORS(c, "ampunit", NULL, earsbufobj_setattr_ampunit);
CLASS_ATTR_BASIC(c, "ampunit", 0);
// @description Sets the unit for amplitudes: Linear (default) or Decibel (0dB corresponding to 1. in the linear scale).

#define earsbufobj_class_add_envampunit_attr
CLASS_ATTR_CHAR(c, "envampunit", 0, t_earsbufobj, l_envampunit);
CLASS_ATTR_STYLE_LABEL(c,"envampunit",0,"enumindex","Envelope Amplitude Values Unit");
CLASS_ATTR_ENUMINDEX(c,"envampunit", 0, "Linear Decibel");
CLASS_ATTR_ACCESSORS(c, "envampunit", NULL, earsbufobj_setattr_envampunit);
CLASS_ATTR_BASIC(c, "envampunit", 0);
// @description Sets the unit for amplitudes inside envelopes: Linear (default) or Decibel (0dB corresponding to 1. in the linear scale).

#define earsbufobj_class_add_timeunit_attr
CLASS_ATTR_CHAR(c, "timeunit", 0, t_earsbufobj, l_timeunit);
CLASS_ATTR_STYLE_LABEL(c,"timeunit",0,"enumindex","Time Values Unit");
CLASS_ATTR_ENUMINDEX(c,"timeunit", 0, "Milliseconds Samples Duration Ratio Milliseconds Difference Samples Difference");
CLASS_ATTR_ACCESSORS(c, "timeunit", NULL, earsbufobj_setattr_timeunit);
CLASS_ATTR_BASIC(c, "timeunit", 0);
CLASS_ATTR_CATEGORY(c, "timeunit", 0, "Units");
// @description Sets the unit for time values: Milliseconds, Samples, Relative (0. to 1. as a percentage of the buffer length),
// The default varies depending on the modules.

#define earsbufobj_class_add_antimeunit_attr
CLASS_ATTR_CHAR(c, "antimeunit", 0, t_earsbufobj, l_antimeunit);
CLASS_ATTR_STYLE_LABEL(c,"antimeunit",0,"enumindex","Analysis Time Values Unit");
CLASS_ATTR_ENUMINDEX(c,"antimeunit", 0, "Milliseconds Samples Relative");
CLASS_ATTR_ACCESSORS(c, "antimeunit", NULL, earsbufobj_setattr_antimeunit);
CLASS_ATTR_BASIC(c, "antimeunit", 0);
CLASS_ATTR_CATEGORY(c, "antimeunit", 0, "Units");
// @description Sets the unit for analysis values: Milliseconds, Samples, Relative (0. to 1. as a percentage of the buffer length).


#define earsbufobj_class_add_envtimeunit_attr
CLASS_ATTR_CHAR(c, "envtimeunit", 0, t_earsbufobj, l_envtimeunit);
CLASS_ATTR_STYLE_LABEL(c,"envtimeunit",0,"enumindex","Envelope Time Values Unit");
CLASS_ATTR_ENUMINDEX(c,"envtimeunit", 0, "Milliseconds Samples Relative");
CLASS_ATTR_ACCESSORS(c, "envtimeunit", NULL, earsbufobj_setattr_envtimeunit);
CLASS_ATTR_BASIC(c, "envtimeunit", 0);
// @description Sets the unit for time values inside envelopes: Milliseconds, Samples, Relative (0. to 1. as a percentage of the buffer length).
// The default is Relative.

#define earsbufobj_class_add_pitchunit_attr
CLASS_ATTR_CHAR(c, "pitchunit", 0, t_earsbufobj, l_pitchunit);
CLASS_ATTR_STYLE_LABEL(c,"pitchunit",0,"enumindex","Pitch Values Unit");
CLASS_ATTR_ENUMINDEX(c,"pitchunit", 0, "Cents MIDI Hertz Frequency Ratio");
CLASS_ATTR_ACCESSORS(c, "pitchunit", NULL, earsbufobj_setattr_pitchunit);
CLASS_ATTR_BASIC(c, "pitchunit", 0);
// @description Sets the unit for pitch values: Cents (default), MIDI, Hertz (frequency), or frequency ratio.

#define earsbufobj_class_add_frequnit_attr
CLASS_ATTR_CHAR(c, "frequnit", 0, t_earsbufobj, l_frequnit);
CLASS_ATTR_STYLE_LABEL(c,"frequnit",0,"enumindex","Frequency Values Unit");
CLASS_ATTR_ENUMINDEX(c,"frequnit", 0, "Hertz BPM Cents MIDI");
CLASS_ATTR_ACCESSORS(c, "frequnit", NULL, earsbufobj_setattr_frequnit);
CLASS_ATTR_BASIC(c, "frequnit", 0);
// @description Sets the unit for pitch values: Hertz (default), BPM, Cents, MIDI numbers (semitones)


#define earsbufobj_class_add_angleunit_attr
CLASS_ATTR_CHAR(c, "angleunit", 0, t_earsbufobj, l_angleunit);
CLASS_ATTR_STYLE_LABEL(c,"angleunit",0,"enumindex","Angle Values Unit");
CLASS_ATTR_ENUMINDEX(c,"angleunit", 0, "Radians Degrees Turns");
CLASS_ATTR_ACCESSORS(c, "angleunit", NULL, earsbufobj_setattr_angleunit);
CLASS_ATTR_BASIC(c, "angleunit", 0);
// @description Sets the unit for angles: Radians (default), Degrees, or Turns.

#define earsbufobj_class_add_resamplingpolicy_attr
CLASS_ATTR_CHAR(c,"resamplingpolicy",0, t_earsbufobj, l_resamplingpolicy);
CLASS_ATTR_STYLE_LABEL(c,"resamplingpolicy",0,"enumindex","Resampling Policy");
CLASS_ATTR_ENUMINDEX(c,"resamplingpolicy", 0, "Don't To Lowest To Highest To Most Common To Max Current");
CLASS_ATTR_CATEGORY(c, "resamplingpolicy", 0, "Resampling");
// @description Sets the resampling policy used when buffers have different sample rates:
// "Don't" (no resampling - beware: temporality is not preserved!), "To lowest" (buffers are to the lowest sample rate),
// "To highest" (buffers are converted to the highest sample rate), "To most common" (buffers are to the most common
// sample rate), "To Max Current" (buffers are converted to the current Max sample rate).

#define earsbufobj_class_add_resamplingfiltersize_attr
CLASS_ATTR_CHAR(c,"resamplingfiltersize",0, t_earsbufobj, l_resamplingfilterwidth);
CLASS_ATTR_STYLE_LABEL(c,"resamplingfiltersize",0,"text","Resampling Filter Size");
CLASS_ATTR_CATEGORY(c, "resamplingfiltersize", 0, "Resampling");
// @description Sets the resampling filter size.


#define earsbufobj_class_add_resamplingmode_attr
CLASS_ATTR_SYM(c, "resamplingmode", 0, t_earsbufobj, l_resamplingmode_sym);
CLASS_ATTR_STYLE_LABEL(c,"resamplingmode",0,"enum","Resampling Mode");
CLASS_ATTR_ENUM(c,"resamplingmode",0,"sinc nearest neighbor sample and hold linear quadratic cubic");
CLASS_ATTR_ACCESSORS(c, "resamplingmode", NULL, earsbufobj_setattr_resamplingmode);
CLASS_ATTR_CATEGORY(c, "resamplingmode", 0, "Resampling");
// @description Sets the resampling mode (Sinc, Nearest Neighbor, Sample and Hold, Linear, Quadratic or Cubic). The default is "Sinc",
// which provides bandlimited interpolation.

#define earsbufobj_class_add_slopemapping_attr
CLASS_ATTR_CHAR(c,"slopemapping",0, t_earsbufobj, l_slopemapping);
CLASS_ATTR_STYLE_LABEL(c,"slopemapping",0,"enumindex","Slope Mapping");
CLASS_ATTR_ENUMINDEX(c,"slopemapping", 0, "bach Max");
// @description Sets the function to be used for slope mapping: either bach (default) or Max.




#define earsbufobj_class_add_framesize_attr
CLASS_ATTR_DOUBLE(c, "framesize", 0, t_earsbufobj, a_framesize);
CLASS_ATTR_STYLE_LABEL(c,"framesize",0,"text","Frame Size");
CLASS_ATTR_BASIC(c, "framesize", 0);
CLASS_ATTR_ACCESSORS(c, "framesize", NULL, earsbufobj_setattr_framesize);
CLASS_ATTR_CATEGORY(c, "framesize", 0, "Analysis");
// @description Sets the analysis frame size or window size (the unit depends on the <m>antimeunit</m> attribute)

#define earsbufobj_class_add_hopsize_attr
CLASS_ATTR_DOUBLE(c, "hopsize", 0, t_earsbufobj, a_hopsize);
CLASS_ATTR_STYLE_LABEL(c,"hopsize",0,"text","Hop Size");
CLASS_ATTR_BASIC(c, "hopsize", 0);
CLASS_ATTR_ACCESSORS(c, "hopsize", NULL, earsbufobj_setattr_hopsize);
CLASS_ATTR_CATEGORY(c, "hopsize", 0, "Analysis");
// @description Sets the analysis hop size (the unit depends on the <m>antimeunit</m> attribute)
// Floating point values are allowed.


#define earsbufobj_class_add_overlap_attr
CLASS_ATTR_DOUBLE(c, "overlap", 0, t_earsbufobj, a_overlap);
CLASS_ATTR_STYLE_LABEL(c,"overlap",0,"text","Overlap");
CLASS_ATTR_ACCESSORS(c, "overlap", NULL, earsbufobj_setattr_overlap);
CLASS_ATTR_CATEGORY(c, "overlap", 0, "Analysis");
// @description Sets the overlap factor between the analysis window size and the hop size.


#define earsbufobj_class_add_numframes_attr
CLASS_ATTR_ATOM(c, "numframes", 0, t_earsbufobj, a_numframes);
CLASS_ATTR_STYLE_LABEL(c,"numframes",0,"text","Number of Analysis Frames");
CLASS_ATTR_ACCESSORS(c, "numframes", NULL, earsbufobj_setattr_numframes);
CLASS_ATTR_CATEGORY(c, "numframes", 0, "Analysis");
// @description Sets the number of analysis frames. Defaults to "auto", as this number is a consequence of the
// <m>framesize</m> and <m>hopsize</m> attributes. If this number is set to a positive integer value, the <m>hopsize</m>
// is ignored and inferred from <m>numframes</m>.


#define earsbufobj_class_add_wintype_attr
CLASS_ATTR_SYM(c, "wintype", 0, t_earsbufobj, a_wintype);
CLASS_ATTR_STYLE_LABEL(c,"wintype",0,"enum","Window Type");
CLASS_ATTR_ENUM(c,"wintype", 0, "rectangular triangular sine hann hamming blackman nuttall blackmannuttall blackmanharris gaussian sqrthann sqrthamming");
CLASS_ATTR_BASIC(c, "wintype", 0);
CLASS_ATTR_CATEGORY(c, "wintype", 0, "Analysis");
// @description Sets the window type.
// Available windows are:
// "rectangular", "triangular", "sine", "hann", "hamming", "blackman", "nuttall", "blackmannuttall", "blackmanharris", "gaussian", "sqrthann", "sqrthamming". <br />
// For objects that may require different windows for analysis and synthesis, two symbols can be defined (the first one being the
// analysis window, the second one being the synthesis window).

#define earsbufobj_class_add_wintype_attr_essentia
CLASS_ATTR_SYM(c, "wintype", 0, t_earsbufobj, a_wintype);
CLASS_ATTR_STYLE_LABEL(c,"wintype",0,"enum","Window Type");
CLASS_ATTR_ENUM(c,"wintype", 0, "hamming hann hannnsgcq triangular square blackmanharris62 blackmanharris70 blackmanharris74 blackmanharris92");
CLASS_ATTR_BASIC(c, "wintype", 0);
CLASS_ATTR_CATEGORY(c, "wintype", 0, "Analysis");
// @description Sets the window type.
// Available windows are the ones allowed by the Essentia library:
// "hamming", "hann", "hannnsgcq", "triangular", "square", "blackmanharris62", "blackmanharris70", "blackmanharris74", "blackmanharris92". <br />
// For objects that may require different windows for analysis and synthesis, two symbols can be defined (the first one being the
// analysis window, the second one being the synthesis window).

#define earsbufobj_class_add_wintype_attr_ansyn
CLASS_ATTR_SYM_ARRAY(c, "wintype", 0, t_earsbufobj, a_wintype, 2);
CLASS_ATTR_STYLE_LABEL(c,"wintype",0,"text","Window Type");
CLASS_ATTR_BASIC(c, "wintype", 0);
CLASS_ATTR_CATEGORY(c, "wintype", 0, "Analysis");
// @description Sets the window type for both analaysis and synthesis (as a couple of symbols).
// Available windows are:
// "rectangular", "triangular", "sine", "hann", "hamming", "blackman", "nuttall", "blackmannuttall", "blackmanharris", "gaussian", "sqrthann", "sqrthamming". <br />
// For objects that may require different windows for analysis and synthesis, two symbols can be defined (the first one being the
// analysis window, the second one being the synthesis window).


#define earsbufobj_class_add_winnormalized_attr
CLASS_ATTR_CHAR(c, "winnormalized", 0, t_earsbufobj, a_winnorm);
CLASS_ATTR_STYLE_LABEL(c,"winnormalized",0,"onoff","Windows Are Normalized");
CLASS_ATTR_CATEGORY(c, "winnormalized", 0, "Analysis");
// @description Toggles the ability for windows to be normalized to have an area of 1 and then scaled by a factor of 2.

#define earsbufobj_class_add_zeropadding_attr
CLASS_ATTR_LONG(c, "zeropadding", 0, t_earsbufobj, a_zeropadding);
CLASS_ATTR_STYLE_LABEL(c,"zeropadding",0,"text","Zero Padding Amount");
CLASS_ATTR_CATEGORY(c, "zeropadding", 0, "Analysis");
// @description Sets the number of samples for zero padding.

#define earsbufobj_class_add_zerophase_attr
CLASS_ATTR_LONG(c, "zerophase", 0, t_earsbufobj, a_zerophase);
CLASS_ATTR_STYLE_LABEL(c,"zerophase",0,"onoff","Zero Phase Windowing");
CLASS_ATTR_CATEGORY(c, "zerophase", 0, "Analysis");
// @description Toggles zero-phase windowing.


#define earsbufobj_class_add_winstartfromzero_attr
CLASS_ATTR_CHAR(c, "winstartfromzero", 0, t_earsbufobj, a_winstartfromzero);
CLASS_ATTR_STYLE_LABEL(c,"winstartfromzero",0,"onoff","First Window Starts At Zero");
CLASS_ATTR_CATEGORY(c, "winstartfromzero", 0, "Analysis");
// @description If on, the first window is centered at framesize/2; if off (default), the first window is centered at zero.



#define earsbufobj_class_add_blocking_attr
CLASS_ATTR_CHAR(c, "blocking", 0, t_earsbufobj, l_blocking);
CLASS_ATTR_STYLE_LABEL(c,"blocking",0,"enumindex","Blocking Mode");
CLASS_ATTR_ENUMINDEX(c,"blocking", 0, "Non-Blocking Blocking (Low Priority) Blocking (High Priority)");
CLASS_ATTR_BASIC(c, "blocking", 0);
CLASS_ATTR_CATEGORY(c, "blocking", 0, "Behavior");
CLASS_ATTR_ACCESSORS(c, "blocking", NULL, earsbufobj_setattr_blocking);
// @description Sets the blocking mode, i.e. the thread to be used for computation: <br />
// 0: the object uses its own separate thread; <br />
// 1: the object uses the main thread (default); <br />
// 2: the object uses whatever thread the message comes in (including, notably the scheduler thread). <br />
// The <m>blocking</m> attribute is static: it can only be set in the object box at instantiation.


#define earsbufobj_class_add_polyout_attr
CLASS_ATTR_CHAR(c, "polyout", 0, t_earsbufobj, l_output_polybuffers);
CLASS_ATTR_STYLE_LABEL(c,"polyout",0,"enumindex","Output Polybuffers");
CLASS_ATTR_ENUMINDEX(c,"polyout", 0, "Don't Yes (Single Symbol) Yes (Buffer List)");
CLASS_ATTR_BASIC(c, "polyout", 0);
CLASS_ATTR_ACCESSORS(c, "polyout", NULL, earsbufobj_setattr_polyout);
CLASS_ATTR_CATEGORY(c, "polyout", 0, "Behavior");
// @description Toggles the ability to output a <o>polybuffer~</o> instead of a list of buffers: <br />
// - 0 (default) means that no polybuffer is created (individual buffers are output); <br />
// - 1 means that a polybuffer is created and its name is output; <br />
// - 2 means that a polybuffer is created and the individual names of its buffers are output.

