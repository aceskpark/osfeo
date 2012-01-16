      SUBROUTINE RSSI (NN,IA,JA,A,RHS,U,IWKSP,NW,WKSP,IPARM,RPARM,IERR) 
C       
C     ITPACK 2C MAIN SUBROUTINE  RSSI  (REDUCED SYSTEM SEMI-ITERATIVE)
C     EACH OF THE MAIN SUBROUTINES:   
C           JCG, JSI, SOR, SSORCG, SSORSI, RSCG, RSSI     
C     CAN BE USED INDEPENDENTLY OF THE OTHERS   
C       
C ... FUNCTION:   
C       
C          THIS SUBROUTINE, RSSI, DRIVES THE  REDUCED SYSTEM SI     
C          ALGORITHM.       
C       
C ... PARAMETER LIST:       
C       
C          N     INPUT INTEGER.  DIMENSION OF THE MATRIX. (= NN)    
C          IA,JA  INPUT INTEGER VECTORS.  THE TWO INTEGER ARRAYS OF 
C                 THE SPARSE MATRIX REPRESENTATION.       
C          A      INPUT D.P. VECTOR.  THE D.P. ARRAY OF THE SPARSE  
C                 MATRIX REPRESENTATION.
C          RHS    INPUT D.P. VECTOR.  CONTAINS THE RIGHT HAND SIDE  
C                 OF THE MATRIX PROBLEM.
C          U      INPUT/OUTPUT D.P. VECTOR.  ON INPUT, U CONTAINS THE 
C                 INITIAL GUESS TO THE SOLUTION. ON OUTPUT, IT CONTAINS 
C                 THE LATEST ESTIMATE TO THE SOLUTION.    
C          IWKSP  INTEGER VECTOR WORKSPACE OF LENGTH 3*N  
C          NW     INPUT INTEGER.  LENGTH OF AVAILABLE WKSP.  ON OUTPUT, 
C                 IPARM(8) IS AMOUNT USED.      
C          WKSP   D.P. VECTOR USED FOR WORKING SPACE.  RSSI 
C                 NEEDS THIS TO BE IN LENGTH AT LEAST  N + NB       
C                 HERE NB IS THE ORDER OF THE BLACK SUBSYSTEM       
C          IPARM  INTEGER VECTOR OF LENGTH 12.  ALLOWS USER TO SPECIFY
C                 SOME INTEGER PARAMETERS WHICH AFFECT THE METHOD.  IF
C          RPARM  D.P. VECTOR OF LENGTH 12. ALLOWS USER TO SPECIFY SOME 
C                 D.P. PARAMETERS WHICH AFFECT THE METHOD.
C          IER     OUTPUT INTEGER.  ERROR FLAG. (= IERR)  
C       
C ... RSSI SUBPROGRAM REFERENCES:     
C       
C          FROM ITPACK    BISRCH, CHEBY, CHGSI, DFAULT, ECHALL,     
C                         ECHOUT, ITERM, TIMER, ITRSSI, IVFILL,     
C                         PARSI, PERMAT, PERR, PERVEC, PMULT,     
C                         PRBNDX, PRSBLK, PRSRED, PSTOP, QSORT,     
C                         DAXPY, SBELM, SCAL, DCOPY, DDOT, SUM3,    
C                         TSTCHG, UNSCAL, VEVMW, VFILL, VOUT,       
C                         WEVMW       
C          SYSTEM         DABS, DLOG10, DBLE(AMAX0), DMAX1, DBLE(FLOAT),
C                         DSQRT       
C       
C     VERSION:  ITPACK 2C (MARCH 1982)
C       
C     CODE WRITTEN BY:  DAVID KINCAID, ROGER GRIMES, JOHN RESPESS   
C                       CENTER FOR NUMERICAL ANALYSIS     
C                       UNIVERSITY OF TEXAS     
C                       AUSTIN, TX  78712       
C                       (512) 471-1242
C       
C     FOR ADDITIONAL DETAILS ON THE   
C          (A) SUBROUTINE SEE TOMS ARTICLE 1982 
C          (B) ALGORITHM  SEE CNA REPORT 150    
C       
C     BASED ON THEORY BY:  DAVID YOUNG, DAVID KINCAID, LOU HAGEMAN  
C       
C     REFERENCE THE BOOK:  APPLIED ITERATIVE METHODS      
C                          L. HAGEMAN, D. YOUNG 
C                          ACADEMIC PRESS, 1981 
C       
C     **************************************************  
C     *               IMPORTANT NOTE                   *  
C     *                                                *  
C     *      WHEN INSTALLING ITPACK ROUTINES ON A      *  
C     *  DIFFERENT COMPUTER, RESET SOME OF THE VALUES  *  
C     *  IN  SUBROUTNE DFAULT.   MOST IMPORTANT ARE    *  
C     *                                                *  
C     *   DRELPR      MACHINE RELATIVE PRECISION       *  
C     *   RPARM(1)    STOPPING CRITERION               *  
C     *                                                *  
C     *   ALSO CHANGE SYSTEM-DEPENDENT ROUTINE         *  
C     *   SECOND USED IN TIMER                         *  
C     *                                                *  
C     **************************************************  
C       
C     SPECIFICATIONS FOR ARGUMENTS    
C       
      INTEGER IA(1),JA(1),IWKSP(1),IPARM(12),NN,NW,IERR   
      DOUBLE PRECISION A(1),RHS(NN),U(NN),WKSP(NW),RPARM(12)
C       
C     SPECIFICATIONS FOR LOCAL VARIABLES
C       
      INTEGER IB1,IB2,IDGTS,IER,IERPER,ITMAX1,JB3,LOOP,N,NB,NR,NRP1,N3
      DOUBLE PRECISION DIGIT1,DIGIT2,TEMP,TIME1,TIME2,TOL 
C       
C *** BEGIN: ITPACK COMMON  
C       
      INTEGER IN,IS,ISYM,ITMAX,LEVEL,NOUT       
      COMMON /ITCOM1/ IN,IS,ISYM,ITMAX,LEVEL,NOUT 
C       
      LOGICAL ADAPT,BETADT,CASEII,HALT,PARTAD   
      COMMON /ITCOM2/ ADAPT,BETADT,CASEII,HALT,PARTAD     
C       
      DOUBLE PRECISION BDELNM,BETAB,CME,DELNNM,DELSNM,FF,GAMMA,OMEGA,QA,
     *   QT,RHO,RRR,SIGE,SME,SPECR,SPR,DRELPR,STPTST,UDNM,ZETA      
      COMMON /ITCOM3/ BDELNM,BETAB,CME,DELNNM,DELSNM,FF,GAMMA,OMEGA,QA, 
     *   QT,RHO,RRR,SIGE,SME,SPECR,SPR,DRELPR,STPTST,UDNM,ZETA      
C       
C *** END  : ITPACK COMMON  
C       
C ... VARIABLES IN COMMON BLOCK - ITCOM1
C       
C     IN     - ITERATION NUMBER       
C     IS     - ITERATION NUMBER WHEN PARAMETERS LAST CHANGED
C     ISYM   - SYMMETRIC/NONSYMMETRIC STORAGE FORMAT SWITCH 
C     ITMAX  - MAXIMUM NUMBER OF ITERATIONS ALLOWED       
C     LEVEL  - LEVEL OF OUTPUT CONTROL SWITCH   
C     NOUT   - OUTPUT UNIT NUMBER     
C       
C ... VARIABLES IN COMMON BLOCK - ITCOM2
C       
C     ADAPT  - FULLY ADAPTIVE PROCEDURE SWITCH  
C     BETADT - SWITCH FOR ADAPTIVE DETERMINATION OF BETA  
C     CASEII - ADAPTIVE PROCEDURE CASE SWITCH   
C     HALT   - STOPPING TEST SWITCH   
C     PARTAD - PARTIALLY ADAPTIVE PROCEDURE SWITCH
C       
C ... VARIABLES IN COMMON BLOCK - ITCOM3
C       
C     BDELNM - TWO NORM OF B TIMES DELTA-SUPER-N
C     BETAB  - ESTIMATE FOR THE SPECTRAL RADIUS OF LU MATRIX
C     CME    - ESTIMATE OF LARGEST EIGENVALUE   
C     DELNNM - INNER PRODUCT OF PSEUDO-RESIDUAL AT ITERATION N      
C     DELSNM - INNER PRODUCT OF PSEUDO-RESIDUAL AT ITERATION S      
C     FF     - ADAPTIVE PROCEDURE DAMPING FACTOR
C     GAMMA  - ACCELERATION PARAMETER 
C     OMEGA  - OVERRELAXATION PARAMETER FOR SOR AND SSOR  
C     QA     - PSEUDO-RESIDUAL RATIO  
C     QT     - VIRTUAL SPECTRAL RADIUS
C     RHO    - ACCELERATION PARAMETER 
C     RRR    - ADAPTIVE PARAMETER     
C     SIGE   - PARAMETER SIGMA-SUB-E  
C     SME    - ESTIMATE OF SMALLEST EIGENVALUE  
C     SPECR  - SPECTRAL RADIUS ESTIMATE FOR SSOR
C     DRELPR - MACHINE RELATIVE PRECISION       
C     STPTST - STOPPING PARAMETER     
C     UDNM   - TWO NORM OF U
C     ZETA   - STOPPING CRITERION     
C       
C ... INITIALIZE COMMON BLOCKS
C       
      LEVEL = IPARM(2)      
      NOUT = IPARM(4)       
      IF (LEVEL.GE.1) WRITE (NOUT,10) 
   10 FORMAT ('0'///1X,'BEGINNING OF ITPACK SOLUTION MODULE  RSSI') 
      IER = 0     
      IF (IPARM(1).LE.0) RETURN       
      N = NN      
      IF (IPARM(11).EQ.0) TIMJ1 = TIMER(DUMMY)  
      IF (LEVEL.GE.3) GO TO 20
      CALL ECHOUT (IPARM,RPARM,7)     
      GO TO 30    
   20 CALL ECHALL (N,IA,JA,A,RHS,IPARM,RPARM,1) 
   30 TEMP = 5.0D2*DRELPR   
      IF (ZETA.GE.TEMP) GO TO 50      
      IF (LEVEL.GE.1) WRITE (NOUT,40) ZETA,DRELPR,TEMP    
   40 FORMAT ('0','*** W A R N I N G ************'/'0',   
     *   '    IN ITPACK ROUTINE RSSI'/' ','    RPARM(1) =',D10.3,   
     *   ' (ZETA)'/' ','    A VALUE THIS SMALL MAY HINDER CONVERGENCE '/
     *   ' ','    SINCE MACHINE PRECISION DRELPR =',D10.3/' ',      
     *   '    ZETA RESET TO ',D10.3)  
      ZETA = TEMP 
   50 CONTINUE    
      TIME1 = RPARM(9)      
      TIME2 = RPARM(10)     
      DIGIT1 = RPARM(11)    
      DIGIT2 = RPARM(12)    
C       
C ... VERIFY N    
C       
      IF (N.GT.0) GO TO 70  
      IER = 71    
      IF (LEVEL.GE.0) WRITE (NOUT,60) N 
   60 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE RSSI '/' ',      
     *   '    INVALID MATRIX DIMENSION, N =',I8)
      GO TO 420   
   70 CONTINUE    
C       
C ... REMOVE ROWS AND COLUMNS IF REQUESTED      
C       
      IF (IPARM(10).EQ.0) GO TO 90    
      TOL = RPARM(8)
      CALL IVFILL (N,IWKSP,0) 
      CALL VFILL (N,WKSP,0.0D0)       
      CALL SBELM (N,IA,JA,A,RHS,IWKSP,WKSP,TOL,ISYM,LEVEL,NOUT,IER) 
      IF (IER.EQ.0) GO TO 90
      IF (LEVEL.GE.0) WRITE (NOUT,80) IER,TOL   
   80 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE RSSI '/' ',      
     *   '    ERROR DETECTED IN SUBROUTINE  SBELM '/' ',  
     *   '    WHICH REMOVES ROWS AND COLUMNS OF SYSTEM '/' ',       
     *   '    WHEN DIAGONAL ENTRY TOO LARGE  '/' ','    IER = ',I5,5X,
     *   ' RPARM(8) = ',D10.3,' (TOL)') 
C       
C ... INITIALIZE WKSP BASE ADDRESSES. 
C       
   90 IB1 = 1     
      IB2 = IB1+N 
      JB3 = IB2+N 
C       
C ... PERMUTE TO  RED-BLACK SYSTEM IF POSSIBLE  
C       
      NB = IPARM(9) 
      IF (NB.GE.0) GO TO 110
      N3 = 3*N    
      CALL IVFILL (N3,IWKSP,0)
      CALL PRBNDX (N,NB,IA,JA,IWKSP,IWKSP(IB2),LEVEL,NOUT,IER)      
      IF (IER.EQ.0) GO TO 110 
      IF (LEVEL.GE.0) WRITE (NOUT,100) IER,NB   
  100 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE RSSI '/' ',      
     *   '    ERROR DETECTED IN SUBROUTINE  PRBNDX'/' ',  
     *   '    WHICH COMPUTES THE RED-BLACK INDEXING'/' ','    IER = ',I5
     *   ,' IPARM(9) = ',I5,' (NB)')  
      GO TO 420   
  110 IF (NB.GE.0.AND.NB.LE.N) GO TO 130
      IER = 74    
      IF (LEVEL.GE.1) WRITE (NOUT,120) IER,NB   
  120 FORMAT (/10X,'ERROR DETECTED IN RED-BLACK SUBSYSTEM INDEX'/10X, 
     *   'IER =',I5,' IPARM(9) =',I5,' (NB)')   
      GO TO 420   
  130 IF (NB.NE.0.AND.NB.NE.N) GO TO 150
      NB = N/2    
      IF (LEVEL.GE.2.AND.IPARM(9).GE.0) WRITE (NOUT,140) IPARM(9),NB
  140 FORMAT (/10X,' IPARM(9) = ',I5,' IMPLIES MATRIX IS DIAGONAL'/10X, 
     *   ' NB RESET TO ',I5)
C       
C ... PERMUTE MATRIX AND RHS
C       
  150 IF (IPARM(9).GE.0) GO TO 190    
      IF (LEVEL.GE.2) WRITE (NOUT,160) NB       
  160 FORMAT (/10X,'ORDER OF BLACK SUBSYSTEM = ',I5,' (NB)')
      CALL PERMAT (N,IA,JA,A,IWKSP,IWKSP(JB3),ISYM,LEVEL,NOUT,IER)  
      IF (IER.EQ.0) GO TO 180 
      IF (LEVEL.GE.0) WRITE (NOUT,170) IER      
  170 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE RSSI '/' ',      
     *   '    ERROR DETECTED IN SUBROUTINE  PERMAT'/' ',  
     *   '    WHICH DOES THE RED-BLACK PERMUTATION'/' ','    IER = ',I5)
      GO TO 420   
  180 CALL PERVEC (N,RHS,IWKSP)       
      CALL PERVEC (N,U,IWKSP) 
C       
C ... INITIALIZE WKSP BASE ADDRESSES  
C       
  190 NR = N-NB   
C       
      NRP1 = NR+1 
      IPARM(8) = N+NB       
      IF (NW.GE.IPARM(8)) GO TO 210   
      IER = 72    
      IF (LEVEL.GE.0) WRITE (NOUT,200) NW,IPARM(8)
  200 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE RSSI '/' ',      
     *   '    NOT ENOUGH WORKSPACE AT ',I10/' ','    SET IPARM(8) =',I10
     *   ,' (NW)')
      GO TO 420   
C       
C ... SCALE LINEAR SYSTEM, U, AND RHS BY THE SQUARE ROOT OF THE     
C ... DIAGONAL ELEMENTS.    
C       
  210 CONTINUE    
      CALL VFILL (IPARM(8),WKSP,0.0D0)
      CALL SCAL (N,IA,JA,A,RHS,U,WKSP,LEVEL,NOUT,IER)     
      IF (IER.EQ.0) GO TO 230 
      IF (LEVEL.GE.0) WRITE (NOUT,220) IER      
  220 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE RSSI '/' ',      
     *   '    ERROR DETECTED IN SUBROUTINE  SCAL  '/' ',  
     *   '    WHICH SCALES THE SYSTEM   '/' ','    IER = ',I5)      
      GO TO 420   
  230 IF (LEVEL.LE.2) GO TO 250       
      WRITE (NOUT,240)      
  240 FORMAT (///1X,'IN THE FOLLOWING, RHO AND GAMMA ARE',
     *   ' ACCELERATION PARAMETERS')  
  250 IF (IPARM(11).NE.0) GO TO 260   
      TIMI1 = TIMER(DUMMY)  
C       
C ... ITERATION SEQUENCE    
C       
  260 IF (N.GT.1) GO TO 270 
      U(1) = RHS(1) 
      GO TO 320   
  270 ITMAX1 = ITMAX+1      
      DO 290 LOOP = 1,ITMAX1
         IN = LOOP-1
         IF (MOD(IN,2).EQ.1) GO TO 280
C       
C ... CODE FOR THE EVEN ITERATIONS.   
C       
C     U           = U(IN)   
C     WKSP(IB1)   = U(IN-1) 
C       
         CALL ITRSSI (N,NB,IA,JA,A,RHS,U,WKSP(IB1),WKSP(IB2))       
C       
         IF (HALT) GO TO 320
         GO TO 290
C       
C ... CODE FOR THE ODD ITERATIONS.    
C       
C     U           = U(IN-1) 
C     WKSP(IB1)   = U(IN)   
C       
  280    CALL ITRSSI (N,NB,IA,JA,A,RHS,WKSP(IB1),U,WKSP(IB2))       
C       
         IF (HALT) GO TO 320
  290 CONTINUE    
C       
C ... ITMAX HAS BEEN REACHED
C       
      IF (IPARM(11).NE.0) GO TO 300   
      TIMI2 = TIMER(DUMMY)  
      TIME1 = DBLE(TIMI2-TIMI1)       
  300 IF (LEVEL.GE.1) WRITE (NOUT,310) ITMAX    
  310 FORMAT ('0','*** W A R N I N G ************'/'0',   
     *   '    IN ITPACK ROUTINE RSSI'/' ','    FAILURE TO CONVERGE IN', 
     *   I5,' ITERATIONS')  
      IER = 73    
      IF (IPARM(3).EQ.0) RPARM(1) = STPTST      
      GO TO 350   
C       
C ... METHOD HAS CONVERGED  
C       
  320 IF (IPARM(11).NE.0) GO TO 330   
      TIMI2 = TIMER(DUMMY)  
      TIME1 = DBLE(TIMI2-TIMI1)       
  330 IF (LEVEL.GE.1) WRITE (NOUT,340) IN       
  340 FORMAT (/1X,'RSSI  HAS CONVERGED IN ',I5,' ITERATIONS')       
C       
C ... PUT SOLUTION INTO U IF NOT ALREADY THERE. 
C       
  350 CONTINUE    
      IF (N.EQ.1) GO TO 360 
      IF (MOD(IN,2).EQ.1) CALL DCOPY (N,WKSP(IB1),1,U,1)  
      CALL DCOPY (NR,RHS,1,U,1)       
      CALL PRSRED (NB,NR,IA,JA,A,U(NRP1),U)     
C       
C ... UNSCALE THE MATRIX, SOLUTION, AND RHS VECTORS.      
C       
  360 CALL UNSCAL (N,IA,JA,A,RHS,U,WKSP)
C       
C ... UN-PERMUTE MATRIX,RHS, AND SOLUTION       
C       
      IF (IPARM(9).GE.0) GO TO 390    
      CALL PERMAT (N,IA,JA,A,IWKSP(IB2),IWKSP(JB3),ISYM,LEVEL,NOUT, 
     *   IERPER)  
      IF (IERPER.EQ.0) GO TO 380      
      IF (LEVEL.GE.0) WRITE (NOUT,370) IERPER   
  370 FORMAT ('0','*** F A T A L     E R R O R ************'/'0',   
     *   '    CALLED FROM ITPACK ROUTINE RSSI '/' ',      
     *   '    ERROR DETECTED IN SUBROUTINE  PERMAT'/' ',  
     *   '    WHICH UNDOES THE RED-BLACK PERMUTATION   '/' ',       
     *   '    IER = ',I5)   
      IF (IER.EQ.0) IER = IERPER      
      GO TO 420   
  380 CALL PERVEC (N,RHS,IWKSP(IB2))  
      CALL PERVEC (N,U,IWKSP(IB2))    
C       
C ... OPTIONAL ERROR ANALYSIS 
C       
  390 IDGTS = IPARM(12)     
      IF (IDGTS.LT.0) GO TO 400       
      IF (IPARM(2).LE.0) IDGTS = 0    
      CALL PERR (N,IA,JA,A,RHS,U,WKSP,DIGIT1,DIGIT2,IDGTS)
C       
C ... SET RETURN PARAMETERS IN IPARM AND RPARM  
C       
  400 IF (IPARM(11).NE.0) GO TO 410   
      TIMJ2 = TIMER(DUMMY)  
      TIME2 = DBLE(TIMJ2-TIMJ1)       
  410 IF (IPARM(3).NE.0) GO TO 420    
      IPARM(1) = IN 
      IPARM(9) = NB 
      RPARM(2) = CME
      RPARM(3) = SME
      RPARM(9) = TIME1      
      RPARM(10) = TIME2     
      RPARM(11) = DIGIT1    
      RPARM(12) = DIGIT2    
C       
  420 CONTINUE    
      IERR = IER  
      IF (LEVEL.GE.3) CALL ECHALL (N,IA,JA,A,RHS,IPARM,RPARM,2)     
C       
      RETURN      
      END 

      SUBROUTINE ITRSSI (N,NNB,IA,JA,A,RHS,UB,UB1,DB)     
C       
C ... FUNCTION:   
C       
C          THIS SUBROUTINE, ITRSSI, PERFORMS ONE ITERATION OF THE   
C          REDUCED SYSTEM SEMI-ITERATION ALGORITHM.  IT IS
C          CALLED BY RSSI.  
C       
C ... PARAMETER LIST:       
C       
C          N      INPUT INTEGER.  DIMENSION OF THE MATRIX.
C          NB     INPUT INTEGER.  CONTAINS THE NUMBER OF BLACK POINTS 
C                 IN THE RED-BLACK MATRIX. (= NNB)
C          IA,JA  INPUT INTEGER VECTORS.  THE TWO INTEGER ARRAYS OF 
C                 THE SPARSE MATRIX REPRESENTATION.       
C          A      INPUT D.P. VECTOR.  THE D.P. ARRAY OF THE SPARSE  
C                 MATRIX REPRESENTATION.
C          RHS    INPUT D.P. VECTOR.  CONTAINS THE RIGHT HAND SIDE  
C                 OF THE MATRIX PROBLEM.
C          UB     INPUT D.P. VECTOR.  CONTAINS THE ESTIMATE FOR THE 
C                 SOLUTION ON THE BLACK POINTS AFTER IN ITERATIONS. 
C          UB1    INPUT/OUTPUT D.P. VECTOR.  ON INPUT, UB1 CONTAINS THE 
C                 SOLUTION VECTOR AFTER IN-1 ITERATIONS.  ON OUTPUT,
C                 IT WILL CONTAIN THE NEWEST ESTIMATE FOR THE SOLUTION
C                 VECTOR.  THIS IS ONLY FOR THE BLACK POINTS.       
C          DB     INPUT D.P. ARRAY.  DB CONTAINS THE VALUE OF THE   
C                 CURRENT PSEUDO-RESIDUAL ON THE BLACK POINTS.      
C       
C ... SPECIFICATIONS FOR ARGUMENTS    
C       
      INTEGER IA(1),JA(1),N,NNB       
      DOUBLE PRECISION A(1),RHS(N),UB(N),UB1(N),DB(NNB)   
C       
C ... SPECIFICATIONS FOR LOCAL VARIABLES
C       
      INTEGER NB,NR,NRP1    
      DOUBLE PRECISION CONST,C1,C2,C3,DNRM      
      LOGICAL Q1  
C       
C ... SPECIFICATIONS FOR FUNCTION SUBPROGRAMS   
C       
      DOUBLE PRECISION DDOT 
      LOGICAL TSTCHG
C       
C *** BEGIN: ITPACK COMMON  
C       
      INTEGER IN,IS,ISYM,ITMAX,LEVEL,NOUT       
      COMMON /ITCOM1/ IN,IS,ISYM,ITMAX,LEVEL,NOUT 
C       
      LOGICAL ADAPT,BETADT,CASEII,HALT,PARTAD   
      COMMON /ITCOM2/ ADAPT,BETADT,CASEII,HALT,PARTAD     
C       
      DOUBLE PRECISION BDELNM,BETAB,CME,DELNNM,DELSNM,FF,GAMMA,OMEGA,QA,
     *   QT,RHO,RRR,SIGE,SME,SPECR,SPR,DRELPR,STPTST,UDNM,ZETA      
      COMMON /ITCOM3/ BDELNM,BETAB,CME,DELNNM,DELSNM,FF,GAMMA,OMEGA,QA, 
     *   QT,RHO,RRR,SIGE,SME,SPECR,SPR,DRELPR,STPTST,UDNM,ZETA      
C       
C *** END  : ITPACK COMMON  
C       
C     DESCRIPTION OF VARIABLES IN COMMON BLOCKS IN SUBROUTINE RSSI  
C       
C ... COMPUTE UR(IN) INTO UB
C       
      NB = NNB    
      NR = N-NB   
      NRP1 = NR+1 
      CALL DCOPY (NR,RHS,1,UB,1)      
      CALL PRSRED (NB,NR,IA,JA,A,UB(NRP1),UB)   
C       
C ... COMPUTE PSEUDO-RESIDUAL, DB(IN) 
C       
      CALL DCOPY (NB,RHS(NRP1),1,DB,1)
      CALL PRSBLK (NB,NR,IA,JA,A,UB,DB) 
      CALL VEVMW (NB,DB,UB(NRP1))     
C       
C ... TEST FOR STOPPING     
C       
      DELNNM = DDOT(NB,DB,1,DB,1)     
      DNRM = DELNNM 
      CONST = CME 
      CALL PSTOP (NB,UB(NRP1),DNRM,CONST,2,Q1)  
      IF (HALT) GO TO 20    
      IF (.NOT.ADAPT) GO TO 10
C       
C ... TEST TO CHANGE PARAMETERS       
C       
      IF (.NOT.TSTCHG(2)) GO TO 10    
C       
C ... CHANGE PARAMETERS     
C       
      CALL VFILL (NR,UB1,0.D0)
      CALL PRSRED (NB,NR,IA,JA,A,DB,UB1)
      DNRM = DDOT(NR,UB1,1,UB1,1)     
      CALL CHGSI (DNRM,2)   
      IF (.NOT.ADAPT) GO TO 10
C       
C ... COMPUTE UB(N+1) AFTER CHANGING PARAMETERS 
C       
      CALL DCOPY (NB,UB(NRP1),1,UB1(NRP1),1)    
      CALL DAXPY (NB,GAMMA,DB,1,UB1(NRP1),1)    
      GO TO 20    
C       
C ... COMPUTE UB(N+1) WITHOUT CHANGE OF PARAMETERS
C       
   10 CALL PARSI (C1,C2,C3,2) 
      CALL SUM3 (NB,C1,DB,C2,UB(NRP1),C3,UB1(NRP1))       
C       
C ... OUTPUT INTERMEDIATE INFORMATION 
C       
   20 CALL ITERM (NB,A(NRP1),UB(NRP1),DB,7)     
C       
      RETURN      
      END 
