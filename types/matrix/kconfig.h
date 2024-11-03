/*************************************************************************************************************
 * This file contains configuration parameters
 * 
 * 
 * See https://github.com/pronenewbits for more!
 ************************************************************************************************************/
#ifndef KONFIG_H
#define KONFIG_H

#include <stdint.h>
// #include <math.h>


/* State Space dimension */
#define SS_X_LEN    (2)
#define SS_Z_LEN    (1)
#define SS_U_LEN    (1)
#define SS_DT_MILIS (10)                            /* 10 ms */
#define SS_DT       float_prec(SS_DT_MILIS/1000.)   /* Sampling time */


/* Change this size based on the biggest matrix you will use */
#define MATRIX_MAXIMUM_SIZE     (6)

/* Define this to enable matrix bound checking */
// #define MATRIX_USE_BOUNDS_CHECKING

/* Set this define to choose math precision of the system */
#define PRECISION_SINGLE    1
#define PRECISION_DOUBLE    2
#define FPU_PRECISION       (PRECISION_SINGLE)

#define float_prec_ZERO     (1e-7)
#define float_prec_ZERO_ECO (1e-5)      /* 'Economical' zero, for noisy calculation where 'somewhat zero' is good enough */



/* Set this define to choose system implementation (mainly used to define how you print the matrix via the Matrix_t::vPrint() & Matrix_t::vPrintFull() function) */
#define SYSTEM_IMPLEMENTATION_PC                    1
#define SYSTEM_IMPLEMENTATION_EMBEDDED_CUSTOM       2
#define SYSTEM_IMPLEMENTATION_EMBEDDED_ARDUINO      3

#define SYSTEM_IMPLEMENTATION                       (SYSTEM_IMPLEMENTATION_EMBEDDED_ARDUINO)

#define MATRIX_ASSERT(truth, str) 


#endif // KONFIG_H