/* 
 * Calculation of Integral
 */
#include <stdio.h>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_monte.h>
#include <gsl/gsl_monte_vegas.h>

#include "timer.h"

double g (double *t, size_t dim, void *params);

int main (void)
{
    double res, err;
    double dist, dmin = 1.001, dmax = 4.;
    size_t dim = 6;
    int np = 20;
    double vegas[20], vegaserr[20];
    double time1, time2; 

    double dstep = (dmax - dmin) / (np - 1);

    double xl[] = { 0., 0., 0., 0., 0., 0. };
    double xu[] = { 1., 1., 1., 1., 1., 1. };

    gsl_rng *r = gsl_rng_alloc (gsl_rng_taus2);
    unsigned long seed = 1UL;

    gsl_rng_set (r, seed);

    size_t calls = 1000000;

    gsl_monte_function G = { &g, dim, &dist };

    gsl_monte_vegas_state *sv = gsl_monte_vegas_alloc (dim);

    gsl_monte_vegas_init (sv);

    dist = dmin;

// Vegas algorithm calculation

    timer_start ();

    // printf ("# Dist      Energy     ErrEst    Dipole\n");
    for (int i = 0; i < np; i++)
    {
        gsl_monte_vegas_integrate (&G, xl, xu, dim, calls / 5, r, sv, &res,
            &err);

        do
        {
            gsl_monte_vegas_integrate (&G, xl, xu, dim, calls, r, sv, &res,
                &err);
        }

        while (fabs (gsl_monte_vegas_chisq (sv) - 1.0) > 0.2);


        //printf ("% .6f  % .6f % .6f  % .6f\n", dist, res, err,
        //    -2. / pow (dist, 3.));

        //fflush (stdout);
        vegas[i] = res;
        vegaserr[i] = err;

        dist += dstep;
    }

    time1 = timer_stop ();

    gsl_monte_vegas_free (sv);

    dist = dmin;
    printf ("# Dist      HMC        Vegas      Verr       Dipole    Difference\n");

    double t[6];
    
// Calculation using homemade Monte Carlo algorithm
    
    timer_start ();

    for (int i = 0; i < np; i++)
    {
        double sum = 0.;

        for (int j = 0; j < (int) calls; j++)
        {
            for (int m = 0; m < (int) dim; m++)
            {
                t[m] = gsl_rng_uniform (r);
            }
            sum += g (t, dim, &dist);
        }
        double energy = sum / calls;
        double difference = fabs (energy - vegas[i]);

        printf ("% .6f  % .6f % .6f  % .6f  % .6f  % .6f\n", 
            dist, energy, vegas[i], vegaserr[i], -2./pow(dist, 3.), difference );
        dist += dstep;
     
     }

   time2 = timer_stop ();
    printf ("Time Vegas: %.2f Time HMC: %.2f\n", time1, time2);
    printf ("Speedup: %.2f\n", time1 / time2);
 
    gsl_rng_free (r);
     

    return 0;
}
