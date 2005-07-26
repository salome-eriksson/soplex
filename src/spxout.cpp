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
#pragma ident "@(#) $Id: spxout.cpp,v 1.1 2005/07/25 15:22:10 bzforlow Exp $"

#include "spxout.h"

namespace soplex
{
   /// constructor
   SPxOut::SPxOut()
      : m_verbosity( ERROR )
      , m_streams( new std::ostream*[ DEBUG+1 ] )
   {
      m_streams[ ERROR ] = m_streams[ WARNING ] = &std::cerr;
      for ( int i = VERBOSE1; i <= DEBUG; ++i )
         m_streams[ i ] = &std::cout;
   }

   //---------------------------------------------------

   // destructor
   SPxOut::~SPxOut()
   {
      delete [] m_streams;
   }

   //---------------------------------------------------

   // define global instance
   SPxOut spxout;


} // namespace soplex

//-----------------------------------------------------------------------------
//Emacs Local Variables:
//Emacs mode:c++
//Emacs c-basic-offset:3
//Emacs tab-width:8
//Emacs indent-tabs-mode:nil
//Emacs End:
//-----------------------------------------------------------------------------