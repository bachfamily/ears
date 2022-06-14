/**
 @file
 ears.polyinterp.c
 
 @name
 ears.polyinterp
 
 @realname
 ears.polyinterp
 
 @type
 object
 
 @module
 ears
 
 @author
 Daniele Ghisi
 
 @status
 experimental
 
 @digest
 Get information on buffers
 
 @description
 Retrieves information about buffers
 
 @discussion
 
 @category
 ears buffer operations
 
 @keywords
 buffer, info, get, retrieve, rms, amplitude, peak
 
 @seealso
 info~
 
 @owner
 Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.buf.object.h"
#include <stdio.h>
#include <gsl/gsl_poly.h>

typedef struct _buf_info {
    t_earsbufobj       e_ob;
    
} t_buf_info;



// Prototypes
t_buf_info*         buf_info_new(t_symbol *s, short argc, t_atom *argv);
void            buf_info_free(t_buf_info *x);
void            buf_info_bang(t_buf_info *x);
void            buf_info_anything(t_buf_info *x, t_symbol *msg, long ac, t_atom *av);
void            buf_info_int(t_buf_info *x, t_atom_long num);
void            buf_info_float(t_buf_info *x, t_atom_float num);

void buf_info_assist(t_buf_info *x, void *b, long m, long a, char *s);
void buf_info_inletinfo(t_buf_info *x, void *b, long a, char *t);


// Globals and Statics
static t_class    *s_tag_class = NULL;
static t_symbol    *ps_event = NULL;

/**********************************************************************/
// Class Definition and Life Cycle

void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.polyinterp",
                         (method)buf_info_new,
                         (method)buf_info_free,
                         sizeof(t_buf_info),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    class_addmethod(c, (method)buf_info_int,                "int",            A_LONG, 0);
    class_addmethod(c, (method)buf_info_float,                "float",            A_FLOAT, 0);
    class_addmethod(c, (method)buf_info_anything,            "anything",            A_GIMME, 0);
    class_addmethod(c, (method)buf_info_anything,            "list",            A_GIMME, 0);
    class_addmethod(c, (method)buf_info_bang,               "bang",     0);
    
    class_addmethod(c, (method)buf_info_assist,                "assist",                A_CANT,        0);
    class_addmethod(c, (method)buf_info_inletinfo,            "inletinfo",            A_CANT,        0);
    
    earsbufobj_add_common_methods(c);
    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}



void buf_info_assist(t_buf_info *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
    } else {
        sprintf(s, "Output decomposed samples");
    }
}

void buf_info_inletinfo(t_buf_info *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_info *buf_info_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_info *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_info*)object_alloc_debug(s_tag_class);
    if (x) {
        
        earsbufobj_setup((t_earsbufobj *)x, "e", "4", NULL, EARSBUFOBJ_FLAG_DONT_DUPLICATE_INPUT_BUFFERS);
        
        attr_args_process(x, argc, argv);
    }
    return x;
}


void buf_info_free(t_buf_info *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_info_int(t_buf_info *x, t_atom_long num)
{
    t_atom argv[1];
    atom_setlong(argv, num);
    buf_info_anything(x, _sym_list, 1, argv);
}

void buf_info_float(t_buf_info *x, t_atom_float num)
{
    t_atom argv[1];
    atom_setfloat(argv, num);
    buf_info_anything(x, _sym_list, 1, argv);
}

void buf_info_bang(t_buf_info *x)
{

}


double sampleidx_to_x(long idx, double sr)
{
    return idx/sr;
}



void get_interpolation_coeffs(long numpts, double *xval, double *yval, long numcoeffs, double *coeffs)
{
    long n = numpts;
    
    double *temp = (double *)bach_newptr(numpts*sizeof(double));
    double *tempcoeffs = (double *)bach_newptr(numcoeffs*sizeof(double));
    
    for (long j = 0; j < numpts; j++)
        temp[j] = yval[j];
    
    for (long j = 0; j < n-1; j++) {
        for (long i = n-1; i > j; i--)
            temp[i] = (temp[i]-temp[i-1])/(xval[i]-xval[i-j-1]);
    }
    
    for (long c = 0; c < numcoeffs; c++)
        coeffs[c] = 0.;

    for (long i = n-1; i >= 0; i--) {
        
        tempcoeffs[0] = 1.;
        for (long c = 1; c < numcoeffs; c++)
            tempcoeffs[c] = 0.;
        
        for (long j = 0; j < i; j++) {
            for (long c = numcoeffs - 1; c >= 0; c--)
                tempcoeffs[c] = (c > 0 ? tempcoeffs[c-1] : 0) - xval[j] * tempcoeffs[c];
            long foo = 2;
            foo++;
//            mult *= (a-xval[j]);           /// x - xval[j]
        }
        
        for (long c = 0; c < numcoeffs; c++)
            tempcoeffs[c] *= temp[i];
        
        for (long c = 0; c < numcoeffs; c++)
            coeffs[c] += tempcoeffs[c];
    }
}


void buf_info_anything(t_buf_info *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);
    
    t_llll *parsed = llllobj_parse_llll((t_object *) x, LLLL_OBJ_VANILLA, msg, ac, av, LLLL_PARSE_CLONE);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0, false);
            
            double *yval = NULL;
            long numsamples = 0;
            
            t_buffer_obj *buf = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0);
            double sr = 1; //ears_buffer_get_sr((t_object *)x, buf);

            numsamples = ears_buffer_channel_to_double_array((t_object *)x, buf, 0, &yval);

            double *xval = (double *)bach_newptr(numsamples * sizeof(double));
            for (long i = 0; i < numsamples; i++)
                xval[i] = sampleidx_to_x(i, sr);
            
/*            // Lagrange interpolation
            t_llll *out = llll_get();
            
            for (long i = 0; i < numsamples; i++) {
                t_llll *this_poly = llll_get();
                for (long t = 0; t < numsamples; t++) {
                    double res = yval[i];
                    for (long k = 0; k < numsamples; k++) {
                        if (k != i) {
                            res *= (xval[t] - xval[k]);
                            res /=(xval[i] - xval[k]);
                        }
                    }
                    llll_appenddouble(this_poly, res);
                }
                llll_appendllll(out, this_poly);
            } */
            
            
//            Newton interpolation
            long numcoeffs = numsamples;
            long degree = numcoeffs - 1;
            double *coeffs = (double *)bach_newptr(numcoeffs * sizeof(double));
            double *roots = (double *)bach_newptr(2 * degree * sizeof(double));
            
            get_interpolation_coeffs(numsamples, xval, yval, numcoeffs, coeffs);
            
            
            int i;
            /* coefficients of P(x) =  -1 + x^5  */
            double a[6] = { -1, 0, 0, 0, 0, 1 };
            double z[10];
            
            gsl_poly_complex_workspace *w = gsl_poly_complex_workspace_alloc (numsamples);

            // gsl_poly_complex_solve (a, 6, w, z);
            gsl_poly_complex_solve (coeffs, numcoeffs, w, roots);
            
            t_llll *out = llll_get();
            for (long i = 0; i < degree; i++) {
                llll_appendllll(out, double_couple_to_llll(roots[2*i], roots[2*i+1]));
//                printf ("z%d = %+.18f %+.18f\n", i, z[2*i], z[2*i+1]);
            }
            earsbufobj_outlet_llll((t_earsbufobj *)x, 0, out);

            gsl_poly_complex_workspace_free (w);
            bach_freeptr(coeffs);
            bach_freeptr(roots);
            bach_freeptr(xval);
            bach_freeptr(yval);
            llll_free(out);
        }
    }
    llll_free(parsed);
}


