/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the class library                   */
/*       SoPlex --- the Sequential object-oriented simPlex.                  */
/*                                                                           */
/*    Copyright (C) 1996-2017 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SoPlex is distributed under the terms of the ZIB Academic Licence.       */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SoPlex; see the file COPYING. If not email to soplex@zib.de.  */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <assert.h>
#include <iostream>
#include <sstream>

#include "spxdefines.h"
#include "soplex.h"
#include "spxpricer.h"
#include "spxratiotester.h"
#include "spxstarter.h"
#include "spxout.h"
#include "timerfactory.h"

namespace soplex
{

  template <class R>
  bool SPxSolver<R>::read(std::istream& in, NameSet* rowNames, 
                          NameSet* colNames, DIdxSet* intVars)
  {
    if( initialized )
      {
        clear();
        unInit();

        if (thepricer)
          thepricer->clear();

        if (theratiotester)
          theratiotester->clear();
      }

    this->unLoad();
    if (!SPxLP::read(in, rowNames, colNames, intVars))
      return false;

    this->theLP = this;

    return true;
  }

  template <class R>
  void SPxSolver<R>::reLoad()
  {
    forceRecompNonbasicValue();
    unInit();
    this->unLoad();
    this->theLP = this;
    m_status = SPxSolver<R>::UNKNOWN;
    if (thepricer)
      thepricer->clear();
    if (theratiotester)
      theratiotester->clear();
  }

  /// #template #temp
  template <>
  void SPxSolver<Real>::loadLP(const SPxLP& lp, bool initSlackBasis)
  {
    clear();
    unInit();
    this->unLoad();
    resetClockStats();
    if (thepricer)
      thepricer->clear();
    if (theratiotester)
      theratiotester->clear();
    SPxLP::operator=(lp);
    reDim();
    SPxBasis<Real>::load(this, initSlackBasis);
  }

  template <class R>
  void SPxSolver<R>::setBasisSolver(SLinSolver* slu, const bool destroy)
  {
    // we need to set the outstream before we load the solver to ensure that the basis
    // can be initialized with this pointer in loadSolver()
    assert(spxout != 0);
    slu->spxout = spxout;
    SPxBasis<R>::loadBasisSolver(slu, destroy);
  }

  template <class R>
  void SPxSolver<R>::loadBasis(const typename SPxBasis<R>::Desc& p_desc)
  {
    unInit();
    if (SPxBasis<R>::status() == SPxBasis<R>::NO_PROBLEM)
      {
        SPxBasis<R>::load(this, false);
      }
    setBasisStatus(SPxBasis<R>::REGULAR);
    SPxBasis<R>::loadDesc(p_desc);
  }

  /// #template #temp
  template <>
  void SPxSolver<Real>::setPricer(SPxPricer<Real>* x, const bool destroy)
  {
   
    assert(!freePricer || thepricer != 0);

    if(freePricer)
      {
        delete thepricer;
        thepricer = 0;
      }

    if (x != 0 && x != thepricer)
      {
        setPricing(FULL);
        if (isInitialized())
          x->load(this);
        else
          x->clear();
      }

    if (thepricer && thepricer != x)
      thepricer->clear();
    thepricer = x;

    freePricer = destroy;
  }

  /// #template #temp 
  template <>
  void SPxSolver<Real>::setTester(SPxRatioTester<Real>* x, const bool destroy)
  {
    assert(!freeRatioTester || theratiotester != 0);

    if(freeRatioTester)
      {
        delete theratiotester;
        theratiotester = 0;
      }

    theratiotester = x;

    // set the solver pointer inside the ratiotester
    if( theratiotester != 0 )
      {
        if( isInitialized() )
          theratiotester->load(this);
        else
          theratiotester->clear();
      }

    freeRatioTester = destroy;
  }

  template <class R>
  void SPxSolver<R>::setStarter(SPxStarter<R>* x, const bool destroy)
  {

    assert(!freeStarter || thestarter != 0);

    if(freeStarter)
      {
        delete thestarter;
        thestarter = 0;
      }
    thestarter = x;

    freeStarter = destroy;
  }

  template <class R>
  void SPxSolver<R>::setType(Type tp)
  {

    if (theType != tp)
      {
        theType = tp;

        forceRecompNonbasicValue();

        unInit();
#if 0
        else
          {
            if (!matrixIsSetup)
              {
                SPxBasis<R>::load(this);
                // SPxBasis<R>::load(desc());
                // not needed, because load(this) allready loads descriptor
              }
            factorized = false;
            m_numCycle = 0;
#endif
            MSG_INFO3( (*spxout), (*spxout) << "Switching to "
                       << static_cast<const char*>((tp == LEAVE)
                                                   ? "leaving" : "entering")
                       << " algorithm" << std::endl; )
              }
      }

    template <class R>
      void SPxSolver<R>::initRep(Representation p_rep)
    {

      Real tmpfeastol = feastol();
      Real tmpopttol = opttol();

      theRep = p_rep;

      if (theRep == COLUMN)
        {
          thevectors   = this->colSet();
          thecovectors = this->rowSet(); 
          theFrhs      = &primRhs;
          theFvec      = &primVec;
          theCoPrhs    = &dualRhs;
          theCoPvec    = &dualVec;
          thePvec      = &addVec;
          theRPvec     = theCoPvec;
          theCPvec     = thePvec;
          theUbound    = &theUCbound;
          theLbound    = &theLCbound;
          theCoUbound  = &theURbound;
          theCoLbound  = &theLRbound;
        }
      else
        {
          assert(theRep == ROW);

          thevectors   = this->rowSet(); 
          thecovectors = this->colSet();
          theFrhs      = &dualRhs;
          theFvec      = &dualVec;
          theCoPrhs    = &primRhs;
          theCoPvec    = &primVec;
          thePvec      = &addVec;
          theRPvec     = thePvec;
          theCPvec     = theCoPvec;
          theUbound    = &theURbound;
          theLbound    = &theLRbound;
          theCoUbound  = &theUCbound;
          theCoLbound  = &theLCbound;
        }
      unInit();
      reDim();

      forceRecompNonbasicValue();

      setFeastol(tmpfeastol);
      setOpttol(tmpopttol);

      SPxBasis<R>::setRep();
      if (SPxBasis<R>::status() > SPxBasis<R>::NO_PROBLEM)
        SPxBasis<R>::loadDesc(this->desc());

      if (thepricer && thepricer->solver() == this)
        thepricer->setRep(p_rep);
    }

    template <class R>
      void SPxSolver<R>::setRep(Representation p_rep)
    {

      if (p_rep != theRep)
        initRep(p_rep);
    }

    // needed for strongbranching. use carefully
    template <class R>
      void SPxSolver<R>::reinitializeVecs()
    {
   
      initialized = true;

      if (type() == ENTER)
        {
          if (rep() == COLUMN)
            setPrimalBounds();
          else
            setDualRowBounds();

          setEnterBounds();
          computeEnterCoPrhs();
        }
      else
        {
          if (rep() == ROW)
            setPrimalBounds();
          else
            setDualColBounds();

          setLeaveBounds();
          computeLeaveCoPrhs();
        }

      SPxBasis<R>::coSolve(*theCoPvec, *theCoPrhs);
      computePvec();
      computeFrhs();
      SPxBasis<R>::solve(*theFvec, *theFrhs);

      theShift  = 0.0;
      lastShift = 0.0;

      if (type() == ENTER)
        {
          computeCoTest();
          computeTest();
        }
      else
        {
          computeFtest();
        }
      assert((testBounds(), 1));
    }

    template <class R>
      void SPxSolver<R>::resetClockStats()
    {
      nClckSkipsLeft = 0;
      nCallsToTimelim = 0;
      theCumulativeTime = 0.0;
    }

    /// #temp #template
    template <>
      void SPxSolver<Real>::init()
    {

      assert(thepricer      != 0);
      assert(theratiotester != 0);

      if (!initialized)
        {
          initialized = true;
          clearUpdateVecs();
          reDim();
          if (SPxBasis<Real>::status() <= SPxBasis<Real>::NO_PROBLEM || this->solver() != this)
            SPxBasis<Real>::load(this);
          initialized = false;
        }
      if (!this->matrixIsSetup)
        SPxBasis<Real>::loadDesc(this->desc());

      // Inna/Tobi: don't "upgrade" a singular basis to a regular one
      if( SPxBasis<Real>::status() == SPxBasis<Real>::SINGULAR )
        return;

      // catch pathological case for LPs with zero constraints
      if( dim() == 0 )
        {
          this->factorized = true;
        }

      // we better factorize explicitly before solving
      if( !this->factorized )
        {
          try
            {
              SPxBasis<Real>::factorize();
            }
          catch( const SPxException& )
            {
              // reload inital slack basis in case the factorization failed
              assert(SPxBasis<Real>::status() <= SPxBasis<Real>::SINGULAR);
              SPxBasis<Real>::restoreInitialBasis();
              SPxBasis<Real>::factorize();
              assert(this->factorized);
            }
        }

      m_numCycle = 0;

      if (type() == ENTER)
        {
          if (rep() == COLUMN)
            {
              setPrimalBounds();
              setBasisStatus(SPxBasis<Real>::PRIMAL);
            }
          else
            {
              setDualRowBounds();
              setBasisStatus(SPxBasis<Real>::DUAL);
            }
          setEnterBounds();
          computeEnterCoPrhs();
          // prepare support vectors for sparse pricing
          infeasibilities.setMax(dim());
          infeasibilitiesCo.setMax(coDim());
          isInfeasible.reSize(dim());
          isInfeasibleCo.reSize(coDim());
          theratiotester->setDelta(entertol());
        }
      else
        {
          if (rep() == ROW)
            {
              setPrimalBounds();
              setBasisStatus(SPxBasis<Real>::PRIMAL);
            }
          else
            {
              setDualColBounds();
              setBasisStatus(SPxBasis<Real>::DUAL);
            }
          setLeaveBounds();
          computeLeaveCoPrhs();
          // prepare support vectors for sparse pricing
          infeasibilities.setMax(dim());
          isInfeasible.reSize(dim());
          theratiotester->setDelta(leavetol());
        }

      SPxBasis<Real>::coSolve(*theCoPvec, *theCoPrhs);
      computePvec();
      computeFrhs();
      SPxBasis<Real>::solve(*theFvec, *theFrhs);

      theShift = 0.0;

      if (type() == ENTER)
        {
          shiftFvec();
          lastShift = theShift + entertol();

          computeCoTest();
          computeTest();
        }
      else
        {
          shiftPvec();
          lastShift = theShift + leavetol();

          computeFtest();
        }

      if (!initialized)
        {
          // if(thepricer->solver() != this)
          thepricer->load(this);
          // if(theratiotester->solver() != this)
          theratiotester->load(this);
          initialized = true;
        }
    }

    template <class R>
      void SPxSolver<R>::setPricing(Pricing pr)
    {
      thePricing = pr;
      if (initialized && type() == ENTER)
        {
          computePvec();
          computeCoTest();
          computeTest();
        }
    }

    template <class R>
      void SPxSolver<R>::setDecompStatus(DecompStatus decomp_stat)
    {
      if( decomp_stat == FINDSTARTBASIS )
        getStartingDecompBasis = true;
      else
        getStartingDecompBasis = false;
    }

    /*
      The following method resizes all vectors and arrays of |SoPlex|
      (excluding inherited vectors).
    */
    template <class R>
      void SPxSolver<R>::reDim()
    {

      int newsize = SPxLP::nCols() > SPxLP::nRows() ? SPxLP::nCols() : SPxLP::nRows();

      if (newsize > unitVecs.size())
        {
          unitVecs.reSize(newsize);

          while (newsize-- > 0)
            unitVecs[newsize] = UnitVector(newsize);
        }

      if (isInitialized())
        {
          theFrhs->reDim(dim());
          theFvec->reDim(dim());
          thePvec->reDim(coDim());

          theCoPrhs->reDim(dim());
          theCoPvec->reDim(dim());

          theTest.reDim(coDim());
          theCoTest.reDim(dim());

          theURbound.reDim(SPxLP::nRows());
          theLRbound.reDim(SPxLP::nRows());
          theUCbound.reDim(SPxLP::nCols());
          theLCbound.reDim(SPxLP::nCols());
          theUBbound.reDim(dim());
          theLBbound.reDim(dim());
        }
    }

    template <class R>
      void SPxSolver<R>::clear()
    {
      unitVecs.reSize(0);

      dualRhs.clear();
      dualVec.clear();
      primRhs.clear();
      primVec.clear();
      addVec.clear();
      theURbound.clear();
      theLRbound.clear();
      theUCbound.clear();
      theLCbound.clear();
      theTest.clear();
      theCoTest.clear();

      forceRecompNonbasicValue();
      unInit();
      SPxLP::clear();
      setBasisStatus(SPxBasis<R>::NO_PROBLEM);
      // clear the basis only when theLP is present, because LP data (nrows, ncols) is used in reDim()
      if( this->theLP != 0 )
        SPxBasis<R>::reDim();

      infeasibilities.clear();
      infeasibilitiesCo.clear();
      isInfeasible.clear();
      isInfeasibleCo.clear();
    }

    /// #template #temp
    template <>
      void SPxSolver<Real>::unscaleLPandReloadBasis()
    {
      SPxLPBase<Real>::unscaleLP();
      SPxBasis<Real>::invalidate();
      unInit();
      init();
    }

    template <class R>
      void SPxSolver<R>::clearUpdateVecs(void)
    {
      theFvec->clearUpdate();
      thePvec->clearUpdate();
      theCoPvec->clearUpdate();
      solveVector2 = 0;
      solveVector3 = 0;
      coSolveVector2 = 0;
      coSolveVector3 = 0;
    }

    /*
      When the basis matrix factorization is recomputed from scratch, 
      we also recompute the vectors.
    */
    template <class R>
      void SPxSolver<R>::factorize()
    {

      MSG_INFO3( (*spxout), (*spxout) << " --- refactorizing basis matrix" << std::endl; )

        try
          {
            SPxBasis<R>::factorize();
          }
        catch( const SPxStatusException& )
          {
            assert(SPxBasis<R>::status() == SPxBasis<R>::SINGULAR);
            m_status = SINGULAR;
            std::stringstream s;
            s << "Basis is singular (numerical troubles, feastol = " << feastol() << ", opttol = " << opttol() << ")";
            throw SPxStatusException(s.str());
          }

      if( !initialized )
        {
          init();  // not sure if init() is neccessary here
          // we must not go on here because not all vectors (e.g. fVec) may be set up correctly
          return;
        }

      if (SPxBasis<R>::status() >= SPxBasis<R>::REGULAR)
        {
#ifndef NDEBUG
          DVector ftmp(fVec());
          DVector ptmp(pVec());
          DVector ctmp(coPvec());
#endif  // NDEBUG

          if (type() == LEAVE)
            {
              /* we have to recompute theFrhs, because roundoff errors can occur during updating, especially when
               * columns/rows with large bounds are present
               */
              computeFrhs();
              SPxBasis<R>::solve (*theFvec, *theFrhs);
              SPxBasis<R>::coSolve(*theCoPvec, *theCoPrhs);

#ifndef NDEBUG
              ftmp -= fVec();
              ptmp -= pVec();
              ctmp -= coPvec();
              if (ftmp.length() > DEFAULT_BND_VIOL)
                {
                  MSG_DEBUG( std::cout << "DSOLVE21 fVec:   " << ftmp.length() << std::endl; )
                    ftmp = fVec();
                  multBaseWith(ftmp);
                  ftmp -= fRhs();
                  if (ftmp.length() > DEFAULT_BND_VIOL)
                    MSG_INFO1( (*spxout), (*spxout) << "ESOLVE29 " << iteration() << ": fVec error = "
                               << ftmp.length() << " exceeding DEFAULT_BND_VIOL = " << DEFAULT_BND_VIOL << std::endl; )
                      }
              if (ctmp.length() > DEFAULT_BND_VIOL)
                {
                  MSG_DEBUG( std::cout << "DSOLVE23 coPvec: " << ctmp.length() << std::endl; )
                    ctmp = coPvec();
                  multWithBase(ctmp);
                  ctmp -= coPrhs();
                  if (ctmp.length() > DEFAULT_BND_VIOL)
                    MSG_INFO1( (*spxout), (*spxout) << "ESOLVE30 " << iteration() << ": coPvec error = "
                               << ctmp.length() << " exceeding DEFAULT_BND_VIOL = " << DEFAULT_BND_VIOL << std::endl; )
                      }
              if (ptmp.length() > DEFAULT_BND_VIOL)
                {
                  MSG_DEBUG( std::cout << "DSOLVE24 pVec:   " << ptmp.length() << std::endl; )
                    }
#endif  // NDEBUG

              computeFtest();
            }
          else
            {
              assert(type() == ENTER);

              SPxBasis<R>::coSolve(*theCoPvec, *theCoPrhs);
              computeCoTest();

              if (pricing() == FULL)
                {
                  /* to save time only recompute the row activities (in row rep) when we are already nearly optimal to
                   * avoid missing any violations from previous updates */
                  if( rep() == ROW && m_pricingViolCo < entertol() && m_pricingViol < entertol() )
                    computePvec();

                  /* was deactivated, but this leads to warnings in testVecs() */
                  computeTest();
                }
            }
        }

#ifdef ENABLE_ADDITIONAL_CHECKS
      /* moved this test after the computation of fTest and coTest below, since these vectors might not be set up at top, e.g. for an initial basis */
      if (SPxBasis<R>::status() > SPxBasis<R>::SINGULAR)
        testVecs();
#endif
    }

    /* We compute how much the current solution violates (primal or dual) feasibility. In the
       row/enter or column/leave algorithm the maximum violation of dual feasibility is
       computed. In the row/leave or column/enter algorithm the primal feasibility is checked.
       Additionally, the violation from pricing is taken into account. */
    template <class R>
      Real SPxSolver<R>::maxInfeas() const
    {
      Real inf = 0.0;

      if (type() == ENTER)
        {
          if( m_pricingViolUpToDate && m_pricingViolCoUpToDate )
            inf = m_pricingViol + m_pricingViolCo;

          for (int i = 0; i < dim(); i++)
            {
              if ((*theFvec)[i] > theUBbound[i])
                inf = MAXIMUM(inf, (*theFvec)[i] - theUBbound[i]);
              else if ((*theFvec)[i] < theLBbound[i])
                inf = MAXIMUM(inf, theLBbound[i] - (*theFvec)[i]);
            }
        }
      else
        {
          assert(type() == LEAVE);

          if( m_pricingViolUpToDate )
            inf = m_pricingViol;

          for (int i = 0; i < dim(); i++)
            {
              if ((*theCoPvec)[i] > (*theCoUbound)[i])
                inf = MAXIMUM(inf, (*theCoPvec)[i] - (*theCoUbound)[i]);
              else if ((*theCoPvec)[i] < (*theCoLbound)[i])
                inf = MAXIMUM(inf, (*theCoLbound)[i] - (*theCoPvec)[i]);
            }
          for (int i = 0; i < coDim(); i++)
            {
              if ((*thePvec)[i] > (*theUbound)[i])
                inf = MAXIMUM(inf, (*thePvec)[i] - (*theUbound)[i]);
              else if ((*thePvec)[i] < (*theLbound)[i])
                inf = MAXIMUM(inf, (*theLbound)[i] - (*thePvec)[i]);
            }
        }

      return inf;
    }

    /* check for (dual) violations above tol and immediately return false w/o checking the remaining values
       This method is useful for verifying whether an objective limit can be used as termination criterion */
    template <class R>
      bool SPxSolver<R>::noViols(Real tol) const
    {
      assert(tol >= 0.0);

      if( type() == ENTER )
        {
          for( int i = 0; i < dim(); i++ )
            {
              if( (*theFvec)[i] - theUBbound[i] > tol )
                return false;
              if( theLBbound[i] - (*theFvec)[i] > tol )
                return false;
            }
        }
      else
        {
          assert(type() == LEAVE);

          for( int i = 0; i < dim(); i++ )
            {
              if( (*theCoPvec)[i] - (*theCoUbound)[i] > tol )
                return false;
              if( (*theCoLbound)[i] - (*theCoPvec)[i] > tol )
                return false;
            }
          for (int i = 0; i < coDim(); i++)
            {
              if( (*thePvec)[i] - (*theUbound)[i] > tol )
                return false;
              if( (*theLbound)[i] - (*thePvec)[i] > tol )
                return false;
            }
        }
      return true;
    }

    template <class R>
      Real SPxSolver<R>::nonbasicValue()
    {
      int i;
      Real val = 0;
      const typename SPxBasis<R>::Desc& ds = this->desc();

#ifndef ENABLE_ADDITIONAL_CHECKS
      // if the value is available we don't need to recompute it
      if ( m_nonbasicValueUpToDate )
        return m_nonbasicValue;
#endif

      if (rep() == COLUMN)
        {
          if (type() == LEAVE)
            {
              for (i = this->nCols() - 1; i >= 0; --i)
                {
                  switch (ds.colStatus(i))
                    {
                    case SPxBasis<R>::Desc::P_ON_UPPER :
                      val += theUCbound[i] * SPxLP::upper(i);
                      //@ val += maxObj(i) * SPxLP::upper(i);
                      break;
                    case SPxBasis<R>::Desc::P_ON_LOWER :
                      val += theLCbound[i] * SPxLP::lower(i);
                      //@ val += maxObj(i) * SPxLP::lower(i);
                      break;
                    case SPxBasis<R>::Desc::P_FIXED :
                      val += this->maxObj(i) * SPxLP::lower(i);
                      break;
                    default:
                      break;
                    }
                }
              for (i = this->nRows() - 1; i >= 0; --i)
                {
                  switch (ds.rowStatus(i))
                    {
                    case SPxBasis<R>::Desc::P_ON_UPPER :
                      val += theLRbound[i] * SPxLP::rhs(i);
                      break;
                    case SPxBasis<R>::Desc::P_ON_LOWER :
                      val += theURbound[i] * SPxLP::lhs(i);
                      break;
                    case SPxBasis<R>::Desc::P_FIXED :
                      val += this->maxRowObj(i) * SPxLP::lhs(i);
                      break;
                    default:
                      break;
                    }
                }
            }
          else
            {
              assert(type() == ENTER);
              for (i = this->nCols() - 1; i >= 0; --i)
                {
                  switch (ds.colStatus(i))
                    {
                    case SPxBasis<R>::Desc::P_ON_UPPER :
                      val += this->maxObj(i) * theUCbound[i];
                      break;
                    case SPxBasis<R>::Desc::P_ON_LOWER :
                      val += this->maxObj(i) * theLCbound[i];
                      break;
                    case SPxBasis<R>::Desc::P_FIXED :
                      assert(theLCbound[i] == theUCbound[i]);
                      val += this->maxObj(i) * theLCbound[i];
                      break;
                    default:
                      break;
                    }
                }
              for (i = this->nRows() - 1; i >= 0; --i)
                {
                  switch (ds.rowStatus(i))
                    {
                    case SPxBasis<R>::Desc::P_ON_UPPER :
                      val += this->maxRowObj(i) * theLRbound[i];
                      break;
                    case SPxBasis<R>::Desc::P_ON_LOWER :
                      val += this->maxRowObj(i) * theURbound[i];
                      break;
                    case SPxBasis<R>::Desc::P_FIXED :
                      val += this->maxRowObj(i) * theURbound[i];
                      break;
                    default:
                      break;
                    }
                }
            }
        }
      else
        {
          assert(rep() == ROW);
          assert(type() == ENTER);
          for (i = this->nCols() - 1; i >= 0; --i)
            {
              switch (ds.colStatus(i))
                {
                case SPxBasis<R>::Desc::D_ON_UPPER :
                  val += theUCbound[i] * this->lower(i);
                  break;
                case SPxBasis<R>::Desc::D_ON_LOWER :
                  val += theLCbound[i] * this->upper(i);
                  break;
                case SPxBasis<R>::Desc::D_ON_BOTH :
                  val += theLCbound[i] * this->upper(i);
                  val += theUCbound[i] * this->lower(i);
                  break;
                default:
                  break;
                }
            }
          for (i = this->nRows() - 1; i >= 0; --i)
            {
              switch (ds.rowStatus(i))
                {
                case SPxBasis<R>::Desc::D_ON_UPPER :
                  val += theURbound[i] * this->lhs(i);
                  break;
                case SPxBasis<R>::Desc::D_ON_LOWER :
                  val += theLRbound[i] * this->rhs(i);
                  break;
                case SPxBasis<R>::Desc::D_ON_BOTH :
                  val += theLRbound[i] * this->rhs(i);
                  val += theURbound[i] * this->lhs(i);
                  break;
                default:
                  break;
                }
            }
        }

#ifdef ENABLE_ADDITIONAL_CHECKS
      if( m_nonbasicValueUpToDate && NE(m_nonbasicValue, val) )
        {
          MSG_ERROR( std::cerr << "stored nonbasic value: " << m_nonbasicValue
                     << ", correct nonbasic value: " << val
                     << ", violation: " << val - m_nonbasicValue << std::endl; )
            assert(EQrel(m_nonbasicValue, val, 1e-12));
        }
#endif

      if( !m_nonbasicValueUpToDate )
        {
          m_nonbasicValue = val;
          m_nonbasicValueUpToDate = true;
        }

      return val;
    }

    template <class R>
      Real SPxSolver<R>::value()
    {
      assert(isInitialized());

      Real x;

      // calling value() without having a suitable status is an error.
      if (!isInitialized())
        return infinity;

      if (rep() == ROW)
        {
          if (type() == LEAVE)
            x = SPxLP::spxSense() * (coPvec() * fRhs()); // the contribution of maxRowObj() is missing
          else
            x = SPxLP::spxSense() * (nonbasicValue() + (coPvec() * fRhs()));
        }
      else
        x = SPxLP::spxSense() * (nonbasicValue() + fVec() * coPrhs());

      return x + this->objOffset();
    }

    template <class R>
      bool SPxSolver<R>::updateNonbasicValue(Real objChange)
    {
      if( m_nonbasicValueUpToDate )
        m_nonbasicValue += objChange;

      MSG_DEBUG( std::cout
                 << "Iteration: " << iteration()
                 << ": updated objValue: " << objChange
                 << ", new value: " << m_nonbasicValue
                 << ", correct value: " << nonbasicValue()
                 << std::endl;
                 )

        return m_nonbasicValueUpToDate;
    }



    template <class R>
      void SPxSolver<R>::setFeastol(Real d)
    {

      if( d <= 0.0 )
        throw SPxInterfaceException("XSOLVE30 Cannot set feastol less than or equal to zero.");

      if( theRep == COLUMN )
        m_entertol = d;
      else
        m_leavetol = d;
    }

    template <class R>
      void SPxSolver<R>::setOpttol(Real d)
    {

      if( d <= 0.0 )
        throw SPxInterfaceException("XSOLVE31 Cannot set opttol less than or equal to zero.");

      if( theRep == COLUMN )
        m_leavetol = d;
      else
        m_entertol = d;
    }

    template <class R>
      void SPxSolver<R>::setDelta(Real d)
    {

      if( d <= 0.0 )
        throw SPxInterfaceException("XSOLVE32 Cannot set delta less than or equal to zero.");

      m_entertol = d;
      m_leavetol = d;
    }

    template <class R>
      void SPxSolver<R>::hyperPricing(bool h)
    {
      hyperPricingEnter = h;
      hyperPricingLeave = h;
      if( h )
        {
          updateViols.setMax(dim());
          updateViolsCo.setMax(coDim());
        }
    }

    template <class R>
      SPxSolver<R>::SPxSolver(
                              Type            p_type, 
                              Representation  p_rep,
                              Timer::TYPE     ttype)
      : theType (p_type)
      , thePricing(FULL)
      , theRep(p_rep)
      , polishObj(POLISH_OFF)
      , theTime(0)
      , timerType(ttype)
      , theCumulativeTime(0.0)
      , maxIters (-1)
      , maxTime (infinity)
      , nClckSkipsLeft(0)
      , nCallsToTimelim(0)
      , objLimit(infinity)
      , m_status(UNKNOWN)
      , m_nonbasicValue(0.0)
      , m_nonbasicValueUpToDate(false)
      , m_pricingViol(0.0)
      , m_pricingViolUpToDate(false)
      , m_pricingViolCo(0.0)
      , m_pricingViolCoUpToDate(false)
      , theShift (0)
      , m_maxCycle(100)
      , m_numCycle(0)
      , initialized (false)
      , solveVector2 (0)
      , solveVector3 (0)
      , coSolveVector2(0)
      , coSolveVector3(0)
      , freePricer (false)
      , freeRatioTester (false)
      , freeStarter (false)
      , displayLine (0)
      , displayFreq (200)
      , sparsePricingFactor(SPARSITYFACTOR)
      , getStartingDecompBasis(false)
      , computeDegeneracy(false)
      , degenCompIterOffset(0)
      , fullPerturbation(false)
      , printCondition(0)
      , unitVecs (0)
      , primVec (0, Param::epsilon())
      , dualVec (0, Param::epsilon())
      , addVec (0, Param::epsilon())
      , thepricer (0)
      , theratiotester (0)
      , thestarter (0)
      , infeasibilities(0)
      , infeasibilitiesCo(0)
      , isInfeasible(0)
      , isInfeasibleCo(0)
      , sparsePricingLeave(false)
      , sparsePricingEnter(false)
      , sparsePricingEnterCo(false)
      , hyperPricingLeave(true)
      , hyperPricingEnter(true)
      , remainingRoundsLeave(0)
      , remainingRoundsEnter(0)
      , remainingRoundsEnterCo(0)
      , weights(0)
      , coWeights(0)
      , weightsAreSetup(false)
      , integerVariables(0)
      {
        theTime = TimerFactory::createTimer(timerType);

        setDelta(DEFAULT_BND_VIOL);

        this->theLP = this;
        initRep(p_rep);

        // info: SPxBasis is not consistent in this moment.
        //assert(SPxSolver<R>::isConsistent());
      }

    template <class R>
      SPxSolver<R>::~SPxSolver()
    {
      assert(!freePricer || thepricer != 0);
      assert(!freeRatioTester || theratiotester != 0);
      assert(!freeStarter || thestarter != 0);

      if(freePricer)
        {
          delete thepricer;
          thepricer = 0;
        }

      if(freeRatioTester)
        {
          delete theratiotester;
          theratiotester = 0;
        }

      if(freeStarter)
        {
          delete thestarter;
          thestarter = 0;
        }

      // free timer
      assert(theTime);
      theTime->~Timer();
      spx_free(theTime);
    }


    template <class R>
      SPxSolver<R>& SPxSolver<R>::operator=(const SPxSolver<R>& base)
      {
        if(this != &base)
          {
            SPxLP::operator=(base);
            SPxBasis<R>::operator=(base);
            theType = base.theType;
            thePricing = base.thePricing;
            theRep = base.theRep;
            polishObj = base.polishObj;
            timerType = base.timerType;
            maxIters = base.maxIters;
            maxTime = base.maxTime;
            objLimit = base.objLimit;
            m_status = base.m_status;
            m_nonbasicValue = base.m_nonbasicValue;
            m_nonbasicValueUpToDate = base.m_nonbasicValueUpToDate;
            m_pricingViol = base.m_pricingViol;
            m_pricingViolUpToDate = base.m_pricingViolUpToDate;
            m_pricingViolCo = base.m_pricingViolCo;
            m_pricingViolCoUpToDate = base.m_pricingViolCoUpToDate;
            m_entertol = base.m_entertol;
            m_leavetol = base.m_leavetol;
            theShift = base.theShift;
            lastShift = base.lastShift;
            m_maxCycle = base.m_maxCycle;
            m_numCycle = base.m_numCycle;
            initialized = base.initialized;
            instableLeaveNum = base.instableLeaveNum;
            instableLeave = base.instableLeave;
            instableLeaveVal = base.instableLeaveVal;
            instableEnterId = base.instableEnterId;
            instableEnter = base.instableEnter;
            instableEnterVal = base.instableEnterVal;
            displayLine = base.displayLine;
            displayFreq = base.displayFreq;
            sparsePricingFactor = base.sparsePricingFactor;
            getStartingDecompBasis = base.getStartingDecompBasis;
            computeDegeneracy = base.computeDegeneracy;
            degenCompIterOffset = base.degenCompIterOffset;
            decompIterationLimit = base.decompIterationLimit;
            fullPerturbation = base.fullPerturbation;
            printCondition = base.printCondition;
            unitVecs = base.unitVecs;
            primRhs = base.primRhs;
            primVec = base.primVec;
            dualRhs = base.dualRhs;
            dualVec = base.dualVec;
            addVec = base.addVec;
            theURbound = base.theURbound;
            theLRbound = base.theLRbound;
            theUCbound = base.theUCbound;
            theLCbound = base.theLCbound;
            theUBbound = base.theUBbound;
            theLBbound = base.theLBbound;
            theCoTest = base.theCoTest;
            theTest = base.theTest;
            primalRay = base.primalRay;
            dualFarkas = base.dualFarkas;
            leaveCount = base.leaveCount;
            enterCount = base.enterCount;
            theCumulativeTime = base.theCumulativeTime;
            primalCount = base.primalCount;
            polishCount = base.polishCount;
            boundflips = base.boundflips;
            totalboundflips = base.totalboundflips;
            enterCycles = base.enterCycles;
            leaveCycles = base.leaveCycles;
            enterDegenCand = base.enterDegenCand;
            leaveDegenCand = base.leaveDegenCand;
            primalDegenSum = base.primalDegenSum;
            infeasibilities = base.infeasibilities;
            infeasibilitiesCo = base.infeasibilitiesCo;
            isInfeasible = base.isInfeasible;
            isInfeasibleCo = base.isInfeasibleCo;
            sparsePricingLeave = base.sparsePricingLeave;
            sparsePricingEnter = base.sparsePricingEnter;
            sparsePricingEnterCo = base.sparsePricingEnterCo;
            sparsePricingFactor = base.sparsePricingFactor;
            hyperPricingLeave = base.hyperPricingLeave;
            hyperPricingEnter = base.hyperPricingEnter;
            remainingRoundsLeave = base.remainingRoundsLeave;
            remainingRoundsEnter = base.remainingRoundsEnter;
            remainingRoundsEnterCo = base.remainingRoundsEnterCo;
            weights = base.weights;
            coWeights = base.coWeights;
            weightsAreSetup = base.weightsAreSetup;
            spxout = base.spxout;
            integerVariables = base.integerVariables;

            if (base.theRep == COLUMN)
              {
                thevectors   = this->colSet();
                thecovectors = this->rowSet(); 
                theFrhs      = &primRhs;
                theFvec      = &primVec;
                theCoPrhs    = &dualRhs;
                theCoPvec    = &dualVec;
                thePvec      = &addVec;
                theRPvec     = theCoPvec;
                theCPvec     = thePvec;
                theUbound    = &theUCbound;
                theLbound    = &theLCbound;
                theCoUbound  = &theURbound;
                theCoLbound  = &theLRbound;
              }
            else
              {
                assert(base.theRep == ROW);
         
                thevectors   = this->rowSet(); 
                thecovectors = this->colSet();
                theFrhs      = &dualRhs;
                theFvec      = &dualVec;
                theCoPrhs    = &primRhs;
                theCoPvec    = &primVec;
                thePvec      = &addVec;
                theRPvec     = thePvec;
                theCPvec     = theCoPvec;
                theUbound    = &theURbound;
                theLbound    = &theLRbound;
                theCoUbound  = &theUCbound;
                theCoLbound  = &theLCbound;
              }

            SPxBasis<R>::theLP = this;

            assert(!freePricer || thepricer != 0);
            assert(!freeRatioTester || theratiotester != 0);
            assert(!freeStarter || thestarter != 0);

            // thepricer
            if(freePricer)
              {
                delete thepricer;
                thepricer = 0;
              }
            if(base.thepricer == 0)
              {
                thepricer = 0;
                freePricer = false;
              }
            else
              {
                thepricer = base.thepricer->clone();
                freePricer = true;
                thepricer->load(this);
              }

            // theratiotester
            if(freeRatioTester)
              {
                delete theratiotester;
                theratiotester = 0;
              }
            if(base.theratiotester == 0)
              {
                theratiotester = 0;
                freeRatioTester = false;
              }
            else
              {
                theratiotester = base.theratiotester->clone();
                freeRatioTester = true;
                theratiotester->load(this);
              }

            // thestarter
            if(freeStarter)
              {
                delete thestarter;
                thestarter = 0;
              }
            if(base.thestarter == 0)
              {
                thestarter = 0;
                freeStarter = false;
              }
            else
              {
                thestarter = base.thestarter->clone();
                freeStarter = true;
              }

            assert(SPxSolver<R>::isConsistent());
          }

        return *this;
      }


    template <class R>
      SPxSolver<R>::SPxSolver(const SPxSolver<R>& base)
      : SPxLP (base)
      , SPxBasis<R>(this->basSe)
      , theType(base.theType)
      , thePricing(base.thePricing)
      , theRep(base.theRep)
      , polishObj(base.polishObj)
      , timerType(base.timerType)
      , theCumulativeTime(base.theCumulativeTime)
      , maxIters(base.maxIters)
      , maxTime(base.maxTime)
      , nClckSkipsLeft(base.nClckSkipsLeft)
      , nCallsToTimelim(base.nCallsToTimelim)
      , objLimit(base.objLimit)
      , m_status(base.m_status)
      , m_nonbasicValue(base.m_nonbasicValue)
      , m_nonbasicValueUpToDate(base.m_nonbasicValueUpToDate)
      , m_pricingViol(base.m_pricingViol)
      , m_pricingViolUpToDate(base.m_pricingViolUpToDate)
      , m_pricingViolCo(base.m_pricingViolCo)
      , m_pricingViolCoUpToDate(base.m_pricingViolCoUpToDate)
      , m_entertol(base.m_entertol)
      , m_leavetol(base.m_leavetol)
      , theShift(base.theShift)
      , lastShift(base.lastShift)
      , m_maxCycle(base.m_maxCycle)
      , m_numCycle(base.m_numCycle)
      , initialized(base.initialized)
      , solveVector2 (0)
      , solveVector2rhs(base.solveVector2rhs)
      , solveVector3 (0)
      , solveVector3rhs(base.solveVector3rhs)
      , coSolveVector2(0)
      , coSolveVector2rhs(base.coSolveVector2rhs)
      , coSolveVector3(0)
      , coSolveVector3rhs(base.coSolveVector3rhs)
      , instableLeaveNum(base.instableLeaveNum)
      , instableLeave(base.instableLeave)
      , instableLeaveVal(base.instableLeaveVal)
      , instableEnterId(base.instableEnterId)
      , instableEnter(base.instableEnter)
      , instableEnterVal(base.instableEnterVal)
      , displayLine(base.displayLine)
      , displayFreq(base.displayFreq)
      , sparsePricingFactor(base.sparsePricingFactor)
      , getStartingDecompBasis(base.getStartingDecompBasis)
      , computeDegeneracy(base.computeDegeneracy)
      , degenCompIterOffset(base.degenCompIterOffset)
      , decompIterationLimit(base.decompIterationLimit)
      , fullPerturbation(base.fullPerturbation)
      , printCondition(base.printCondition)
      , unitVecs(base.unitVecs)
      , primRhs(base.primRhs)
      , primVec(base.primVec)
      , dualRhs(base.dualRhs)
      , dualVec(base.dualVec)
      , addVec(base.addVec)
      , theURbound(base.theURbound)
      , theLRbound(base.theLRbound)
      , theUCbound(base.theUCbound)
      , theLCbound(base.theLCbound)
      , theUBbound(base.theUBbound)
      , theLBbound(base.theLBbound)
      , theCoTest(base.theCoTest)
      , theTest(base.theTest)
      , primalRay(base.primalRay)
      , dualFarkas(base.dualFarkas)
      , leaveCount(base.leaveCount)
      , enterCount(base.enterCount)
      , primalCount(base.primalCount)
      , polishCount(base.polishCount)
      , boundflips(base.boundflips)
      , totalboundflips(base.totalboundflips)
      , enterCycles(base.enterCycles)
      , leaveCycles(base.leaveCycles)
      , enterDegenCand(base.enterDegenCand)
      , leaveDegenCand(base.leaveDegenCand)
      , primalDegenSum(base.primalDegenSum)
      , dualDegenSum(base.dualDegenSum)
      , infeasibilities(base.infeasibilities)
      , infeasibilitiesCo(base.infeasibilitiesCo)
      , isInfeasible(base.isInfeasible)
      , isInfeasibleCo(base.isInfeasibleCo)
      , sparsePricingLeave(base.sparsePricingLeave)
      , sparsePricingEnter(base.sparsePricingEnter)
      , sparsePricingEnterCo(base.sparsePricingEnterCo)
      , hyperPricingLeave(base.hyperPricingLeave)
      , hyperPricingEnter(base.hyperPricingEnter)
      , remainingRoundsLeave(base.remainingRoundsLeave)
      , remainingRoundsEnter(base.remainingRoundsEnter)
      , remainingRoundsEnterCo(base.remainingRoundsEnterCo)
      , weights(base.weights)
      , coWeights(base.coWeights)
      , weightsAreSetup(base.weightsAreSetup)
      , spxout(base.spxout)
      , integerVariables(base.integerVariables)
      {
        theTime = TimerFactory::createTimer(timerType);

        if (base.theRep == COLUMN)
          {
            thevectors   = this->colSet();
            thecovectors = this->rowSet(); 
            theFrhs      = &primRhs;
            theFvec      = &primVec;
            theCoPrhs    = &dualRhs;
            theCoPvec    = &dualVec;
            thePvec      = &addVec;
            theRPvec     = theCoPvec;
            theCPvec     = thePvec;
            theUbound    = &theUCbound;
            theLbound    = &theLCbound;
            theCoUbound  = &theURbound;
            theCoLbound  = &theLRbound;
          }
        else
          {
            assert(base.theRep == ROW);

            thevectors   = this->rowSet(); 
            thecovectors = this->colSet();
            theFrhs      = &dualRhs;
            theFvec      = &dualVec;
            theCoPrhs    = &primRhs;
            theCoPvec    = &primVec;
            thePvec      = &addVec;
            theRPvec     = thePvec;
            theCPvec     = theCoPvec;
            theUbound    = &theURbound;
            theLbound    = &theLRbound;
            theCoUbound  = &theUCbound;
            theCoLbound  = &theLCbound;
          }

        SPxBasis<R>::theLP = this;

        if(base.thepricer == 0)
          {
            thepricer = 0;
            freePricer = false;
          }
        else
          {
            thepricer = base.thepricer->clone();
            freePricer = true;
            thepricer->clear();
            thepricer->load(this);
          }

        if(base.theratiotester == 0)
          {
            theratiotester = 0;
            freeRatioTester = false;
          }
        else
          {
            theratiotester = base.theratiotester->clone();
            freeRatioTester = true;
            theratiotester->clear();
            theratiotester->load(this);
          }

        if(base.thestarter == 0)
          {
            thestarter = 0;
            freeStarter = false;
          }
        else
          {
            thestarter = base.thestarter->clone();
            freeStarter = true;
          }

        assert(SPxSolver<R>::isConsistent());
      }

    template <class R>
      bool SPxSolver<R>::isConsistent() const
    {
#ifdef ENABLE_CONSISTENCY_CHECKS
      if (epsilon() < 0)
        return MSGinconsistent("SPxSolver");

      if (primVec.delta().getEpsilon() != dualVec.delta().getEpsilon())
        return MSGinconsistent("SPxSolver");

      if (dualVec.delta().getEpsilon() != addVec.delta().getEpsilon())
        return MSGinconsistent("SPxSolver");

      if (unitVecs.size() < SPxLP::this->nCols() || unitVecs.size() < SPxLP::this->nRows())
        return MSGinconsistent("SPxSolver");

      if (initialized)
        {
          if (theFrhs->dim() != dim())
            return MSGinconsistent("SPxSolver");
          if (theFvec->dim() != dim())
            return MSGinconsistent("SPxSolver");

          if (theCoPrhs->dim() != dim())
            return MSGinconsistent("SPxSolver");
          if (thePvec->dim() != coDim())
            return MSGinconsistent("SPxSolver");
          if (theCoPvec->dim() != dim())
            return MSGinconsistent("SPxSolver");

          if (theTest.dim() != coDim())
            return MSGinconsistent("SPxSolver");
          if (theCoTest.dim() != dim())
            return MSGinconsistent("SPxSolver");

          if (theURbound.dim() != SPxLP::this->nRows())
            return MSGinconsistent("SPxSolver");
          if (theLRbound.dim() != SPxLP::this->nRows())
            return MSGinconsistent("SPxSolver");
          if (theUCbound.dim() != SPxLP::this->nCols())
            return MSGinconsistent("SPxSolver");
          if (theLCbound.dim() != SPxLP::this->nCols())
            return MSGinconsistent("SPxSolver");
          if (theUBbound.dim() != dim())
            return MSGinconsistent("SPxSolver");
          if (theLBbound.dim() != dim())
            return MSGinconsistent("SPxSolver");
        }

      if (rep() == COLUMN)
        {
          if(thecovectors != 
             reinterpret_cast<const SVSet*>(static_cast<const LPRowSet*>(this)) 
             || thevectors != 
             reinterpret_cast<const SVSet*>(static_cast<const LPColSet*>(this)) 
             || theFrhs != &primRhs ||
             theFvec != &primVec ||
             theCoPrhs != &dualRhs ||
             theCoPvec != &dualVec ||
             thePvec != &addVec ||
             theRPvec != theCoPvec ||
             theCPvec != thePvec ||
             theUbound != &theUCbound ||
             theLbound != &theLCbound ||
             theCoUbound != &theURbound ||
             theCoLbound != &theLRbound)
            return MSGinconsistent("SPxSolver");
        }
      else
        {
          if (thecovectors 
              != reinterpret_cast<const SVSet*>(static_cast<const LPColSet*>(this))
              || thevectors 
              != reinterpret_cast<const SVSet*>(static_cast<const LPRowSet*>(this))
              || theFrhs != &dualRhs ||
              theFvec != &dualVec ||
              theCoPrhs != &primRhs ||
              theCoPvec != &primVec ||
              thePvec != &addVec ||
              theRPvec != thePvec ||
              theCPvec != theCoPvec ||
              theUbound != &theURbound ||
              theLbound != &theLRbound ||
              theCoUbound != &theUCbound ||
              theCoLbound != &theLCbound)
            return MSGinconsistent("SPxSolver");
        }

      return SPxLP::isConsistent()
        && primRhs.isConsistent()
        && primVec.isConsistent()
        && dualRhs.isConsistent()
        && dualVec.isConsistent()
        && addVec.isConsistent()
        && theTest.isConsistent()
        && theCoTest.isConsistent()
        && theURbound.isConsistent()
        && theLRbound.isConsistent()
        && theUCbound.isConsistent()
        && theLCbound.isConsistent()
        && SPxBasis<R>::isConsistent()
        ;
#else
      return true;
#endif
    }


    template <class R>
      void SPxSolver<R>::setTerminationTime(Real p_time)
    {
      if( p_time < 0.0 )
        p_time = 0.0;
      maxTime = p_time;
    }

    template <class R>
      Real SPxSolver<R>::terminationTime() const
    {
      return maxTime;
    }

    template <class R>
      void SPxSolver<R>::setTerminationIter(int p_iteration)
    {
      if( p_iteration < 0 )
        p_iteration = -1;
      maxIters = p_iteration;
    }

    template <class R>
      int SPxSolver<R>::terminationIter() const
    {
      return maxIters;
    }

    // returns whether current time limit is reached; call to time() may be skipped unless \p forceCheck is true
    template <class R>
      bool SPxSolver<R>::isTimeLimitReached(const bool forceCheck)
    {
      // always update the number of calls, since the user might set a time limit later in the solving process
      ++nCallsToTimelim;

      // check if a time limit is actually set
      if( maxTime >= infinity )
        return false;

      // check if the expensive system call to update the time should be skipped again
      if( forceCheck || nCallsToTimelim < NINITCALLS ||  nClckSkipsLeft <= 0 )
        {
          Real currtime = time();

          if( currtime >= maxTime )
            return true;

          // determine the number of times the clock can be skipped again.
          int nClckSkips = MAXNCLCKSKIPS;
          Real avgtimeinterval = (currtime + cumulativeTime()) / (Real)(nCallsToTimelim);

          // it would not be safe to skip the clock so many times since we are approaching the time limit
          if( SAFETYFACTOR * (maxTime - currtime) / (avgtimeinterval + 1e-6) < nClckSkips )
            nClckSkips = 0;
          nClckSkipsLeft = nClckSkips;
        }
      else
        --nClckSkipsLeft;

      return false;
    }


    /**@todo A first version for the termination value is
     *       implemented. Currently we check if no bound violations (shifting)
     *       is present. It might be even possible to use this termination
     *       value in case of bound violations (shifting) but in this case it
     *       is quite difficult to determine if we already reached the limit.
     */
    template <>
      void SPxSolver<Real>::setTerminationValue(Real p_value)
    {
      objLimit = p_value;
    }

    template <class R>
      Real SPxSolver<R>::terminationValue() const
    {
      return objLimit;
    }
   
    template <class R>
      typename SPxSolver<R>::VarStatus
      SPxSolver<R>::basisStatusToVarStatus( typename SPxBasis<R>::Desc::Status stat ) const
    {
      VarStatus vstat;

      switch( stat )
        {
        case SPxBasis<R>::Desc::P_ON_LOWER:
          vstat = ON_LOWER;
          break;
        case SPxBasis<R>::Desc::P_ON_UPPER:
          vstat = ON_UPPER;
          break;
        case SPxBasis<R>::Desc::P_FIXED:
          vstat = FIXED;
          break;
        case SPxBasis<R>::Desc::P_FREE:
          vstat = ZERO;
          break;
        case SPxBasis<R>::Desc::D_ON_UPPER:
        case SPxBasis<R>::Desc::D_ON_LOWER:
        case SPxBasis<R>::Desc::D_ON_BOTH:
        case SPxBasis<R>::Desc::D_UNDEFINED:
        case SPxBasis<R>::Desc::D_FREE:
          vstat = BASIC;
          break;
        default:
          MSG_ERROR( std::cerr << "ESOLVE26 ERROR: unknown basis status (" << stat << ")"
                     << std::endl; )
            throw SPxInternalCodeException("XSOLVE22 This should never happen.");
        }
      return vstat;
    }

    template <class R>
      typename SPxBasis<R>::Desc::Status
      SPxSolver<R>::varStatusToBasisStatusRow( int row, SPxSolver<R>::VarStatus stat ) const
    {
      typename SPxBasis<R>::Desc::Status rstat;

      switch( stat )
        {
        case FIXED :
          assert(this->rhs(row) == this->lhs(row));
          rstat = SPxBasis<R>::Desc::P_FIXED;
          break;
        case ON_UPPER :
          assert(this->rhs(row) < infinity);
          rstat = this->lhs(row) < this->rhs(row)
            ? SPxBasis<R>::Desc::P_ON_UPPER
            : SPxBasis<R>::Desc::P_FIXED;
          break;
        case ON_LOWER :
          assert(this->lhs(row) > -infinity);
          rstat = this->lhs(row) < this->rhs(row)
            ? SPxBasis<R>::Desc::P_ON_LOWER
            : SPxBasis<R>::Desc::P_FIXED;
          break;
        case ZERO :
          /* A 'free' row (i.e., infinite lower & upper bounds) does not really make sense. The user
           * might (think to) know better, e.g., when temporarily turning off a row. We therefore apply
           * the same adjustment as in the column case in varStatusToBasisStatusCol(). */
          if (this->lhs(row) <= -infinity && this->rhs(row) >= infinity)
            rstat = SPxBasis<R>::Desc::P_FREE;
          else
            {
              if ( this->lhs(row) == this->rhs(row) )
                {
                  assert( this->rhs(row) < infinity );
                  rstat = SPxBasis<R>::Desc::P_FIXED;
                }
              else
                {
                  if ( this->lhs(row) > -infinity )
                    rstat = SPxBasis<R>::Desc::P_ON_LOWER;
                  else
                    {
                      assert( this->rhs(row) < infinity );
                      rstat = SPxBasis<R>::Desc::P_ON_UPPER;
                    }
                }
            }
          break;
        case BASIC :
          rstat = this->dualRowStatus(row);
          break;
        default:
          MSG_ERROR( std::cerr << "ESOLVE27 ERROR: unknown VarStatus (" << int(stat) << ")"
                     << std::endl; )
            throw SPxInternalCodeException("XSOLVE23 This should never happen.");
        }
      return rstat;
    }

    template <class R>
      typename SPxBasis<R>::Desc::Status 
      SPxSolver<R>::varStatusToBasisStatusCol( int col, SPxSolver<R>::VarStatus stat ) const
    {
      typename SPxBasis<R>::Desc::Status cstat;

      switch( stat )
        {
        case FIXED :
          if (this->upper(col) == this->lower(col))
            cstat = SPxBasis<R>::Desc::P_FIXED;
          else if (this->maxObj(col) > 0.0)
            cstat = SPxBasis<R>::Desc::P_ON_UPPER;
          else
            cstat = SPxBasis<R>::Desc::P_ON_LOWER;
          break;
        case ON_UPPER :
          assert(this->upper(col) < infinity);
          cstat = this->lower(col) < this->upper(col)
            ? SPxBasis<R>::Desc::P_ON_UPPER
            : SPxBasis<R>::Desc::P_FIXED;
          break;
        case ON_LOWER :
          assert(this->lower(col) > -infinity);
          cstat = this->lower(col) < this->upper(col)
            ? SPxBasis<R>::Desc::P_ON_LOWER
            : SPxBasis<R>::Desc::P_FIXED;
          break;
        case ZERO :
          /* In this case the upper and lower bounds on the variable should be infinite. The bounds
           * might, however, have changed and we try to recover from this by changing the status to
           * 'resonable' settings. A solve has to find the correct values afterwards. Note that the
           * approach below is consistent with changesoplex.cpp (e.g., changeUpperStatus() and
           * changeLowerStatus() ). */
          if (this->lower(col) <= -infinity && this->upper(col) >= infinity)
            cstat = SPxBasis<R>::Desc::P_FREE;
          else
            {
              if ( this->lower(col) == this->upper(col) )
                {
                  assert( this->upper(col) < infinity );
                  cstat = SPxBasis<R>::Desc::P_FIXED;
                }
              else
                {
                  if ( this->lower(col) > -infinity )
                    cstat = SPxBasis<R>::Desc::P_ON_LOWER;
                  else
                    {
                      assert( this->upper(col) < infinity );
                      cstat = SPxBasis<R>::Desc::P_ON_UPPER;
                    }
                }
            }
          break;
        case BASIC :
          cstat = this->dualColStatus(col);
          break;
        default:
          MSG_ERROR( std::cerr << "ESOLVE28 ERROR: unknown VarStatus (" << int(stat) << ")"
                     << std::endl; )
            throw SPxInternalCodeException("XSOLVE24 This should never happen.");
        }
      return cstat;
    }

    /// #template #temp #baseclass
    template <>
      typename SPxSolver<Real>::VarStatus SPxSolver<Real>::getBasisRowStatus( int row ) const
    {
      assert( 0 <= row && row < this->nRows() );
      return basisStatusToVarStatus( this->desc().rowStatus( row ) );
    }

    /// #template #temp #baseclass
    template <>
      typename SPxSolver<Real>::VarStatus SPxSolver<Real>::getBasisColStatus( int col ) const
    {
      assert( 0 <= col && col < this->nCols() );
      return basisStatusToVarStatus( this->desc().colStatus( col ) );
    }

    /// #template #temp 
    template <>
      typename SPxSolver<Real>::Status SPxSolver<Real>::getBasis(VarStatus row[], VarStatus col[], const int rowsSize, const int colsSize) const
    {
      const typename SPxBasis<Real>::Desc& d = this->desc();
      int i;

      assert(rowsSize < 0 || rowsSize >= this->nRows());
      assert(colsSize < 0 || colsSize >= this->nCols());

      if (col)
        for (i = this->nCols() - 1; i >= 0; --i)
          col[i] = basisStatusToVarStatus( d.colStatus(i) );

      if (row)
        for (i = this->nRows() - 1; i >= 0; --i)
          row[i] = basisStatusToVarStatus( d.rowStatus(i) );

      return status();
    }

    template <class R>
      bool SPxSolver<R>::isBasisValid(DataArray<VarStatus> p_rows, DataArray<VarStatus> p_cols)
    {

      int basisdim;

      if ( p_rows.size() != this->nRows() || p_cols.size() != this->nCols() )
        return false;

      basisdim = 0;
      for ( int row = this->nRows()-1; row >= 0; --row )
        {
          if ( p_rows[row] == UNDEFINED )
            return false;
          // row is basic
          else if ( p_rows[row] == BASIC )
            {
              basisdim++;
            }
          // row is nonbasic
          else
            {
              if ( (p_rows[row] == FIXED && this->lhs(row) != this->rhs(row))
                   || (p_rows[row] == ON_UPPER && this->rhs(row) >= infinity)
                   || (p_rows[row] == ON_LOWER && this->lhs(row) <= -infinity) )
                return false;
            }
        }

      for ( int col = this->nCols()-1; col >= 0; --col )
        {
          if ( p_cols[col] == UNDEFINED )
            return false;
          // col is basic
          else if ( p_cols[col] == BASIC )
            {
              basisdim++;
            }
          // col is nonbasic
          else
            {
              if ( (p_cols[col] == FIXED && this->lower(col) != this->upper(col))
                   || (p_cols[col] == ON_UPPER && this->upper(col) >= infinity)
                   || (p_cols[col] == ON_LOWER && this->lower(col) <= -infinity) )
                return false;
            }
        }

      if ( basisdim != dim() )
        return false;

      // basis valid
      return true;
    }

    template <>
      void SPxSolver<Real>::setBasis(const VarStatus p_rows[], const VarStatus p_cols[])
    {
      if (SPxBasis<Real>::status() == SPxBasis<Real>::NO_PROBLEM)
        SPxBasis<Real>::load(this, false);

      typename SPxBasis<Real>::Desc ds = this->desc();
      int i;

      for(i = 0; i < this->nRows(); i++)
        ds.rowStatus(i) = varStatusToBasisStatusRow( i, p_rows[i] );

      for(i = 0; i < this->nCols(); i++)
        ds.colStatus(i) = varStatusToBasisStatusCol( i, p_cols[i] );

      loadBasis(ds);
      forceRecompNonbasicValue();
    }

    // NOTE: This only works for the row representation. Need to update to account for column representation.
    // The degenvec differs relative to the algorithm being used.
    // For the primal simplex, degenvec is the primal solution values.
    // For the dual simplex, the degenvec is the feasvec (ROW) and pVec (COLUMN).
    template <class R>
      Real SPxSolver<R>::getDegeneracyLevel(Vector degenvec)
    {
      int numDegenerate = 0;
      Real degeneracyLevel = 0;

      // iterating over all columns in the basis matrix
      // this identifies the basis indices and those that have a zero dual multiplier (rows) or zero reduced cost (cols).
      if( rep() == ROW )
        {
          for( int i = 0; i < this->nCols(); ++i ) // @todo Check the use of numColsReal for the reduced problem.
            {
              // degeneracy in the dual simplex exists if there are rows with a zero dual multiplier or columns with a zero
              // reduced costs. This requirement is regardless of the objective sense.
              if( isZero(degenvec[i], feastol()) )
                numDegenerate++;
            }

          if( type() == ENTER )   // dual simplex
            degeneracyLevel = Real(numDegenerate)/this->nCols();
          else                    // primal simplex
            {
              assert(type() == LEAVE);
              Real degenVars = (numDegenerate > (this->nCols() - this->nRows())) ? Real(numDegenerate - (this->nCols() - this->nRows())) : 0.0;
              degeneracyLevel = degenVars/this->nRows();
            }
        }
      else
        {
          assert(rep() == COLUMN);

          for( int i = 0; i < this->nCols(); i++ )
            {
              if( type() == LEAVE )   // dual simplex
                {
                  if( isZero(this->maxObj()[i] - degenvec[i], feastol()) )
                    numDegenerate++;
                }
              else                    // primal simplex
                {
                  assert( type() == ENTER );
                  if( isZero(degenvec[i], feastol()) )
                    numDegenerate++;
                }
            }


          if( type() == LEAVE )   // dual simplex
            {
              Real degenVars = this->nRows() > numDegenerate ? Real(this->nRows() - numDegenerate) : 0.0;
              degeneracyLevel = degenVars/this->nCols();
            }
          else                    // primal simplex
            {
              assert(type() == ENTER);
              Real degenVars = (numDegenerate > (this->nCols() - this->nRows())) ? Real(numDegenerate - (this->nCols() - this->nRows())) : 0.0;
              degeneracyLevel = degenVars/this->nRows();
            }
        }

      return degeneracyLevel;
    }

    template <class R>
      void SPxSolver<R>::getNdualNorms(int& nnormsRow, int& nnormsCol) const
    {
      nnormsRow = 0;
      nnormsCol = 0;

      if( weightsAreSetup )
        {
          if( type() == SPxSolver<R>::LEAVE && rep() == SPxSolver<R>::COLUMN )
            {
              nnormsRow = coWeights.dim();
              nnormsCol = 0;

              assert(nnormsRow == dim());
            }
          else if( type() == SPxSolver<R>::ENTER && rep() == SPxSolver<R>::ROW )
            {
              nnormsRow = weights.dim();
              nnormsCol = coWeights.dim();

              assert(nnormsRow == coDim());
              assert(nnormsCol == dim());
            }
        }
    }

    template <class R>
      bool SPxSolver<R>::getDualNorms(int& nnormsRow, int& nnormsCol, Real* norms) const
    {
      nnormsRow = 0;
      nnormsCol = 0;

      if( !weightsAreSetup )
        return false;

      if( type() == SPxSolver<R>::LEAVE && rep() == SPxSolver<R>::COLUMN )
        {
          nnormsCol = 0;
          nnormsRow = coWeights.dim();

          assert(nnormsRow == dim());

          for( int i = 0; i < nnormsRow; ++i)
            norms[i] = coWeights[i];
        }
      else if( type() == SPxSolver<R>::ENTER && rep() == SPxSolver<R>::ROW )
        {
          nnormsRow = weights.dim();
          nnormsCol = coWeights.dim();

          assert(nnormsCol == dim());
          assert(nnormsRow == coDim());

          for( int i = 0; i < nnormsRow; ++i )
            norms[i] = weights[i];

          for( int i = 0; i < nnormsCol; ++i )
            norms[nnormsRow + i] = coWeights[i];
        }
      else
        return false;

      return true;
    }

    template <class R>
      bool SPxSolver<R>::setDualNorms(int nnormsRow, int nnormsCol, Real* norms)
    {
      weightsAreSetup = false;

      if( type() == SPxSolver<R>::LEAVE && rep() == SPxSolver<R>::COLUMN)
        {
          coWeights.reDim(dim(), false);
          assert(coWeights.dim() >= nnormsRow);
          for( int i = 0; i < nnormsRow; ++i )
            coWeights[i] = norms[i];
          weightsAreSetup = true;
        }
      else if( type() == SPxSolver<R>::ENTER && rep() == SPxSolver<R>::ROW)
        {
          weights.reDim(coDim(), false);
          coWeights.reDim(dim(), false);
          assert(weights.dim() >= nnormsRow);
          assert(coWeights.dim() >= nnormsCol);
          for( int i = 0; i < nnormsRow; ++i )
            weights[i] = norms[i];
          for( int i = 0; i < nnormsCol; ++i )
            coWeights[i] = norms[nnormsRow + i];
          weightsAreSetup = true;
        }
      else
        return false;
      return true;
    }

    template <class R>
      void SPxSolver<R>::setIntegralityInformation(int ncols, int* intInfo)
    {
      assert(ncols == this->nCols() || (ncols == 0 && intInfo == NULL));

      integerVariables.reSize(ncols);
      for( int i = 0; i < ncols; ++i )
        {
          integerVariables[i] = intInfo[i];
        }
    }



    //
    // Auxiliary functions.
    //

    // Pretty-printing of variable status.
    template <class R>
      std::ostream& operator<<( std::ostream& os,
                                const typename SPxSolver<R>::VarStatus& status )
    {
      switch( status )
        {
        case SPxSolver<R>::BASIC:
          os << "BASIC";
          break;
        case SPxSolver<R>::FIXED:
          os << "FIXED";
          break;
        case SPxSolver<R>::ON_LOWER:
          os << "ON_LOWER";
          break;
        case SPxSolver<R>::ON_UPPER:
          os << "ON_UPPER";
          break;
        case SPxSolver<R>::ZERO:
          os << "ZERO";
          break;
        case SPxSolver<R>::UNDEFINED:
          os << "UNDEFINED";
          break;
        default:
          os << "?invalid?";
          break;
        }
      return os;
    }

    // Pretty-printing of solver status.
    template <class R>
      std::ostream& operator<<( std::ostream& os,
                                const typename SPxSolver<R>::Status& status )
    {
      switch ( status )
        {
        case SPxSolver<R>::ERROR:
          os << "ERROR";
          break;
        case SPxSolver<R>::NO_RATIOTESTER:
          os << "NO_RATIOTESTER";
          break;
        case SPxSolver<R>::NO_PRICER:
          os << "NO_PRICER";
          break;
        case SPxSolver<R>::NO_SOLVER:
          os << "NO_SOLVER";
          break;
        case SPxSolver<R>::NOT_INIT:
          os << "NOT_INIT";
          break;
        case SPxSolver<R>::ABORT_CYCLING:
          os << "ABORT_CYCLING";
          break;
        case SPxSolver<R>::ABORT_TIME:
          os << "ABORT_TIME";
          break;
        case SPxSolver<R>::ABORT_ITER:
          os << "ABORT_ITER";
          break;
        case SPxSolver<R>::ABORT_VALUE:
          os << "ABORT_VALUE";
          break;
        case SPxSolver<R>::SINGULAR:
          os << "SINGULAR";
          break;
        case SPxSolver<R>::NO_PROBLEM:
          os << "NO_PROBLEM";
          break;
        case SPxSolver<R>::REGULAR:
          os << "REGULAR";
          break;
        case SPxSolver<R>::RUNNING:
          os << "RUNNING";
          break;
        case SPxSolver<R>::UNKNOWN:
          os << "UNKNOWN";
          break;
        case SPxSolver<R>::OPTIMAL:
          os << "OPTIMAL";
          break;
        case SPxSolver<R>::UNBOUNDED:
          os << "UNBOUNDED";
          break;
        case SPxSolver<R>::INFEASIBLE:
          os << "INFEASIBLE";
          break;
        default:
          os << "?other?";
          break;
        }
      return os;
    }

    // Pretty-printing of algorithm.
    template <class R>
      std::ostream& operator<<( std::ostream& os,
                                const typename SPxSolver<R>::Type& status )
    {
      switch ( status )
        {
        case SPxSolver<R>::ENTER:
          os << "ENTER";
          break;
        case SPxSolver<R>::LEAVE:
          os << "LEAVE";
          break;
        default:
          os << "?other?";
          break;
        }
      return os;
    }

    // Pretty-printing of representation.
    template <class R>
      std::ostream& operator<<( std::ostream& os,
                                const typename SPxSolver<R>::Representation& status )
    {
      switch ( status )
        {
        case SPxSolver<R>::ROW:
          os << "ROW";
          break;
        case SPxSolver<R>::COLUMN:
          os << "COLUMN";
          break;
        default:
          os << "?other?";
          break;
        }
      return os;
    }


  } // namespace soplex
