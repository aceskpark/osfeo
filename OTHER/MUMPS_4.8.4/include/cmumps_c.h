/*
 *
 *  This file is part of MUMPS 4.8.4, built on Mon Dec 15 15:31:38 UTC 2008
 *
 *
 *  This version of MUMPS is provided to you free of charge. It is public
 *  domain, based on public domain software developed during the Esprit IV
 *  European project PARASOL (1996-1999) by CERFACS, ENSEEIHT-IRIT and RAL.
 *  Since this first public domain version in 1999, the developments are
 *  supported by the following institutions: CERFACS, ENSEEIHT-IRIT, and
 *  INRIA.
 *
 *  Main contributors are Patrick Amestoy, Iain Duff, Abdou Guermouche,
 *  Jacko Koster, Jean-Yves L'Excellent, and Stephane Pralet.
 *
 *  Up-to-date copies of the MUMPS package can be obtained
 *  from the Web pages:
 *  http://mumps.enseeiht.fr/  or  http://graal.ens-lyon.fr/MUMPS
 *
 *
 *   THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY
 *   EXPRESSED OR IMPLIED. ANY USE IS AT YOUR OWN RISK.
 *
 *
 *  User documentation of any code that uses this software can
 *  include this complete notice. You can acknowledge (using
 *  references [1], [2], and [3]) the contribution of this package
 *  in any scientific publication dependent upon the use of the
 *  package. You shall use reasonable endeavours to notify
 *  the authors of the package of this publication.
 *
 *   [1] P. R. Amestoy, I. S. Duff and  J.-Y. L'Excellent,
 *   Multifrontal parallel distributed symmetric and unsymmetric solvers,
 *   in Comput. Methods in Appl. Mech. Eng., 184,  501-520 (2000).
 *
 *   [2] P. R. Amestoy, I. S. Duff, J. Koster and  J.-Y. L'Excellent,
 *   A fully asynchronous multifrontal solver using distributed dynamic
 *   scheduling, SIAM Journal of Matrix Analysis and Applications,
 *   Vol 23, No 1, pp 15-41 (2001).
 *
 *   [3] P. R. Amestoy and A. Guermouche and J.-Y. L'Excellent and
 *   S. Pralet, Hybrid scheduling for the parallel solution of linear
 *   systems. Parallel Computing Vol 32 (2), pp 136-156 (2006).
 *
 */

/* Mostly written in march 2002 (JYL) */

#ifndef CMUMPS_C_H
#define CMUMPS_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mumps_compat.h"
/* Next line defines MUMPS_INT, CMUMPS_COMPLEX and CMUMPS_REAL */
#include "mumps_c_types.h"

#define MUMPS_VERSION "4.8.4"
#define MUMPS_VERSION_MAX_LEN 14

/*
 * Definition of the (simplified) MUMPS C structure.
 * NB: CMUMPS_COMPLEX are REAL types in s and d arithmetics.
 */
typedef struct {

    MUMPS_INT      sym, par, job;
    MUMPS_INT      comm_fortran;    /* Fortran communicator */
    MUMPS_INT      icntl[40];
    CMUMPS_REAL    cntl[15];
    MUMPS_INT      n;

    MUMPS_INT      nz_alloc; /* used in matlab interface to decide if we
                                free + malloc when we have large variation */

    /* Assembled entry */
    MUMPS_INT      nz;
    MUMPS_INT      *irn;
    MUMPS_INT      *jcn;
    CMUMPS_COMPLEX *a;

    /* Distributed entry */
    MUMPS_INT      nz_loc;
    MUMPS_INT      *irn_loc;
    MUMPS_INT      *jcn_loc;
    CMUMPS_COMPLEX *a_loc;

    /* Element entry */
    MUMPS_INT      nelt;
    MUMPS_INT      *eltptr;
    MUMPS_INT      *eltvar;
    CMUMPS_COMPLEX *a_elt;

    /* Ordering, if given by user */
    MUMPS_INT      *perm_in;

    /* Orderings returned to user */
    MUMPS_INT      *sym_perm;    /* symmetric permutation */
    MUMPS_INT      *uns_perm;    /* column permutation */

    /* Scaling (input only in this version) */
    CMUMPS_REAL    *colsca;
    CMUMPS_REAL    *rowsca;

    /* RHS, solution, ouptput data and statistics */
    CMUMPS_COMPLEX *rhs, *redrhs, *rhs_sparse, *sol_loc;
    MUMPS_INT      *irhs_sparse, *irhs_ptr, *isol_loc;
    MUMPS_INT      nrhs, lrhs, lredrhs, nz_rhs, lsol_loc;
    MUMPS_INT      schur_mloc, schur_nloc, schur_lld;
    MUMPS_INT      mblock, nblock, nprow, npcol;
    MUMPS_INT      info[40],infog[40];
    CMUMPS_REAL    rinfo[20], rinfog[20];

    /* Null space */
    MUMPS_INT      deficiency;
    MUMPS_INT      *pivnul_list;
    MUMPS_INT      *mapping;

    /* Schur */
    MUMPS_INT      size_schur;
    MUMPS_INT      *listvar_schur;
    CMUMPS_COMPLEX *schur;

    /* Internal parameters */
    MUMPS_INT      instance_number;

    /* Version number: length=14 in FORTRAN + 1 for final \0 + 1 for alignment */
    char version_number[MUMPS_VERSION_MAX_LEN + 1 + 1];
    /* For out-of-core */
    char ooc_tmpdir[256];
    char ooc_prefix[64];
    /* To save the matrix in matrix market format */
    char write_problem[256];

} CMUMPS_STRUC_C;


void MUMPS_CALL
cmumps_c( CMUMPS_STRUC_C * cmumps_par );


#ifdef __cplusplus
}
#endif

#endif /* CMUMPS_C_H */
