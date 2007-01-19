/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the class library                   */
/*       SoPlex --- the Sequential object-oriented simPlex.                  */
/*                                                                           */
/*    Copyright (C) 1997-1999 Roland Wunderling                              */
/*                  1997-2002 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SoPlex is distributed under the terms of the ZIB Academic Licence.       */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SoPlex; see the file COPYING. If not email to soplex@zib.de.  */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma ident "@(#) $Id: soplex.cpp,v 1.85 2007/01/19 14:22:12 bzftuchs Exp $"

#include <iostream>

#include "soplex.h"

namespace soplex
{
SoPlex::SoPlex(SPxSolver::Type p_type, SPxSolver::Representation p_rep)
   : m_solver(p_type, p_rep)
   , m_preScaler(0)
   , m_postScaler(0)
   , m_simplifier(0)
   , m_vanished(false)
{
   m_solver.setSolver(&m_slu);  
   m_solver.setTester(&m_fastRT);
   m_solver.setPricer(&m_steepPR);
   m_solver.setStarter(0);

   assert(SoPlex::isConsistent());
}

SoPlex::~SoPlex()
{}

void SoPlex::setPreScaler(SPxScaler* x)
{
   METHOD( "SoPlex::setScaler()" );

   m_preScaler = x;
}

void SoPlex::setPostScaler(SPxScaler* x)
{
   METHOD( "SoPlex::setScaler()" );

   m_postScaler = x;
}

void SoPlex::setSimplifier(SPxSimplifier* x)
{
   METHOD( "SoPlex::setSimplifier()" );

   m_simplifier = x;
}

Real SoPlex::objValue() const
{
   METHOD( "SoPlex::value()" );

   DVector x(nCols());

   getPrimal(x);

   return x * maxObj() * Real(spxSense());
}

SPxSolver::Status SoPlex::solve()
{
   METHOD( "SoPlex::solve()" );

   if (nRows() <= 0 && nCols() <= 0) // no problem loaded
      return SPxSolver::NO_PROBLEM;

   // assume presolver did NOT solve problem
   m_vanished = false; 

   {  // context for working LP
      SPxLP work(*this);

      // should the LP be scaled
      if (m_preScaler != 0)
         m_preScaler->scale(work);

      // should the LP be simplified ?
      if (m_simplifier != 0)
      {
         switch(m_simplifier->simplify(work, m_solver.epsilon(), m_solver.delta()))
         {
         case SPxSimplifier::UNBOUNDED :
            m_solver.setBasisStatus(SPxBasis::UNBOUNDED);
            return SPxSolver::UNBOUNDED;
         case SPxSimplifier::INFEASIBLE :
            m_solver.setBasisStatus(SPxBasis::INFEASIBLE);
            return SPxSolver::INFEASIBLE;
         case SPxSimplifier::VANISHED :
            m_vanished = true;
            return SPxSolver::OPTIMAL;
         case SPxSimplifier::OKAY:
            break;
         default:
            assert(false);
         }
      }
      // should the LP be scaled after simplifing?
      if (m_postScaler != 0)
         m_postScaler->scale(work);

      m_solver.loadLP(work);
   }
   return m_solver.solve();
}

SPxSolver::Status SoPlex::getPrimal(Vector& x) const
{
   METHOD( "SoPlex::getPrimal()" );
   
   if (has_simplifier())
   {
      if (!m_simplifier->isUnsimplified())
         unsimplify();

      x = m_simplifier->unsimplifiedPrimal();
         
      // unscale prescaling
      if (m_preScaler != 0)
         m_preScaler->unscalePrimal(x);
      
      if (m_vanished)
         return SPxSolver::OPTIMAL;
      else
         return m_solver.status();
   }
   
   // else 
   SPxSolver::Status stat = m_solver.getPrimal(x);
            
   // unscale postscaling
   if (m_postScaler != 0)
      m_postScaler->unscalePrimal(x);
 
   // unscale prescaling
   if (m_preScaler != 0)
      m_preScaler->unscalePrimal(x);
           
   return stat;
}

SPxSolver::Status SoPlex::getSlacks(Vector& s) const
{
   METHOD( "SoPlex::getSlacks()" );

   if (has_simplifier())
   {
      if (!m_simplifier->isUnsimplified())
         unsimplify();

      s = m_simplifier->unsimplifiedSlacks();
         
      // unscale prescaling
      if (m_preScaler != 0)
         m_preScaler->unscaleSlacks(s);
      
      if (m_vanished)
         return SPxSolver::OPTIMAL;
      else
         return m_solver.status();
   }
   
   // else 
   SPxSolver::Status stat = m_solver.getSlacks(s);
            
   // unscale postscaling
   if (m_postScaler != 0)
      m_postScaler->unscaleSlacks(s);
 
   // unscale prescaling
   if (m_preScaler != 0)
      m_preScaler->unscaleSlacks(s);
           
   return stat;
}

SPxSolver::Status SoPlex::getDual(Vector& pi) const
{
   METHOD( "SoPlex::getDual()" );
   
   if (has_simplifier())
   {
      if (!m_simplifier->isUnsimplified())
         unsimplify();

      pi = m_simplifier->unsimplifiedDual();
         
      // unscale prescaling
      if (m_preScaler != 0)
         m_preScaler->unscaleDual(pi);
      
      if (m_vanished)
         return SPxSolver::OPTIMAL;
      else
         return m_solver.status();
   }
   
   // else 
   SPxSolver::Status stat = m_solver.getDual(pi);
            
   // unscale postscaling
   if (m_postScaler != 0)
      m_postScaler->unscaleDual(pi);
 
   // unscale prescaling
   if (m_preScaler != 0)
      m_preScaler->unscaleDual(pi);
           
   return stat;
}
  
SPxSolver::Status SoPlex::getRedCost(Vector& rdcost) const
{
   METHOD( "SoPlex::getRedCost()" );
   
   if (has_simplifier())
   {
      if (!m_simplifier->isUnsimplified())
         unsimplify();

      rdcost = m_simplifier->unsimplifiedRedCost();
         
      // unscale prescaling
      if (m_preScaler != 0)
         m_preScaler->unscaleRedCost(rdcost);
      
      if (m_vanished)
         return SPxSolver::OPTIMAL;
      else
         return m_solver.status();
   }
   
   // else 
   SPxSolver::Status stat = m_solver.getRedCost(rdcost);
            
   // unscale postscaling
   if (m_postScaler != 0)
      m_postScaler->unscaleRedCost(rdcost);
 
   // unscale prescaling
   if (m_preScaler != 0)
      m_preScaler->unscaleRedCost(rdcost);
           
   return stat;
}

SPxSolver::VarStatus SoPlex::getBasisRowStatus(int i) const
{
   if (has_simplifier())
   {
      if (!m_simplifier->isUnsimplified())
         unsimplify();
      
      return m_simplifier->getBasisRowStatus(i);
   }
   else
      return m_solver.getBasisRowStatus(i);
}

SPxSolver::VarStatus SoPlex::getBasisColStatus(int j) const
{
   if (has_simplifier())
   {
      if (!m_simplifier->isUnsimplified())
         unsimplify();
      
      return m_simplifier->getBasisColStatus(j);
   }
   else
      return m_solver.getBasisColStatus(j);
}

SPxSolver::Status SoPlex::getBasis(SPxSolver::VarStatus rows[], SPxSolver::VarStatus cols[]) const
{
   if (has_simplifier())
   {
      if (!m_simplifier->isUnsimplified())
         unsimplify();
      
      m_simplifier->getBasis(rows, cols);
      return m_solver.status();
   }

   else
      return m_solver.getBasis(rows, cols);
}

SPxSolver::Status SoPlex::getDualfarkas(Vector& dualfarkas) const
{
   /// Does not work yet with presolve
   if (has_simplifier())
   {
      MSG_ERROR( spxout << "ESOLVR02 Not yet implemented" << std::endl; )
      return SPxSolver::ERROR;
   }
   SPxSolver::Status stat = m_solver.getDualfarkas(dualfarkas);

   if (m_postScaler != 0)
      m_postScaler->unscaleDual(dualfarkas);

   if (m_preScaler != 0)
      m_preScaler->unscaleDual(dualfarkas);
   
   return stat;
}
  
void SoPlex::qualConstraintViolation(
   Real& maxviol, 
   Real& sumviol) const
{
   maxviol = 0.0;
   sumviol = 0.0;

   DVector solu( nCols() );

   getPrimal( solu );

   for( int row = 0; row < nRows(); ++row )
   {
      const SVector& rowvec = rowVector( row );

      Real val = 0.0;         

      for( int col = 0; col < rowvec.size(); ++col )
         val += rowvec.value( col ) * solu[rowvec.index( col )];

      Real viol = 0.0;

      assert(lhs( row ) <= rhs( row ));

      if (val < lhs( row )) 
         viol = fabs(val - lhs( row ));
      else
         if (val > rhs( row ))
            viol = fabs(val - rhs( row ));

      if (viol > maxviol)
         maxviol = viol;

      sumviol += viol;
   }
}

void SoPlex::qualBoundViolation(
   Real& maxviol, 
   Real& sumviol) const
{
   maxviol = 0.0;
   sumviol = 0.0;

   DVector solu( nCols() );

   getPrimal( solu );

   for( int col = 0; col < nCols(); ++col )
   {
      assert( lower( col ) <= upper( col ));

      Real viol = 0.0;

      if (solu[col] < lower( col ))
         viol = fabs( solu[col] - lower( col ));
      else
         if (solu[col] > upper( col ))
            viol = fabs( solu[col] - upper( col ));
         
      if (viol > maxviol)
         maxviol = viol;

      sumviol += viol;
   }
}

bool SoPlex::writeBasisFile(
   const char* filename, 
   const NameSet& rowNames, 
   const NameSet& colNames)
   const
{   
   return m_solver.writeBasisFile(filename, rowNames, colNames);
}

void SoPlex::unsimplify() const
{
   assert(has_simplifier());

   if (m_simplifier->isUnsimplified())
      return;

   DVector psp_x(m_solver.nCols());  // primal solution (prescaled simplified postscaled)
   DVector psp_y(m_solver.nRows());  // dual solution   (prescaled simplified postscaled) 
   DVector psp_s(m_solver.nRows());  // slacks          (prescaled simplified postscaled)
   DVector psp_r(m_solver.nCols());  // reduced costs   (prescaled simplified postscaled)
    
   // If there is no sensible solution, do nothing.
   const SPxSolver::Status  stat = status();
   if (stat != SPxSolver::OPTIMAL)
      return;
    
   if (! m_vanished) {
      m_solver.getPrimal(psp_x);
      m_solver.getDual(psp_y);
      m_solver.getSlacks(psp_s);
      m_solver.getRedCost(psp_r);
   
      // unscale postscaling
      if (m_postScaler != 0)
      {
         m_postScaler->unscalePrimal(psp_x);
         m_postScaler->unscaleDual(psp_y);
         m_postScaler->unscaleSlacks(psp_s);
         m_postScaler->unscaleRedCost(psp_r);
      }
   }
   else
   {
      psp_x.reDim(0);
      psp_y.reDim(0);
      psp_s.reDim(0);
      psp_r.reDim(0);
   }

   // unsimplify
   SPxSolver::VarStatus* rows = new SPxSolver::VarStatus[m_solver.nRows()];
   SPxSolver::VarStatus* cols = new SPxSolver::VarStatus[m_solver.nCols()];
   m_solver.getBasis(rows, cols);

   m_simplifier->unsimplify(psp_x, psp_y, psp_s, psp_r, rows, cols);
   
   delete[] rows;
   delete[] cols;
}

} // namespace soplex

//-----------------------------------------------------------------------------
//Emacs Local Variables:
//Emacs mode:c++
//Emacs c-basic-offset:3
//Emacs tab-width:8
//Emacs indent-tabs-mode:nil
//Emacs End:
//-----------------------------------------------------------------------------







