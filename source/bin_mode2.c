#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "Rtypes.h"
#include "TROOT.h"
#include "TFile.h"
#include "TRandom.h"
#include "TH1.h"
#include "TH2.h"
#include "TObjArray.h"
#include "TObject.h"
#include "TKey.h"
#include "TSystem.h"
#include "TCutG.h"
#include "TTree.h"
#include "gdecomp.h"

#include "GEBSort.h"
#include "GTMerge.h"

#include "SortManager.h"

float egamBinWidth;

/* pointers to ROOT spectra */

//TH2F *SMAP_allhits;
//TH1D *hitpat;
//TH1D *CCmult;
//TH1D *CCsum;
//TH1D *CCsum_s;
//TH1D *CCadd;
//TH1D *radius_all;
//TH1D *rate_mode2;
//TH2F *CCe;
//TH2F *ggCC;
//TH2F *evsr_all;
//TH2F *z_plot;
//TH2F *xy_plot;

/* parameters */
extern TH1D* ehi[MAXDETPOS + 1];

/* for mean polar and azimuth angles */

double pol[MAXDETPOS + 1];
double azi[MAXDETPOS + 1];
long long int ndethits[MAXDETPOS + 1];

/* ----------------------------------------------------------------- */

int sup_mode2()
{
    /* declarations  else
       {
       printf ("Error: ascii AGATA_crmat.dat not found, quit\n");
       exit (1);
       };
     */

    //char str1[STRLEN], str2[STRLEN]; //unused
    //float pi, il;//unused
    int i;
    PARS* pars= SortManager::sinstance()->execParams;

    if ( !pars->noHists )
    {
        TH1D* mkTH1D ( char*, char*, int, double, double );
        TH2F* mkTH2F ( char*, char*, int, double, double, int, double, double );
    }

    /* initialize */

    for ( i = 0; i <= MAXDETPOS; i++ )
    {
        pol[i] = 0;
        azi[i] = 0;
        ndethits[i] = 0;
    };

    for ( i = 1; i <= MAXDETPOS; i++ )
    {
        pars->detpolang[i] = 0;
    }
    printf ( "MAXDETPOS=%i\n", MAXDETPOS );
    fflush ( stdout );

    return 0;

}

/* ----------------------------------------------------------------- */

int exit_mode2 ()
{
    /* declarations*/

    int i, mod, crystal;

    /* normalize and report */

    printf ( "\n" );
    printf ( "mean observed polar and azimuth angles\n" );
    printf ( "for the detectors we saw (module,crystal)\n" );
    printf ( "\n" );
    printf ( "------------------------------------------\n" );

    for ( i = 0; i <= MAXDETPOS; i++ )
        if ( ndethits[i] > 0 )
        {
            pol[i] /= ndethits[i];
            azi[i] /= ndethits[i];
            mod = i / 4;
            crystal = i - 4 * mod;
            printf ( "mean pol/azi angle for detector %3i (%2i,%1i) are %7.2f/%7.2f\n", i, mod, crystal, pol[i] * 57.2958,
                     azi[i] * 57.2958 );
        };

    printf ( "------------------------------------------\n" );
    printf ( "\n" );

    return 0;

}

/* ----------------------------------------------------------------- */

int bin_mode2 ( GEB_EVENT* gebEvt )
{
    /* declarations */
    //int nn, rmax,rmin;//unused
    int i, j, crystalno, moduleno, detno;
    float sX, sY, polAng, aziAng, rr, xx, yy, zz, r1;
    //double d2;//unused
    double d1;
    char str[128];
    //float detDpFac, orig_seg_e;//unused
    float dp, addedEnergy = 0, r2;
    float RAD2DEG = 0.0174532925;
    float CCenergies[MAX_GAMMA_RAYS];
    static int firsttime = 1;
    static long long int t0;
    float polang[MAX_GAMMA_RAYS];
    float doppler_factor[MAX_GAMMA_RAYS];
    //float xar[MAXCOINEV], yar[MAXCOINEV], zar[MAXCOINEV];//unused
    //int detectorPosition, crystalNumber;//unused
    int i1, ndecomp;
    int nCCenergies;
    static int nperrors = 0;

    PARS* pars= SortManager::sinstance()->execParams;
    char* ptinp;

    if ( pars->CurEvNo <= pars->NumToPrint )
    {
        printf ( "entered bin_mode2: %i/%i\n", pars.CurEvNo, pars.NumToPrint );
    }

    if ( pars.requiretracked )
    {

        /* require tracked data before we bin mode2 data */

        i1 = 0;
        for ( i = 0; i < gebEvt->mult; i++ )
            if ( gebEvt->ptgd[i]->type == GEB_TYPE_TRACK )
            {
                i1++;
            }
        if ( i1 == 0 )
        {
            return ( 0 );
        }

    }


    addedEnergy = 0;
    ndecomp = 0;
    for ( i = 0; i < gebEvt->mult; i++ )
    {
        CCenergies[i] = 0;
    }
    nCCenergies = 0;
    for ( i = 0; i < gebEvt->mult; i++ )
    {
        if ( gebEvt->ptgd[i]->type == GEB_TYPE_DECOMP )
        {
            ndecomp++;

            /* cast */

            ptinp = ( char* ) gebEvt->ptinp[i];

            if ( pars.CurEvNo <= pars.NumToPrint )
            {
                GebTypeStr ( gebEvt->ptgd[i]->type, str );
                printf ( "bin_mode2, %2i> %2i, %s, TS=%lli\n", i, gebEvt->ptgd[i]->type, str,
                         gebEvt->ptgd[i]->timestamp );
            }


            /* mode 2 rate spectrum, x=minute, y=Hz */

            if ( firsttime )
            {
                firsttime = 0;
                t0 = gebEvt->ptgd[i]->timestamp;
            };
            d1 = ( double ) ( gebEvt->ptgd[i]->timestamp - t0 );
            d1 /= 100000000;
            d1 /= 60;

            //if (d1 > 0 && d1 < (double) RATELEN)
            //if ( !pars.noHists ) rate_mode2->Fill (d1, 1 / 60.0);

            /* find basic info */

            crystalno = ( ptinp->crystal_id & 0x0003 );
            moduleno = ( ( ptinp->crystal_id & 0xfffc ) >> 2 );
            detno = moduleno * 4 + crystalno;

            /* make z_plot and xy_plot */

            //if ( !pars.noHists )
            //{
            //for (j = 0; j < ptinp->num; j++)
            //  {
            //  if (pars.AGATA_data==0)
            //    //z_plot->Fill((double)(moduleno * 4 + crystalno),(double)ptinp->intpts[j].z,1.0);
            //  //else if (pars.AGATA_data==1)
            //    //z_plot->Fill((double)(moduleno * 3 + crystalno),(double)ptinp->intpts[j].z,1.0);
            //   //xy_plot->Fill((double)ptinp->intpts[j].x,(double)ptinp->intpts[j].y,1.0);
            //  }
            //}


            if ( pars.CurEvNo <= pars.NumToPrint )
            {
                printf ( "* %i/%i, is GEB_TYPE_DECOMP: num=%i\n", i, gebEvt->mult, ptinp->num );
                printf ( "__detno: %i, module: %i, crystalNumber: %i\n", detno, moduleno, crystalno );
            }

            /* calibrate mode 2 CC data */

            ptinp->tot_e = ptinp->tot_e * pars.CCcal_gain[detno] + pars.CCcal_offset[detno];
            addedEnergy += ptinp->tot_e;
            CCenergies[nCCenergies++] = ptinp->tot_e;
            if ( pars.CurEvNo <= pars.NumToPrint )
            {
                printf ( "CCenergies[%i]=%f\n", nCCenergies - 1, CCenergies[nCCenergies - 1] );
            }

            /* calibrate mode2 segment energies */

            for ( j = 0; j < ptinp->num; j++ )
            {
//            printf("%3i,%3i: e=%7.1f --> ", detno,ptinp->intpts[j].seg,ptinp->intpts[j].e);
                ptinp->intpts[j].e = ptinp->intpts[j].e * pars.SEGcal_gain[detno][ptinp->intpts[j].seg]
                                     + pars.SEGcal_offset[detno][ptinp->intpts[j].seg];
//            printf("%7.1f (%7.3f,%7.3f)\n", ptinp->intpts[j].e,pars.SEGcal_offset[detno][ptinp->intpts[j].seg],pars.SEGcal_gain[detno][ptinp->intpts[j].seg]);
            }

            /* store original/calibrated segment energy for later use */

            //orig_seg_e = ptinp->intpts[0].e;//unused

            /* hit pattern */

            //if ( !pars.noHists ) hitpat->Fill ((double) detno, 1);

            /* worldmap all hits */

            for ( j = 0; j < ptinp->num; j++ )
            {

                if ( pars.nocrystaltoworldrot == 0 )
                {

                    if ( pars.AGATA_data == 0 )
                    {

                        /* rotate into world coordinates first */
                        /* and make it cm rather than mm because */
                        /* crmat needs it in cm */

                        if ( pars.CurEvNo <= pars.NumToPrint )
                        {
                            printf ( "* %i: ", j );
                            printf ( "%7.2f,%7.2f,%7.2f --> ", ptinp->intpts[j].x, ptinp->intpts[j].y, ptinp->intpts[j].z );
                        }

                        xx = ptinp->intpts[j].x / 10.0;
                        yy = ptinp->intpts[j].y / 10.0;
                        zz = ptinp->intpts[j].z / 10.0;


                        ptinp->intpts[j].x = pars.crmat[moduleno][crystalno][0][0] * xx
                                             + pars.crmat[moduleno][crystalno][0][1] * yy
                                             + pars.crmat[moduleno][crystalno][0][2] * zz + pars.crmat[moduleno][crystalno][0][3];

                        ptinp->intpts[j].y = pars.crmat[moduleno][crystalno][1][0] * xx
                                             + pars.crmat[moduleno][crystalno][1][1] * yy
                                             + pars.crmat[moduleno][crystalno][1][2] * zz + pars.crmat[moduleno][crystalno][1][3];

                        ptinp->intpts[j].z = pars.crmat[moduleno][crystalno][2][0] * xx
                                             + pars.crmat[moduleno][crystalno][2][1] * yy
                                             + pars.crmat[moduleno][crystalno][2][2] * zz + pars.crmat[moduleno][crystalno][2][3];

                        if ( pars.CurEvNo <= pars.NumToPrint )
                        {
                            printf ( "%7.2f,%7.2f,%7.2f\n", ptinp->intpts[j].x, ptinp->intpts[j].y, ptinp->intpts[j].z );
                        }

                    }
                    else if ( pars.AGATA_data == 1 )
                    {
                        detno = moduleno * 3 + crystalno;


                        xx = ptinp->intpts[j].x;
                        yy = ptinp->intpts[j].y;
                        zz = ptinp->intpts[j].z;
//                      printf("nnn: %i,%f\n", moduleno * 3 + crystalno,(float)ptinp->intpts[j].z,1.0);

                        ptinp->intpts[j].x =
                            pars.rotxx[detno] * xx + pars.rotxy[detno] * yy + pars.rotxz[detno] * zz + pars.TrX[detno];
                        ptinp->intpts[j].y =
                            pars.rotyx[detno] * xx + pars.rotyy[detno] * yy + pars.rotyz[detno] * zz + pars.TrY[detno];;
                        ptinp->intpts[j].z =
                            pars.rotzx[detno] * xx + pars.rotzy[detno] * yy + pars.rotzz[detno] * zz + pars.TrZ[detno];;


                        if ( pars.CurEvNo <= pars.NumToPrint )
                        {
                            printf ( "AG::x: %9.2f --> %9.2f\n", xx, ptinp->intpts[j].x );
                            printf ( "AG::y: %9.2f --> %9.2f\n", yy, ptinp->intpts[j].y );
                            printf ( "AG::z: %9.2f --> %9.2f\n", zz, ptinp->intpts[j].z );
                            r1 = xx * xx + yy * yy + zz * zz;
                            r1 = sqrtf ( r1 );
                            r2 = ptinp->intpts[j].x * ptinp->intpts[j].x
                                 + ptinp->intpts[j].y * ptinp->intpts[j].y + ptinp->intpts[j].z * ptinp->intpts[j].z;
                            r2 = sqrtf ( r2 );
                            printf ( "AG::radius %f --> %f\n", r1, r2 );
                        }

                        ptinp->intpts[j].x /= 10;
                        ptinp->intpts[j].y /= 10;
                        ptinp->intpts[j].z /= 10;


                    }

                    else
                    {
                        /* no rotation case, just make it cm */

                        xx = ptinp->intpts[j].x / 10.0;
                        yy = ptinp->intpts[j].y / 10.0;
                        zz = ptinp->intpts[j].z / 10.0;

                    };

                }

                polAng = findPolarFromCartesian ( ptinp->intpts[j].x, ptinp->intpts[j].y, ptinp->intpts[j].z, &rr );
                aziAng = findAzimuthFromCartesian ( ptinp->intpts[j].x, ptinp->intpts[j].y );

                ndethits[detno]++;
                pol[detno] += polAng;
                azi[detno] += aziAng;

                if ( rr > RMIN && rr < RMAX )
                {
                    //if ( !pars.noHists ) radius_all->Fill ((double) rr, 1);
                    //need to make sure if the following if statement controls the evsr Fill
                    //if ( !pars.noHists && ptinp->intpts[j].e > 0 && ptinp->intpts[j].e < MEDIUMLEN ) {};
                    //evsr_all->Fill ((double) rr, ptinp->intpts[j].e);
                };

                /* SMAP coordinates */

                sX = aziAng * sinf ( polAng ) / RAD2DEG;
                sY = polAng / RAD2DEG;    /* + 1.5; */

                if ( pars.CurEvNo <= pars.NumToPrint && 0 )
                {
                    printf ( "%i [type %i] ", j, gebEvt->ptgd[i]->type );
                    printf ( "e: %9.2f/%9.2f ", ptinp->intpts[j].e, ptinp->tot_e );
                    printf ( "(%6.2f,%6.2f,%6.2f)cry --> ", xx, yy, zz );
                    printf ( "(%6.2f,%6.2f,%6.2f)world(cm); ", ptinp->intpts[j].x, ptinp->intpts[j].y, ptinp->intpts[j].z );
//                  printf (" sX,sY=%6.2f,%6.2f ", sX, sY);
                    printf ( "\n" );
                };

                /* update */

                //if (sX >= -180 && sX <= 180 && sY >= 0 && sY <= 180)
                //if ( !pars.noHists ) SMAP_allhits->Fill (sX, sY, 1);
                //else
                //{
                if ( nperrors < 10 )
                {
                    nperrors++;
                    printf ( "error: sX,sY= ( %11.6f , %11.6f )\n", sX, sY );
//                          exit (1);
                };
                //};
            };

            /* simple dopler corrected sum of CC energies */

            //if ( !pars.noHists ) CCsum_s->Fill (ptinp->tot_e / pars.modCCdopfac[ptinp->crystal_id], 1);


            /* quietly rescale all interaction energies to the CC energy */

            r1 = 0;
            for ( j = 0; j < ptinp->num; j++ )
            {
                r1 += ptinp->intpts[j].e;
            }
            for ( j = 0; j < ptinp->num; j++ )
            {
                ptinp->intpts[j].e = ptinp->intpts[j].e / r1 * ptinp->tot_e;
            }

            /* doppler correction (this is not the only way to do this!) */
            /* and you must do the rescale above for it to work */

            for ( j = 0; j < ptinp->num; j++ )
            {
                rr =
                    ptinp->intpts[j].x * ptinp->intpts[j].x + ptinp->intpts[j].y * ptinp->intpts[j].y +
                    ptinp->intpts[j].z * ptinp->intpts[j].z;
                rr = sqrtf ( rr );

                dp = ( ptinp->intpts[j].x * pars.beamdir[0] +
                       ptinp->intpts[j].y * pars.beamdir[1] + ptinp->intpts[j].z * pars.beamdir[2] ) / rr;

                if ( dp < -1.0 )
                {
                    dp = -1.0;
                }
                if ( dp > 1.0 )
                {
                    dp = 1.0;
                }
                polang[j] = acosf ( dp );

                rr = 1.0 - pars.beta * pars.beta;
                doppler_factor[j] = sqrt ( rr ) / ( 1.0 - pars.beta * cos ( polang[j] ) );

            };

            ptinp->tot_e = 0;
            for ( j = 0; j < ptinp->num; j++ )
            {
                ptinp->tot_e += ( ptinp->intpts[j].e / doppler_factor[j] );
            }


            /* central contact energy matrix and total energy */

            /*if ( !pars.noHists )
            {
                if ( detno > 0 && detno < MAXDETPOS )
                    if ( ptinp->tot_e > 0 && ptinp->tot_e < LONGLEN )
                    {
                        //CCsum->Fill ((double) ptinp->tot_e, 1);
                        //CCe->Fill ((double) detno, (double) ptinp->tot_e, 1);
            //                ehi[detno]->Fill ((double) ptinp->tot_e, 1);
                    };
            }*/

        };

    };

    /* update added energy spectrum */

    //if ( !pars.noHists ) CCadd->Fill ((double) addedEnergy, 1);

    /* fill the ggCC martrix */

    //CCmult->Fill (nCCenergies, 1);
    /*if ( !pars.noHists && nCCenergies >= pars.multlo && nCCenergies <= pars.multhi )
        for ( i = 0; i < nCCenergies; i++ )
            for ( j = i + 1; j < nCCenergies; j++ )
            {
                //ggCC->Fill (CCenergies[i], CCenergies[j], 1.0);
                //ggCC->Fill (CCenergies[j], CCenergies[i], 1.0);
            };
    */
    /* done */

    if ( pars.CurEvNo <= pars.NumToPrint )
    {
        printf ( "exit bin_mode2\n" );
    }

    return ( 0 );

}
